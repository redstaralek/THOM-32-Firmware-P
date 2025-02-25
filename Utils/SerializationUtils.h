
class SerializationUtils{
    
    public: static String getLeituraJson(LeituraTS *ts, volatile Tempos *tempos, bool incluiHorario) {
        
        String jsonStr;
        DynamicJsonDocument doc(JSON_OBJ_SIZE);
        //---------------------- horario
        // if (incluiHorario && syncedTime > 0)
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
                doc["dsc"]          = String(_descricaoResetSoftware);
            }
        }

        serializeJson(doc, jsonStr);

        return String(jsonStr);
        
    }
    
    public: static String envelopa(String jsonStrEnvio, ConfigEstacao *config){

        //---------------------- FIXOS NESTA VERSÃO DO SOFTWARE / HARDWARE (n muda na msma série de pacotes)
        //------- Textuais
        return "{\"id\":\""     + config->token                  + "\","
               +"\"ver\":\""    + VERSAO_FIRMWARE                + "\","
               +"\"mac\": \""   + WiFi.macAddress()              + "\","
               // Numéricos                   
               +"\"firm\": "    + String(ESP.getSketchSize())    + ","
               +"\"flsh\": "    + String(ESP.getFlashChipSize()) + ","
               +"\"fs\": "      + String(sizeof(*config))+ ","
               // Leituras (muda no msm pacote, proc a parte)
               +"\"dados\": ["  + jsonStrEnvio                   + "]}";

    }


    public: static String getConfigJson(ConfigEstacao *config){
        String jsonStr;
        DynamicJsonDocument doc(200);

        doc["url"]      = config->url;
        doc["tokn"]     = config->token;

        doc["ssid"]     = config->ssid;
        doc["user"]     = config->user;
        doc["pass"]     = config->senha;

        doc["apn"]      = config->apn;
        doc["userSIM"]  = config->userSIM;
        doc["senhaSIM"] = config->senhaSIM;

        doc["anem"] = config->modeloBiruta;
        
        serializeJson(doc, jsonStr);

        return String(jsonStr);
    }


    public: static ConfigEstacao getConfigObj(const String &jsonStr) {
        DynamicJsonDocument doc(200);
        DeserializationError error = deserializeJson(doc, jsonStr);
        ConfigEstacao config;

        if (error) {
            Serial.println("Erro ao deserializar obj de configuração da estação.");
            return ConfigEstacao();
        }

        if (doc.containsKey("tokn"))
            config.token    = doc["tokn"].as<String>();

        if (doc.containsKey("url"))
            config.url      = doc["url"].as<String>();

        if (doc.containsKey("ssid"))
            config.ssid     = doc["ssid"].as<String>();

        if (doc.containsKey("user"))
            config.user     = doc["user"].as<String>();

        if (doc.containsKey("pass"))
            config.senha    = doc["pass"].as<String>();

        if (doc.containsKey("apn"))
            config.apn   = doc["apn"].as<String>();

        if (doc.containsKey("userSIM"))
            config.userSIM   = doc["userSIM"].as<String>();

        if (doc.containsKey("senhaSIM"))
            config.senhaSIM  = doc["senhaSIM"].as<String>();
        
        if (doc.containsKey("anem"))
            config.modeloBiruta = doc["anem"].as<String>();

        return config;
    }

};