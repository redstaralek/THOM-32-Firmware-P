class WiFiUtils{
  //==================================================================================
  //================= Colhe configurações básicas de rede do usuário =================
  //==================================================================================
  public: static void configuraEstacaoRede(ConfigEstacao *config){ 

    Serial.setTimeout(10000);
    uint8_t contadorAux = 0;

    //------------------------ Colhe dados de config. da estação (tenta NU_RETRY_CREDENCIAIS vezes)
    while(!ValidateUtils::credenciaisValidas(config)){
      Serial.println(STR_INPUT_S_N);  Serial.flush();
      if(inputToBool(Serial.readStringUntil('\n'))){
        
        Serial.setTimeout(60000);

        //---------------------- Colhe SSID
        Serial.println(STR_INPUT_JSON); Serial.flush();
        String configJson = Serial.readStringUntil('\n');
        *config = SerializationUtils::getConfigObj(configJson); 

        if(!ValidateUtils::credenciaisValidas(config)){
          //-------------------- Dados não fornecidos, busca na memória
          Serial.println(STR_INFO_RECUP_MEM); 
        }else{ 
          //-------------------- Serializa e salva inputs na memória
          configJson = SerializationUtils::getConfigJson(config);
          EepromUtils::escreveDadoEEPROM(ENDERECO_CONFIG_1,  configJson);
        }

      }else{
        Serial.println(STR_INFO_BUSCANDO_WIFI_MEMORIA);
      }
      //------------------------ Recupera dados na memória
      String configJson = EepromUtils::getDadoEEPROM(ENDERECO_CONFIG_1);
      *config = SerializationUtils::getConfigObj(configJson);

      //------------------------ Se dados inválidos -> continua no loop até máximo tentativas
      if(!ValidateUtils::credenciaisValidas(config))
        Serial.println(STR_DADOS_ERROR_VAZIOS);

      contadorAux++;

      if(contadorAux >= NU_RETRY_CREDENCIAIS){
        // Erro de configuração, precisa de correção MANUAL --> DEEP SLEEP DE 1H (evita bateria ser zerada)
        // TODO: Serviço web poderia detectar ausência de 1h e notificar cliente/técnico para configurar a conexão WiFi
        ResetUtils::deep_sleep(SEGS_DEEP_SL_CONFIG, MSG_RESET_BAD_CONFIG);
      }
    }

    Serial.println(String(STR_INFO_CONFIG));
    Serial.println(String(STR_INFO_SSID  ) + String(config->ssid ));
    Serial.println(String(STR_INFO_TOKEN ) + String(config->token));
    Serial.println(String(STR_INFO_BIRUTA) + String(config->modeloBiruta));
                
  } 


  private: static uint8_t* convertBSSIDToArray(const String& bssidStr) {
    static uint8_t bssid[6]; // Static array to hold the BSSID
    int values[6];
    
    // Parse the BSSID string (format: "XX:XX:XX:XX:XX:XX")
    if (sscanf(bssidStr.c_str(), "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            bssid[i] = (uint8_t)values[i];
        }
    } else {
        Serial.println("[ERRO]: Formato de BSSID inválido!");
        return nullptr;
    }
    
    return bssid;
  }

  //==================================================================================
  //============================= INTERNO: Conexão WiFi ==============================
  //==================================================================================
  private: static bool _conectaWiFiMelhorRssi(ConfigEstacao *config, bool contingencia=false) {

    //------------------------ Prepara conexão
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.enableSTA(true);
    WiFi.disconnect(true);
    delay(1000); 

    // Encontra o BSSID com melhor sinal para o dado for the given SSID
    String melhorBSSID = "";
    int melhorIndiceBSSID = 0;
    int melhorRSSI = -100;   // Inicia em um valor muito baixo (inconectável)
    delay(100);

    Serial.println("Buscando redes WiFi compatíveis...");
    uint16_t n = WiFi.scanNetworks();
    for (uint16_t i = 0; i < n; i++) {
        if (WiFi.SSID(i) == config->ssid) {
            Serial.printf("Encontrado SSID: %s, com BSSID: %s e RSSI: %d\n", WiFi.SSID(i).c_str(), WiFi.BSSIDstr(i).c_str(), WiFi.RSSI(i));
            if (WiFi.RSSI(i) > melhorRSSI) {
                melhorRSSI  = WiFi.RSSI(i);
                melhorBSSID = WiFi.BSSIDstr(i);
                melhorIndiceBSSID = i;
            }
        }
    }

    if (melhorBSSID == "") {
        Serial.println("[ERRO]: Nenhum access point encontrado para o dado SSID!");
        return false;
    }

    Serial.printf("Conectando ao BSSID %s (RSSI: %d, indice: %d)\n", melhorBSSID.c_str(), melhorRSSI, melhorIndiceBSSID);

    uint8_t* bssid = WiFiUtils::convertBSSIDToArray(melhorBSSID);
    if (bssid == nullptr) {
        Serial.println("[ERRO]: Falha ao converter BSSID!");
        return false;
    }
    
    // Print the BSSID for debugging
    Serial.printf("BSSID to connect: %02X:%02X:%02X:%02X:%02X:%02X\n", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);


    //------------------------ Attempt connection
    if (ValidateUtils::possuiUser(config)) {
        // Conexão WPA2 (empresarial/campus) via PEAP
        WiFi.begin(config->ssid.c_str(), WPA2_AUTH_PEAP, config->user.c_str(), config->user.c_str(), config->senha.c_str(), NULL, NULL, NULL, 0, WiFi.BSSID(melhorIndiceBSSID),true);
        // WiFi.begin(config->ssid.c_str(), WPA2_AUTH_PEAP, config->user.c_str(), config->user.c_str(), config->senha.c_str(), bssid, true);
    } else {
        // Conexão WPA2 (pessoal)
        WiFi.begin(config->ssid.c_str(), config->senha.c_str(), 0, bssid);
    }

    //------------------------ Espera pela conexão
    uint16_t counterAux = 0;
    do{
        Serial.println(STR_INFO_ESPERANDO_WIFI + String(WiFi.RSSI()));  
        delay(500);
        counterAux++; 
    }while(WiFi.status() != WL_CONNECTED && counterAux < ESPERA_CONEXAO);

    return WiFi.status() == WL_CONNECTED;
}


  //==================================================================================
  //============================= PÚBLICO: Conexão WiFi ==============================
  //==================================================================================
  public: static bool conectaWiFi(ConfigEstacao *config) {

    // setCpuFrequencyMhz(FREQ_CPU_MAX_WIFI);

    //------------------------ Tenta conexão
    uint16_t contadorAux = 0;    
    do{
      Serial.println(STR_INFO_NOVA_TENTATIVA_WIFI + config->ssid + ". Tentativa="+contadorAux);  
      contadorAux++;
    }while(contadorAux <= NU_RETRY_CONEXAO && !_conectaWiFiMelhorRssi(config, false));

    esp_task_wdt_reset();
    
    return (WiFi.status() == WL_CONNECTED);

  }

  
  //==================================================================================
  //=========================== INTERNO: Requisição à API ============================
  //==================================================================================
  private: static bool executaReq(ConfigEstacao *config, String jsonStr, LeituraTS *ts, volatile Tempos *tempos) {

    //---------------------- Prepara requisição
    HTTPClient clienteHttp;
    clienteHttp.begin(config->url);
    clienteHttp.addHeader("Content-Type", CONTENT_TYPE);
    clienteHttp.setTimeout(HTTP_TIMEOUT);
    
    //---------------------- Envia requisicao
    Serial.println("Enviando pacote de dados: " +jsonStr);
    short status = clienteHttp.POST(jsonStr);

    //---------------------- Analisa requisição
    if(status == HTTP_STS_OK){
      Serial.println(STR_INFO_STATUS       + String(status));
      strcpy(_descricaoResetSoftware, "");
    }else{
      Serial.println(STR_INFO_STATUS_FALHA + String(status));
      if (status == HTTP_STS_ERRO_INT){
        ResetUtils::resetaEstacao(ts, tempos, MSG_RESET_FALHA_CONEXAO);
      }
    }
    clienteHttp.end();

    // [  Positivo   ] ---> SUCESSO! (Ignora erros da API, impossível corrigir no lado do µ-controlador)
    // [  Negativo   ] ---> FALHA..! (Falha de comunicação.............................................)
    return status > 0;

  }


  //==================================================================================
  //=========================== PÚBLICO: Requisição à API ============================
  //==================================================================================
  public: static bool preparaEExecutaReq(ConfigEstacao *config, LeituraTS *ts, volatile Tempos *tempos) {

    byte contadorAux = 0;
    String jsonStrEnvio = "";
    String jsonAgora = SerializationUtils::getLeituraJson(ts, tempos, true);
    String leiturasAcumuladas = String(_leiturasAcumuladas);
    if(isNotNullOrEmptyStr(leiturasAcumuladas)){
        size_t totalSize = jsonAgora.length() + 1 + leiturasAcumuladas.length(); // +1 pela ","
        if (totalSize < sizeof(_leiturasAcumuladas)) {
            jsonStrEnvio = leiturasAcumuladas + "," + jsonAgora;
        } else {
            Serial.println("[[WARNING]]: Leituras acumuladas + atual ultrapassam o buffer máximo. Usando apenas atual.");
            jsonStrEnvio = jsonAgora;
        }
    }

    if(!ehJsonSeparadoPorVirgulasValido(jsonStrEnvio)){
      Serial.println("[[ERROR]]: Não foi possível juntar a leitura atual com as acumuladas. Utilizando apenas a atual.");
      jsonStrEnvio = jsonAgora;
    }

    while(!executaReq(config, SerializationUtils::envelopa(jsonStrEnvio, config), ts, tempos)){ 
      contadorAux++;
      if(contadorAux > NU_RETRY_CONEXAO){
        Serial.println("[[WARNING]]: Como não foi possível enviar pacote de dados, as leituras foram acumuladas");
        
        jsonStrEnvio.toCharArray(_leiturasAcumuladas, sizeof(_leiturasAcumuladas));
        strcpy(_leiturasAcumuladas, jsonStrEnvio.c_str());
        
        Serial.println("[[INFO]]: Leituras acumuladas: "+String(_leiturasAcumuladas));
        return false;
      }
    }
    
    memset(_leiturasAcumuladas, 0, sizeof(_leiturasAcumuladas));
    return true;
  }


  //==================================================================================
  //=========================== Desconecta e desliga WiFi ============================
  //==================================================================================
  public: static void desligaWiFi(){

      WiFi.disconnect(true);
      delay(1000);
      WiFi.mode(WIFI_OFF);
      WiFi.setSleep(true);
      esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
      // setCpuFrequencyMhz(FREQ_CPU_MIN_SENS);

  }
  
};

