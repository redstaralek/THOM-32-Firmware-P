
class SerializationUtils{
    public: static void printLeituraJson(LeituraTS *ts, volatile Tempos *tempos) {
        // Use a smaller initial size since we're streaming output
        DynamicJsonDocument doc(JSON_CONFIG_SIZE);
        
        //---------------------- horario
        doc["dta"] = TimeUtils::getTime();

        //---------------------- Digitais
        {
            // BME680
            addFloatToDoc(doc, "tmp", round2(safeDiv(ts->tmp,ts->qtd)));
            addFloatToDoc(doc, "hum", round2(safeDiv(ts->hum, ts->qtd)));
            addFloatToDoc(doc, "prs", round2(safeDiv(ts->prs, ts->qtd)));
            addFloatToDoc(doc, "gas", round2(safeDiv(ts->gas, ts->qtd)));
            
            addFloatToDoc(doc, "cpu", round2(safeDiv(ts->tmpCpu, ts->qtd)));

            // AS7331
            if (ts->qtdUVBruto > 0){
                addFloatToDoc(doc, "uva", round2(safeDiv(ts->uva, ts->qtdUVBruto)));
                addFloatToDoc(doc, "uvb", round2(safeDiv(ts->uvb, ts->qtdUVBruto)));
                addFloatToDoc(doc, "uvc", round2(safeDiv(ts->uvc, ts->qtdUVBruto)));
            }

            // VEML7700
            if(ts->qtdLx > 0){
                addFloatToDoc(doc, "lx", round2(safeDiv(ts->lx, ts->qtdLx)));
            }

            // AS3935
            if(ts->qtdR > 0){
                addFloatToDoc(doc, "rdis", round2(safeDiv(ts->rdis, ts->qtdR)));
                addFloatToDoc(doc, "rpot", round2(safeDiv(ts->rpot, ts->qtdR)));
                addFloatToDoc(doc, "rQtd", round2(ts->qtdR));
            }

            if(ts->qtdCo2 > 0){
                addFloatToDoc(doc, "co2", round2(safeDiv(ts->co2,ts->qtdCo2)));
            }

            if(ts->qtdTvoc > 0){
                addFloatToDoc(doc, "voc",  round2(safeDiv(ts->tvoc ,ts->qtdTvoc)));
                addFloatToDoc(doc, "eCo2", round2(safeDiv(ts->eCo2, ts->qtdTvoc)));
            }
        }

        //---------------------- Analógicos
        {
            addFloatToDoc(doc, "dir", MediaUtils::angular(ts->sen, ts->cos, ts->qtd));

            if (ts->qtdBat > 0)
                addFloatToDoc(doc, "bat", round2(safeDiv(ts->bat, ts->qtdBat)));
        }

        //---------------------- Interrupções
        {
            doc["seg"] = ((float)(getMillis() - tempos->cicloInicio)) / 1000;
            doc["dorm"] = ((float)tempos->tempoDormido) / 1000;
            doc["segP"] = ((float)_tempoGirosMax) / 1000;
            doc["gir"] = _gir;
            doc["girMx"] = _girMax;
            doc["plv"] = _plv;
        }

        //---------------------- Outros dados
        {
            doc["prim"] = _primeiro;
            doc["sin"] = WiFi.RSSI();
            doc["hal"] = safeDiv(ts->hal, ts->qtd);
            doc["mem"] = ESP.getFreeHeap();

            if (doc["prim"]) {
                doc["mot"] = esp_reset_reason();
                doc["dsc"] = "";
            }
        }

        //--------------------- Health Check (sensores)
        {
            doc["bme"] = ts->bmeOk;
            doc["as73"] = ts->as7331Ok;
            doc["veml"] = ts->vemlOk;
            doc["as39"] = ts->as3935Ok;
            doc["scd"] = ts->scdOk;
        }

        // Print directly to avoid String overhead
        serializeJson(doc, Serial);
    }

    
    public: static bool getLeituraJsonCompact(char* buffer, size_t bufferSize, LeituraTS *ts, volatile Tempos *tempos) {
        // Null checks for input pointers
        if (ts == nullptr || tempos == nullptr || buffer == nullptr) {
            Serial.println("[ERRO] Ponteiros nulos em getLeituraJsonCompact");
            if (buffer != nullptr && bufferSize >= 3) {
                strncpy(buffer, "{}", bufferSize);
            }
            return false;
        }

        // Check buffer size
        if (bufferSize < 3) { // Minimum for "{}"
            Serial.println("[ERRO] Buffer muito pequeno");
            return false;
        }

        DynamicJsonDocument doc(JSON_ENVIO_OBJ_MAX_SIZE);
        
        // Check if allocation succeeded
        if (doc.capacity() == 0) {
            Serial.println("[ERRO] Falha ao alocar DynamicJsonDocument");
            strncpy(buffer, "{}", bufferSize);
            return false;
        }

        JsonArray arr = doc.to<JsonArray>();
        if (arr.isNull()) {  // Check array creation
            Serial.println("[ERRO] Falha ao criar JsonArray");
            strncpy(buffer, "{}", bufferSize);
            return false;
        }

        // Helper lambda for safe division
        auto safeDiv = [](float numerator, int denominator, float defaultVal = -1.0f) {
            return (denominator > 0) ? round2(numerator / denominator) : defaultVal;
        };

        // Index 0 - Timestamp
        arr.add(TimeUtils::getTime());                               // 0. Data e hora

        // Index 1-3 - Termohigrobarometria
        arr.add(ts->qtd > 0 ? round2(ts->tmp / ts->qtd) : -1);       // 1. Temperatura
        arr.add(safeDiv(ts->hum, ts->qtd));                          // 2. Umidade
        arr.add(safeDiv(ts->prs, ts->qtd));                          // 3. Pressão

        // Index 4-7 - Variáveis Solares
        arr.add(safeDiv(ts->uva, ts->qtdUVBruto));                   // 4. UVA
        arr.add(safeDiv(ts->uvb, ts->qtdUVBruto));                   // 5. UVB
        arr.add(safeDiv(ts->uvc, ts->qtdUVBruto));                   // 6. UVC
        arr.add(safeDiv(ts->lx, ts->qtdLx));                         // 7. Iluminância

        // Index 8-10 - Anemometria
        arr.add(_gir);                                               // 8. Giro atual
        arr.add(_girMax);                                            // 9. Giro máximo
        arr.add(ts->qtd > 0 ? MediaUtils::angular(ts->sen, ts->cos, ts->qtd) : 0); // 10. Direção

        // Index 11 - Pluviometria
        arr.add(_plv);                                               // 11. Pulsos

        // Index 12-15 - Qualidade do Ar
        arr.add(safeDiv(ts->gas, ts->qtd));                          // 12. Gás
        arr.add(safeDiv((int) ts->co2, ts->qtdCo2));                 // 13. CO2
        arr.add(safeDiv(ts->tvoc, ts->qtdTvoc));                     // 14. TVOC
        arr.add(safeDiv((int) ts->eCo2, ts->qtdTvoc));               // 15. eCO2

        // Index 16-18 - Fulminologia
        arr.add(safeDiv(ts->rdis, ts->qtdR));                        // 16. Distância raio
        arr.add(safeDiv(ts->rpot, ts->qtdR));                        // 17. Potência raio
        arr.add(round2(ts->qtdR));                                   // 18. Quantidade raios

        // Index 19-21 - Tempos
        uint32_t cicloInicio = tempos->cicloInicio;  // Single read of volatile
        uint32_t tempoDormido = tempos->tempoDormido;
        arr.add((getMillis() - cicloInicio) / 1000.0f);              // 19. Tempo ciclo
        arr.add(tempoDormido / 1000.0f);                             // 20. Tempo dormido
        arr.add(_tempoGirosMax / 1000.0f);                           // 21. Tempo pico giros

        // Index 22-29 - Monitoramento
        arr.add(safeDiv((int) ts->tmpCpu, ts->qtd));                 // 22. Temp CPU
        arr.add(safeDiv(ts->bat, ts->qtdBat));                       // 23. Bateria
        arr.add(_primeiro ? 1 : 0);                                  // 24. Primeiro ciclo
        arr.add(WiFi.isConnected() ? WiFi.RSSI() : 0);               // 25. RSSI
        arr.add(ts->qtd > 0 ? (int)(ts->hal / ts->qtd) : 0);         // 26. HAL
        arr.add(ESP.getFreeHeap());                                  // 27. Memória livre
        arr.add(_primeiro ? esp_reset_reason() : 0);                 // 28. Motivo reset
        arr.add("");                                                 // 29. Descrição reset

        // Index 30 - Status sensores
        if(!ts->bmeOk || !ts->as7331Ok || !ts->vemlOk || !ts->as3935Ok || !ts->scdOk) {
            uint _sum = 0;
            if(!ts->bmeOk) _sum += 2;
            if(!ts->as7331Ok) _sum += 3;
            if(!ts->as3935Ok) _sum += 5;
            if(!ts->scdOk) _sum += 7;
            arr.add(_sum);                                          // 30. Status
        }

        // Serialize directly to buffer
        size_t bytesWritten = serializeJson(doc, buffer, bufferSize);
        if (bytesWritten == 0) {
            Serial.println("[ERRO] Falha ao serializar JSON");
            strncpy(buffer, "{}", bufferSize);
            return false;
        }

        // Ensure null termination
        if (bytesWritten >= bufferSize) {
            buffer[bufferSize - 1] = '\0';
            Serial.println("[AVISO] JSON truncado - buffer muito pequeno");
            return false;
        }

        buffer[bytesWritten] = '\0'; // Ensure null termination
        return true;
    }


    public: static bool envelopa(char* outputBuffer, size_t outputBufferSize, const char* jsonStrEnvio, ConfigEstacao *config) {

        // Calculate required buffer size
        int neededSize = snprintf(nullptr, 0, "[\"%s\",\"%s\",\"%s\",%u,%u,%u,[%s]]",
                                config->token.c_str(), VERSAO_FIRMWARE, WiFi.macAddress().c_str(),
                                ESP.getSketchSize(), ESP.getFlashChipSize(), sizeof(*config), 
                                jsonStrEnvio) + 1; // +1 for null terminator

        // Check if output buffer is large enough
        if (neededSize > outputBufferSize) {
            Serial.println("[ERRO] Buffer de saída muito pequeno em envelopa");
            outputBuffer[0] = '\0';
            return false;
        }

        // Format the envelope string
        snprintf(outputBuffer, outputBufferSize, "[\"%s\",\"%s\",\"%s\",%u,%u,%u,[%s]]",
                config->token.c_str(), VERSAO_FIRMWARE, WiFi.macAddress().c_str(),
                ESP.getSketchSize(), ESP.getFlashChipSize(), sizeof(*config), 
                jsonStrEnvio);

        return true;
    }

        
    public: static String getConfigJson(ConfigEstacao *config){
        String jsonStr;
        DynamicJsonDocument doc(JSON_CONFIG_SIZE);

        doc["url"] = config->url;
        doc["tokn"] = config->token;

        JsonArray ssids  = doc.createNestedArray("ssid");
        JsonArray users  = doc.createNestedArray("user");
        JsonArray senhas = doc.createNestedArray("pass");

        for (int i = 0; i < config->qtdRedes; i++) {
            ssids.add(config->ssid[i]);
            users.add(config->user[i]);
            senhas.add(config->senha[i]);
        }

        doc["anem"] = config->modeloBiruta;

        serializeJson(doc, jsonStr);
        return jsonStr;
    }

    public: static ConfigEstacao getConfigObj(const String &jsonStr) {
        DynamicJsonDocument doc(JSON_CONFIG_SIZE);
        DeserializationError error = deserializeJson(doc, jsonStr);
        ConfigEstacao config;

        if (error) {
            Serial.println("Erro ao deserializar obj de configuração da estação.");
            return ConfigEstacao();
        }

        config.token = doc["tokn"] | "";
        config.url   = doc["url"]  | "";

        if (doc.containsKey("ssid") && doc["ssid"].is<JsonArray>()) {
            JsonArray ssids  = doc["ssid"];
            JsonArray users  = doc["user"];
            JsonArray senhas = doc["pass"];

            int n = min((int) ssids.size(), MAX_WIFIS);
            config.qtdRedes = n;

            for (int i = 0; i < n; i++) {
                config.ssid[i]  = ssids[i].as<String>();
                config.user[i]  = users[i].as<String>();
                config.senha[i] = senhas[i].as<String>();
            }
        }

        config.modeloBiruta = doc["anem"] | "";

        return config;
    }


};