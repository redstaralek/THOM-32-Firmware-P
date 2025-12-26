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
            ResetUtils::deep_sleep(SEGS_DEEP_SL_CONFIG);
            }
        }

        Serial.printf("%s \n",   STR_INFO_CONFIG);
        Serial.printf("%s", STR_INFO_SSID); printArray(config->ssid,  config->qtdRedes); Serial.printf("\n");
        printArray(config->user,  config->qtdRedes); Serial.printf("\n");
        printArray(config->senha,  config->qtdRedes); Serial.printf("\n");
        Serial.printf("%s %s\n", STR_INFO_TOKEN,    config->token.c_str());
        Serial.printf("%s %s\n", STR_INFO_BIRUTA,   config->modeloBiruta);
                    
    } 


    private: static bool convertBSSIDToArray(const String& bssidStr, uint8_t* output) {
        int values[6];
        if (sscanf(bssidStr.c_str(), "%x:%x:%x:%x:%x:%x", 
                    &values[0], &values[1], &values[2], 
                    &values[3], &values[4], &values[5]) == 6) {
            for (int i = 0; i < 6; i++) {
                output[i] = (uint8_t)values[i];
            }
            return true;
        }
        return false;
    }

    //==================================================================================
    //============================= INTERNO: Conexão WiFi ==============================
    //==================================================================================
    private: static bool _conectaWiFiMelhorRssi(ConfigEstacao *config, bool contingencia = false) {
         if(!WiFi.enableSTA(true)) {
            Serial.println("[ERRO] Falha ao ativar modo STA!");
            return false;
        }

        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.disconnect(true);
        delay(1000);

        const int MAX_BSSIDS = 20;  
        BSSIDInfo bssids[MAX_BSSIDS];
        int bssidCount = 0;

        int16_t n = WiFi.scanNetworks();
        if (n == WIFI_SCAN_FAILED) {
            Serial.print("Falha no scan de redes.");
            return false;
        }

        // First, collect all compatible networks
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

                    Serial.printf("\nEncontrado SSID: %s, com BSSID: %s e RSSI: %d\n", 
                        scannedSSID.c_str(), bssidStr.c_str(), WiFi.RSSI(i));

                    bssids[bssidCount].bssid = bssidStr;
                    bssids[bssidCount].ssid  = scannedSSID;
                    bssids[bssidCount].rssi  = WiFi.RSSI(i);
                    bssids[bssidCount].user  = config->user[j];
                    bssids[bssidCount].senha = config->senha[j];
                    bssids[bssidCount].hasInternet = false; // Will be tested later
                    bssidCount++;
                }
            }
        }

        if (bssidCount == 0) {
            Serial.print("Nenhum access point compatível encontrado!");
            return false;
        }

        // Sort by best signal
        for (int i = 0; i < bssidCount - 1; i++) {
            for (int j = i + 1; j < bssidCount; j++) {
                if (bssids[i].rssi < bssids[j].rssi) {
                    BSSIDInfo temp = bssids[i];
                    bssids[i] = bssids[j];
                    bssids[j] = temp;
                }
            }
        }

        // Track best networks with different statuses
        int bestTimeoutIndex = -1;    // Best network that had ping timeout (poor signal but potential internet)
        int bestNoInternetIndex = -1; // Best network that definitely has no internet
        int bestConnectedIndex = -1;  // Best network we successfully connected to

        // Try each network
        for (int i = 0; i < min(NUM_TENTATIVAS_CONEXAO, bssidCount); i++) {
            Serial.printf("\nTentativa %d: Conectando a %s (RSSI: %d)\n", 
                        i + 1, bssids[i].ssid.c_str(), bssids[i].rssi);

            uint8_t bssid[6];
            if (!WiFiUtils::convertBSSIDToArray(bssids[i].bssid, bssid)) {
                Serial.println("[ERRO]: Falha ao converter BSSID.");
                continue;
            }

            // Validate BSSID
            bool allZeros = true;
            for (int k = 0; k < 6; k++) {
                if (bssid[k] != 0x00) {
                    allZeros = false;
                    break;
                }
            }
            if (allZeros) {
                Serial.println("[ERRO] BSSID inválido");
                continue;
            }

            // Connect to the network
            if (bssids[i].user.length() > 0) {
                WiFi.begin(bssids[i].ssid.c_str(), WPA2_AUTH_PEAP, 
                        bssids[i].user.c_str(), bssids[i].user.c_str(),
                        bssids[i].senha.c_str(), NULL, NULL, NULL, 0, bssid, true);
            } else {
                WiFi.begin(bssids[i].ssid.c_str(), bssids[i].senha.c_str(), 0, bssid);
            }

            // Wait for connection
            uint16_t counterAux = 0;
            while (WiFi.status() != WL_CONNECTED && counterAux < ESPERA_CONEXAO) {
                Serial.printf("%s %d.", STR_INFO_ESPERANDO_WIFI, WiFi.RSSI());
                delay(500);
                esp_task_wdt_reset();
                counterAux++;
            }

            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[ERRO]: Falha ao conectar. Tentando próximo...");
                WiFi.disconnect(true);
                delay(1000);
                continue;
            }

            // Successfully connected - now test internet with detailed ping analysis
            Serial.println("Conectado! \nTestando internet...");
            delay(2000);

            PingResult result = _detailedPingTest();
            
            if (result.status == PING_SUCCESS) {
                Serial.println("✓ Internet funcionando!");
                return true;
            }
            else if (result.status == PING_TIMEOUT) {
                Serial.printf("⚠ Timeout de ping (sinal fraco?): perda de pacotes: %.1f%%\n", result.packetLoss);
                
                // Remember this as the best timeout network (potential internet)
                if (bestTimeoutIndex == -1 || bssids[i].rssi > bssids[bestTimeoutIndex].rssi) {
                    bestTimeoutIndex = i;
                    Serial.println("Rede com timeout registrada como candidata");
                }
            }
            else if (result.status == PING_NO_INTERNET) {
                Serial.println("✗ Definitivamente sem internet (host inalcançável)");
                
                // Remember this as best no-internet network
                if (bestNoInternetIndex == -1 || bssids[i].rssi > bssids[bestNoInternetIndex].rssi) {
                    bestNoInternetIndex = i;
                }
            }

            // Remember we successfully connected to this network
            if (bestConnectedIndex == -1 || bssids[i].rssi > bssids[bestConnectedIndex].rssi) {
                bestConnectedIndex = i;
            }

            // Disconnect to try next network
            WiFi.disconnect(true);
            delay(1000);
        }

        // Decision logic: Choose the best available option
        if (bestTimeoutIndex != -1) {
            Serial.printf("\nEscolhendo rede com timeout (potencial internet): %s\n", 
                        bssids[bestTimeoutIndex].ssid.c_str());
            return _connectToNetwork(bssids[bestTimeoutIndex]);
        }
        else if (bestConnectedIndex != -1) {
            Serial.printf("\nEscolhendo melhor rede conectável: %s\n", 
                        bssids[bestConnectedIndex].ssid.c_str());
            return _connectToNetwork(bssids[bestConnectedIndex]);
        }
        else if (bestNoInternetIndex != -1) {
            Serial.printf("\nEscolhendo melhor rede (sem internet): %s\n", 
                        bssids[bestNoInternetIndex].ssid.c_str());
            return _connectToNetwork(bssids[bestNoInternetIndex]);
        }

        Serial.println("[ERRO]: Nenhuma rede pôde ser conectada.");
        return false;
    }

    // Enhanced ping test that distinguishes between timeout and no internet
    private: static PingResult _detailedPingTest() {
        PingResult result = {PING_NO_INTERNET, 100};
        
        // Try multiple ping targets first
        const char* targets[] = {"8.8.8.8", "1.1.1.1", "www.google.com"};
        
        for (int i = 0; i < 3; i++) {
            Serial.printf("Testando ping para: %s\n", targets[i]);
            
            // Add WiFi check before each ping
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi desconectado durante teste de ping");
                result.status = PING_NO_INTERNET;
                return result;
            }
            
            bool success = Ping.ping(targets[i], 2); // Reduced to 2 attempts
            
            if (success) {
                result.status = PING_SUCCESS;
                result.packetLoss = 0;
                Serial.println("✓ Ping bem-sucedido!");
                return result;
            }
            
            delay(500); // Reduced delay
            esp_task_wdt_reset(); // Prevent watchdog timeout
        }
        
        // If all pings failed, try HTTP with safety measures
        Serial.println("Todos pings falharam, testando HTTP...");
        
        WiFiClient client;
        client.setTimeout(5000); // Set timeout upfront
        
        // Add connection timeout protection
        unsigned long connectStart = millis();
        bool connected = false;
        
        while (millis() - connectStart < 3000) { // 3 second connect timeout
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi perdido durante teste HTTP");
                result.status = PING_NO_INTERNET;
                return result;
            }
            
            connected = client.connect("httpbin.org", 80);
            if (connected) break;
            delay(100);
        }
        
        if (connected) {
            Serial.println("Conexão HTTP estabelecida, testando resposta...");
            
            client.println("GET /ip HTTP/1.1");
            client.println("Host: httpbin.org");
            client.println("Connection: close");
            client.println();
            
            // Wait for response with timeout
            unsigned long responseStart = millis();
            while (!client.available() && (millis() - responseStart < 3000)) {
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("WiFi perdido aguardando resposta HTTP");
                    client.stop();
                    result.status = PING_NO_INTERNET;
                    return result;
                }
                delay(10);
                esp_task_wdt_reset();
            }
            
            if (client.available()) {
                // Got HTTP response but ping failed - poor signal
                result.status = PING_TIMEOUT;
                Serial.println("✓ HTTP funciona mas ping falha - timeout por sinal fraco");
            } else {
                // No HTTP response - likely no internet
                result.status = PING_NO_INTERNET;
                Serial.println("✗ Sem resposta HTTP - sem internet");
            }
        } else {
            // Couldn't establish HTTP connection
            result.status = PING_NO_INTERNET;
            Serial.println("✗ Falha conexão HTTP - sem internet");
        }
        
        // Safe cleanup
        if (client.connected()) {
            client.stop();
        }
        
        return result;
    }


    // Helper function to connect to a specific network
    private: static bool _connectToNetwork(BSSIDInfo &network) {
        uint8_t bssid[6]; // Stack allocation instead of static
        
        if (!WiFiUtils::convertBSSIDToArray(network.bssid, bssid)) {
            return false;
        }

        // Validate BSSID is not all zeros
        bool allZeros = true;
        for (int i = 0; i < 6; i++) {
            if (bssid[i] != 0x00) {
                allZeros = false;
                break;
            }
        }
        if (allZeros) {
            Serial.println("[ERRO] BSSID inválido (todos zeros)");
            return false;
        }

        // Connection attempt - NO DISCONNECT since we assume already connected
        bool success = false;
        if (network.user.length() > 0) {
            success = WiFi.begin(network.ssid.c_str(), WPA2_AUTH_PEAP, 
                    network.user.c_str(), network.user.c_str(),
                    network.senha.c_str(), NULL, NULL, NULL, 0, bssid, true);
        } else {
            success = WiFi.begin(network.ssid.c_str(), network.senha.c_str(), 0, bssid);
        }

        if (!success) {
            Serial.println("[ERRO] WiFi.begin() retornou false");
            return false;
        }

        // Wait with timeout and watchdog resets
        uint16_t counter = 0;
        while (WiFi.status() != WL_CONNECTED && counter < ESPERA_CONEXAO) {
            delay(500);
            counter++;
            esp_task_wdt_reset(); // Prevent watchdog timeout
        }

        bool connected = (WiFi.status() == WL_CONNECTED);
        if (!connected) {
            Serial.printf("[ERRO] Timeout conexão após %d tentativas\n", counter);
        }
        
        return connected;
    }

    //==================================================================================
    //============================= PÚBLICO: Conexão WiFi ==============================
    //==================================================================================
    public: static bool conectaWiFi(ConfigEstacao *config) {

        // setCpuFrequencyMhz(FREQ_CPU_MAX_WIFI);

        //------------------------ Tenta conexão
        uint16_t contadorAux = 0;    
        Serial.printf(STR_INFO_NOVA_TENTATIVA_WIFI); printArray(config->ssid , config->qtdRedes);Serial.print("\n");
        do{ 
            Serial.printf("...(%d) ", contadorAux);  
            contadorAux++;
            esp_task_wdt_reset();
        }while(contadorAux <= NU_RETRY_CONEXAO && !_conectaWiFiMelhorRssi(config, false));
        Serial.print("\n");
        return (WiFi.status() == WL_CONNECTED);

    }


    //==================================================================================
    //=========================== INTERNO: Requisição à API ============================
    //==================================================================================
    private: static bool executaReq(ConfigEstacao *config, String jsonStr, LeituraTS *ts, volatile Tempos *tempos) {

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

        clienteHttp.end();

        if (status == HTTP_STS_OK) {
            Serial.print(STR_INFO_STATUS); Serial.println(status);
            uint heapAfter = ESP.getFreeHeap();
            Serial.printf("Free heap after POST: %d\n", heapAfter);
            if(heapAfter < 30000){
                ResetUtils::resetaEstacao(ts, tempos);
            }
        }else {
            Serial.print("[[ERROR]] Código: "); Serial.println(status);
            if (status == HTTP_STS_ERRO_INT) {
                ResetUtils::resetaEstacao(ts, tempos);
            }
        }

        return (status == HTTP_STS_OK);
    }


    //==================================================================================
    //=========================== PÚBLICO: Requisição à API ============================
    //==================================================================================
    public: static bool preparaEExecutaReq(ConfigEstacao *config, LeituraTS *ts, volatile Tempos *tempos, char* jsonEnvio) {
        String jsonStrEnvio = String(jsonEnvio);
        ushort contadorAux = 0;
        while(contadorAux <= NU_RETRY_ENVIO) {
            if(executaReq(config, jsonStrEnvio, ts, tempos)) {
                memset(_leiturasAcumuladas, 0, LEITURAS_BUFFER_SIZE);
                return true;
            }
            delay(1000 * (++contadorAux));
            esp_task_wdt_reset();
        }

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
        //setCpuFrequencyMhz(FREQ_CPU_MIN_SENS);
    }
  
};

