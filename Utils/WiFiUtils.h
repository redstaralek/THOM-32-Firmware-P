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
    Serial.println(String(STR_INFO_SSID  ) + arrayToString(config->ssid,  config->qtdRedes));
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
  private: static bool _conectaWiFiMelhorRssi(ConfigEstacao *config, bool contingencia = false) {
    
    if(!WiFi.enableSTA(true)) {
        Serial.println("[ERRO] Falha ao ativar modo STA");
        return false;
    }
    
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.disconnect(true);
    delay(1000);

    struct BSSIDInfo {
        String bssid;
        String ssid;
        String user;
        String senha;
        int rssi;
    };

    const int MAX_BSSIDS = 20;  
    BSSIDInfo bssids[MAX_BSSIDS];
    int bssidCount = 0;

    Serial.println("Buscando redes WiFi compatíveis...");
    int16_t n = WiFi.scanNetworks();
    if (n == WIFI_SCAN_FAILED) {
        Serial.println("[ERRO] Falha no scan de redes");
        return false;
    }

    for (int i = 0; i < n && bssidCount < MAX_BSSIDS; i++) {
        String scannedSSID = WiFi.SSID(i);
        if (scannedSSID.isEmpty()) continue;

        for (int j = 0; j < config->qtdRedes && bssidCount < MAX_BSSIDS; j++) {
            if (scannedSSID == config->ssid[j]) {
                String bssidStr = WiFi.BSSIDstr(i);
                if (bssidStr.isEmpty() || bssidStr == "00:00:00:00:00:00") {
                    Serial.println("[AVISO] BSSID inválido encontrado, ignorando...");
                    continue;
                }

                Serial.printf("Encontrado SSID: %s, com BSSID: %s e RSSI: %d\n", 
                    scannedSSID.c_str(), bssidStr.c_str(), WiFi.RSSI(i));

                bssids[bssidCount].bssid = bssidStr;
                bssids[bssidCount].ssid  = scannedSSID;
                bssids[bssidCount].rssi  = WiFi.RSSI(i);
                bssids[bssidCount].user  = config->user[j];
                bssids[bssidCount].senha = config->senha[j];
                bssidCount++;
            }
        }
    }

    if (bssidCount == 0) {
        Serial.println("[ERRO]: Nenhum access point compatível encontrado.");
        return false;
    }

    // Arranja pelo melhor sinal
    for (int i = 0; i < bssidCount - 1; i++) {
        for (int j = i + 1; j < bssidCount; j++) {
            if (bssids[i].rssi < bssids[j].rssi) {
                BSSIDInfo temp = bssids[i];
                bssids[i] = bssids[j];
                bssids[j] = temp;
            }
        }
    }

    // Tenta os N melhores
    int tentativas = min(NUM_TENTATIVAS_CONEXAO, bssidCount);
    for (int i = 0; i < tentativas; i++) {
        Serial.printf("Tentando conectar ao BSSID %s (SSID: %s, RSSI: %d)\n", bssids[i].bssid.c_str(), bssids[i].ssid.c_str(), bssids[i].rssi);

        uint8_t* bssid = WiFiUtils::convertBSSIDToArray(bssids[i].bssid);
        if (bssid == nullptr) {
            Serial.println("[ERRO]: Falha ao converter BSSID.");
            continue;
        }

        // [Added] Validate BSSID isn't all zeros
        bool allZeros = true;
        for (int k = 0; k < 6; k++) {
            if (bssid[k] != 0x00) {
                allZeros = false;
                break;
            }
        }
        if (allZeros) {
            Serial.println("[ERRO] BSSID inválido (todos zeros)");
            continue;
        }

        Serial.printf("BSSID to connect: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

        // Se tiver user, loga com user, se não, loga normal
        if (bssids[i].user.length() > 0) {
            WiFi.begin(bssids[i].ssid.c_str(), WPA2_AUTH_PEAP, 
                      bssids[i].user.c_str(), bssids[i].user.c_str(),
                      bssids[i].senha.c_str(), NULL, NULL, NULL, 0, bssid, true);
        } else {
            WiFi.begin(bssids[i].ssid.c_str(), bssids[i].senha.c_str(), 0, bssid);
        }

        // Conecta
        uint16_t counterAux = 0;
        do {
            Serial.println(STR_INFO_ESPERANDO_WIFI + String(WiFi.RSSI()));
            delay(500);
            esp_task_wdt_reset();
            counterAux++;
        } while (WiFi.status() != WL_CONNECTED && counterAux < ESPERA_CONEXAO);

        if (WiFi.status() == WL_CONNECTED) {
            uint8_t* connBSSID = WiFi.BSSID();
            Serial.printf("Conectado ao BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
                          connBSSID[0], connBSSID[1], connBSSID[2],
                          connBSSID[3], connBSSID[4], connBSSID[5]);
            return true;
        } else {
            Serial.println("[ERRO]: Falha ao conectar. Tentando próximo...");
            WiFi.disconnect(true);
            delay(1000);
        }
    }

    Serial.println("[ERRO]: Nenhum BSSID pôde ser conectado.");
    return false;
  }


  //==================================================================================
  //============================= PÚBLICO: Conexão WiFi ==============================
  //==================================================================================
  public: static bool conectaWiFi(ConfigEstacao *config) {

    // setCpuFrequencyMhz(FREQ_CPU_MAX_WIFI);

    //------------------------ Tenta conexão
    uint16_t contadorAux = 0;    
    do{
      Serial.println(STR_INFO_NOVA_TENTATIVA_WIFI + arrayToString(config->ssid , config->qtdRedes) + ". Tentativa="+contadorAux);  
      contadorAux++;
      esp_task_wdt_reset();
    }while(contadorAux <= NU_RETRY_CONEXAO && !_conectaWiFiMelhorRssi(config, false));
    
    return (WiFi.status() == WL_CONNECTED);

  }

  
  //==================================================================================
  //=========================== INTERNO: Requisição à API ============================
  //==================================================================================
  private: static bool executaReq(ConfigEstacao *config, String jsonStr, LeituraTS *ts, volatile Tempos *tempos) {
    jsonStr.reserve(jsonStr.length() + 512);

    HTTPClient clienteHttp;
    clienteHttp.begin(config->url);
    clienteHttp.addHeader("Content-Type", CONTENT_TYPE);
    clienteHttp.setTimeout(HTTP_TIMEOUT);
    
    Serial.print("Enviando pacote ("); Serial.print(jsonStr.length()); Serial.print(" bytes) \n"); Serial.println(jsonStr);
    
    Serial.print("Free heap before POST: ");
    Serial.println(ESP.getFreeHeap());
    short status = clienteHttp.POST(
        (uint8_t*)jsonStr.c_str(),
        jsonStr.length()
    );

    if (status == HTTP_STS_OK) {
        Serial.print(STR_INFO_STATUS); Serial.println(status);
    }else {
        Serial.print("[[ERROR]] Código: "); Serial.println(status);
        if (status == HTTP_STS_ERRO_INT) {
            clienteHttp.end();
            ResetUtils::resetaEstacao(ts, tempos, MSG_RESET_FALHA_CONEXAO);
        }
    }

    clienteHttp.end();

    return (status == HTTP_STS_OK);
}


  //==================================================================================
  //=========================== PÚBLICO: Requisição à API ============================
  //==================================================================================
  public: static bool preparaEExecutaReq(ConfigEstacao *config, LeituraTS *ts, volatile Tempos *tempos) {
      TimeUtils::conectaNtpEFallbacks();

      String jsonStrEnvio = ""; jsonStrEnvio.reserve(LEITURAS_BUFFER_SIZE + 256 + 2);

      String jsonAgora; jsonAgora.reserve(256);
      jsonAgora = SerializationUtils::getLeituraJsonCompact(ts, tempos);

      String leiturasAcumuladas; leiturasAcumuladas.reserve(LEITURAS_BUFFER_SIZE);
      leiturasAcumuladas = String(_leiturasAcumuladas);

      jsonStrEnvio = jsonAgora;

      if(isNotNullOrEmptyStr(leiturasAcumuladas)) {
          size_t totalSize = jsonAgora.length() + leiturasAcumuladas.length() + 1;
          if (totalSize <= (LEITURAS_BUFFER_SIZE + 256 + 2)) {
              jsonStrEnvio = leiturasAcumuladas + "," + jsonAgora;
          } else {
              Serial.println("[[WARNING]]: Tamanho excedido. Usando apenas leitura atual.");
          }
      }

      if(!ehJsonSeparadoPorVirgulasValido(jsonStrEnvio)) {
          Serial.println("[[ERROR]]: JSON inválido. Usando leitura atual.");
          jsonStrEnvio = jsonAgora;
      }

      ushort contadorAux = 0;
      while(contadorAux <= NU_RETRY_ENVIO) {
          if(executaReq(config, SerializationUtils::envelopa(jsonStrEnvio, config), ts, tempos)) {
              memset(_leiturasAcumuladas, 0, LEITURAS_BUFFER_SIZE);
              return true;
          }
          delay(1000 * (++contadorAux));
          esp_task_wdt_reset();
      }

      if (jsonStrEnvio.length() < LEITURAS_BUFFER_SIZE - 1)
          snprintf(_leiturasAcumuladas, LEITURAS_BUFFER_SIZE, "%s", jsonStrEnvio.c_str());

      return false;
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

