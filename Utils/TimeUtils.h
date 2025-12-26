
unsigned long   lastSyncMillis = 0;
time_t          syncedTime = 0;

class TimeUtils{
    

    //==================================================================================
    //======================= Conecta ao Servidor NTP e Fallbacks ======================
    //==================================================================================
    public: static bool conectaNtpEFallbacks() {
        const char* servidoresNtp[] = NTP_SERVERS;
        uint8_t qtdServidores = sizeof(servidoresNtp) / sizeof(const char*);

        for (uint8_t i = 0; i < qtdServidores; i++) {
            // Check WiFi before each attempt
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi disconnected during NTP sync");
                return false;
            }

            configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, servidoresNtp[i]);
            delay(5000);
            
            for(uint8_t j = 0; j < 20; j++) {
                // Check WiFi before each time() call
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("WiFi lost during NTP validation");
                    return false;
                }

                time_t agora = time(nullptr);
                
                // Better timestamp validation
                if (agora > 1000000000) { // Verifica se o timestamp é válido (ex.: pós-2001)
                    syncedTime = agora;
                    lastSyncMillis = getMillis();
                    Serial.printf("NTP sync successful! Time: %ld\n", syncedTime);
                    return true;
                } else {
                    Serial.printf("Invalid NTP time (%ld), attempt %d/%d\n", agora, j+1, 20);
                }
                delay(50);
            }
            Serial.printf("NTP server %s failed, trying next...\n", servidoresNtp[i]);
        }

        Serial.println("All NTP servers failed");
        return false;
    }



    //==================================================================================
    //========================== Pega horário do Servidor NTP ==========================
    //==================================================================================
    public: static const char* getTime() {
        static char buffer[20]; // "YYYY-MM-DD HH:MM:SS" + null terminator
        unsigned long millisAtuais = getMillis();
        time_t tempoAtual = syncedTime + (millisAtuais - lastSyncMillis) / 1000;

        // Debug output (optional)
        Serial.printf("syncedTime: %ld, lastSyncMillis: %lu, millisAtuais: %lu, tempoAtual: %ld\n",
                    syncedTime, lastSyncMillis, millisAtuais, tempoAtual);

        // Verifica se tempoAtual está no passado
        if (tempoAtual < syncedTime) {
            Serial.println("[[AVISO]]: Detectado desvio de tempo. Ajustando tempoAtual.");
            tempoAtual = syncedTime;
        }

        struct tm timeInfo;
        if (localtime_r(&tempoAtual, &timeInfo)) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
            return buffer;
        }

        Serial.println("[[ERRO]]: Timestamp inválido!");
        buffer[0] = '\0'; // Return empty string
        return buffer;
    }


};