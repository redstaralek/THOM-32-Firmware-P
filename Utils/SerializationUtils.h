
class SerializationUtils{
    
    public: static String getLeituraJson(LeituraTS *ts, volatile Tempos *tempos) {
        
        String jsonStr;
        DynamicJsonDocument doc(JSON_OBJ_SIZE);
        //---------------------- horario
        doc["dta"] = TimeUtils::getTime();

        //---------------------- Digitais
        {
            // BME680
            addFloatToDoc(doc, "tmp",   round2(ts->tmp / ts->qtd));
            addFloatToDoc(doc, "hum",   round2(((float)ts->hum) / ts->qtd));
            addFloatToDoc(doc, "prs",   round2(((float)ts->prs) / ts->qtd));
            addFloatToDoc(doc, "gas",   round2(((float)ts->gas) / ts->qtd));
            
            addFloatToDoc(doc, "cpu",   round2(((float)ts->tmpCpu) / ts->qtd));

            // AS7331
            if (ts->qtdUVBruto > 0){
                addFloatToDoc(doc, "uva",  round2(((float)ts->uva) / ts->qtdUVBruto));
                addFloatToDoc(doc, "uvb",  round2(((float)ts->uvb) / ts->qtdUVBruto));
                addFloatToDoc(doc, "uvc",  round2(((float)ts->uvc) / ts->qtdUVBruto));
            }

            // VEML7700
            if(ts->qtdLx > 0){
                addFloatToDoc(doc, "lx",    round2(((float)ts->lx) / ts->qtdLx));
            }

            // AS3935
            if(ts->qtdR > 0){
                addFloatToDoc(doc, "rdis",   round2(((float)ts->rdis) / ts->qtdR));
                addFloatToDoc(doc, "rpot",   round2(((float)ts->rpot) / ts->qtdR));
                addFloatToDoc(doc, "rQtd",   round2(ts->qtdR));
            }

            if(ts->qtdCo2 >0){
                addFloatToDoc(doc, "co2",   round2(((float)ts->co2) / ts->qtdCo2));
            }

            if(ts->qtdTvoc >0){
                addFloatToDoc(doc, "voc",   round2(((float)ts->tvoc) / ts->qtdTvoc));
                addFloatToDoc(doc, "eCo2",   round2(((float)ts->eCo2) / ts->qtdTvoc));
            }
        }

        //---------------------- Analógicos
        {
            addFloatToDoc(doc, "dir", MediaUtils::angular(ts->sen, ts->cos, ts->qtd));

            if (ts->qtdBat > 0)
                addFloatToDoc(doc, "bat", round2(ts->bat / ts->qtdBat));
        }

        //---------------------- Interrupções
        {
            doc["seg"]              = ((float) (getMillis() - tempos->cicloInicio)) / 1000;
            doc["dorm"]             = ((float) tempos->tempoDormido)    / 1000;
            doc["segP"]             = ((float) _tempoGirosMax)          / 1000;
            doc["gir"]              = _gir;
            doc["girMx"]            = _girMax;
            doc["plv"]              = _plv;
        }

        //---------------------- Outros dados
        {
            doc["prim"]             = _primeiro;
            doc["sin"]              = WiFi.RSSI();
            doc["hal"]              = (int) ts->hal      / ts->qtd;
            doc["mem"]              = ESP.getFreeHeap();

            if (doc["prim"]) {
                doc["mot"]          = esp_reset_reason();
                doc["dsc"]          = "";//String(_descricaoResetSoftware);
            }
        }

        //--------------------- Health Check (sensores)
        {
            doc["bme"]              = ts->bmeOk;
            doc["as73"]             = ts->as7331Ok;
            doc["veml"]             = ts->vemlOk;
            doc["as39"]             = ts->as3935Ok;
            doc["scd"]              = ts->scdOk;
        }

        serializeJson(doc, jsonStr);

        return String(jsonStr);
        
    }
    
    public: static String getLeituraJsonCompact(LeituraTS *ts, volatile Tempos *tempos) {
        // [Added] Null checks for input pointers
        if (ts == nullptr || tempos == nullptr) {
            Serial.println("[ERRO] Ponteiros nulos em getLeituraJsonCompact");
            return "{}";
        }

        String jsonStr;
        DynamicJsonDocument doc(JSON_OBJ_SIZE);
        
        // [Added] Check if allocation succeeded
        if (doc.capacity() == 0) {
            Serial.println("[ERRO] Falha ao alocar DynamicJsonDocument");
            return "{}";
        }

        JsonArray arr = doc.to<JsonArray>();
        if (arr.isNull()) {  // [Added] Check array creation
            Serial.println("[ERRO] Falha ao criar JsonArray");
            return "{}";
        }

        // Helper lambda for safe division [Added]
        auto safeDiv = [](float numerator, int denominator, float defaultVal = -1.0f) {
            return (denominator > 0) ? round2(numerator / denominator) : defaultVal;
        };

        // Index 0 - Timestamp
        arr.add(TimeUtils::getTime());  // 0. Data e hora

        // Index 1-3 - Termohigrobarometria [Added division checks]
        arr.add(ts->qtd > 0 ? round2(ts->tmp / ts->qtd) : -1);      // 1. Temperatura
        arr.add(safeDiv(ts->hum, ts->qtd));                          // 2. Umidade
        arr.add(safeDiv(ts->prs, ts->qtd));                          // 3. Pressão

        // Index 4-7 - Variáveis Solares [Already has checks]
        arr.add(safeDiv(ts->uva, ts->qtdUVBruto));                   // 4. UVA
        arr.add(safeDiv(ts->uvb, ts->qtdUVBruto));                   // 5. UVB
        arr.add(safeDiv(ts->uvc, ts->qtdUVBruto));                   // 6. UVC
        arr.add(safeDiv(ts->lx, ts->qtdLx));                         // 7. Iluminância

        // Index 8-10 - Anemometria
        arr.add(_gir);                                               // 8. Giro atual
        arr.add(_girMax);                                            // 9. Giro máximo
        // [Added] Check for division by zero in angular calculation
        arr.add(ts->qtd > 0 ? MediaUtils::angular(ts->sen, ts->cos, ts->qtd) : 0); // 10. Direção

        // Index 11 - Pluviometria
        arr.add(_plv);                                               // 11. Pulsos

        // Index 12-15 - Qualidade do Ar [Added checks]
        arr.add(safeDiv(ts->gas, ts->qtd));                          // 12. Gás
        arr.add(safeDiv((int) ts->co2, ts->qtdCo2));                 // 13. CO2
        arr.add(safeDiv(ts->tvoc, ts->qtdTvoc));                     // 14. TVOC
        arr.add(safeDiv((int) ts->eCo2, ts->qtdTvoc));               // 15. eCO2

        // Index 16-18 - Fulminologia [Added checks]
        arr.add(safeDiv(ts->rdis, ts->qtdR));                        // 16. Distância raio
        arr.add(safeDiv(ts->rpot, ts->qtdR));                        // 17. Potência raio
        arr.add(round2(ts->qtdR));                                   // 18. Quantidade raios

        // Index 19-21 - Tempos [Added volatile safety]
        uint32_t cicloInicio = tempos->cicloInicio;  // Single read of volatile
        uint32_t tempoDormido = tempos->tempoDormido;
        arr.add((getMillis() - cicloInicio) / 1000.0f);  // 19. Tempo ciclo
        arr.add(tempoDormido / 1000.0f);                // 20. Tempo dormido
        arr.add(_tempoGirosMax / 1000.0f);              // 21. Tempo pico giros

        // Index 22-29 - Monitoramento [Added checks]
        arr.add(safeDiv((int) ts->tmpCpu, ts->qtd));                 // 22. Temp CPU
        arr.add(safeDiv(ts->bat, ts->qtdBat));                       // 23. Bateria
        arr.add(_primeiro ? 1 : 0);                                  // 24. Primeiro ciclo
        arr.add(WiFi.isConnected() ? WiFi.RSSI() : 0);               // 25. RSSI (safe)
        arr.add(ts->qtd > 0 ? (int)(ts->hal / ts->qtd) : 0);         // 26. HAL
        arr.add(ESP.getFreeHeap());                                  // 27. Memória livre
        arr.add(_primeiro ? esp_reset_reason() : 0);                 // 28. Motivo reset
        arr.add("");                                                 // 29. Descrição reset

        // Index 30 - Status sensores [Added checks]
        if(!ts->bmeOk || !ts->as7331Ok || !ts->vemlOk || !ts->as3935Ok || !ts->scdOk) {
            uint _sum = 0;
            if(!ts->bmeOk) _sum += 2;
            if(!ts->as7331Ok) _sum += 3;
            if(!ts->as3935Ok) _sum += 5;
            if(!ts->scdOk) _sum += 7;
            arr.add(_sum);  // 30. Status
        }

        // [Added] Serialization error handling
        if (serializeJson(doc, jsonStr) == 0) {
            Serial.println("[ERRO] Falha ao serializar JSON");
            return "{}";
        }

        return jsonStr;
    }

    public: static String envelopa(String jsonStrEnvio, ConfigEstacao *config) {
        
        String output; output.reserve(256 + jsonStrEnvio.length());

        output = "[\"" + config->token + "\","
                + "\"" + VERSAO_FIRMWARE + "\","
                + "\"" + WiFi.macAddress() + "\","
                + String(ESP.getSketchSize()) + ","
                + String(ESP.getFlashChipSize()) + ","
                + String(sizeof(*config)) + ","
                + "[" + (jsonStrEnvio) + "]]";

        return output;
    }

        
    public: static String getConfigJson(ConfigEstacao *config){
        String jsonStr;
        DynamicJsonDocument doc(512);

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
        DynamicJsonDocument doc(512);
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