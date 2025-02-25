
unsigned long   lastSyncMillis = 0;
time_t          syncedTime = 0;

class TimeUtils{
    

    //==================================================================================
    //======================= Conecta ao Servidor NTP e Fallbacks ======================
    //==================================================================================
    public: static bool conectaNtpEFallbacks() {
        String servidoresNtp[] = NTP_SERVERS;
        uint8_t qtdServidores = sizeof(servidoresNtp) / sizeof(String);

        for (uint8_t i = 0; i < qtdServidores; i++) {
            configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, servidoresNtp[i].c_str());
            delay(5000); // Espera para sincronizar
            for(uint8_t j = 0; j < 10; j++){
                time_t agora = time(nullptr);
                if (agora > 1000000000) { // Verifica se o timestamp é válido (ex.: pós-2001)
                    syncedTime = agora;
                    lastSyncMillis = getMillis();
                    Serial.printf("Sincronização NTP bem-sucedida! syncedTime: %ld\n", syncedTime);
                    return true;
                } else {
                    Serial.printf("Servidor NTP retornou tempo inválido (%ld).", agora);
                }
            }
            Serial.printf("Servidor NTP retornou tempo inválido muitas vezes. Tentando o próximo...\n");
        }

        return false; // Falha em todos os servidores
    }



    //==================================================================================
    //========================== Pega horário do Servidor NTP ==========================
    //==================================================================================
    public: static String getTime() {
        unsigned long millisAtuais = getMillis();
        time_t tempoAtual = syncedTime + (millisAtuais - lastSyncMillis) / 1000;

        Serial.printf("syncedTime: %ld, lastSyncMillis: %lu, millisAtuais: %lu, tempoAtual: %ld\n",
                    syncedTime, lastSyncMillis, millisAtuais, tempoAtual);

        // Verifica se tempoAtual está no passado
        if (tempoAtual < syncedTime) {
            Serial.println("[[AVISO]]: Detectado desvio de tempo. Ajustando tempoAtual.");
            tempoAtual = syncedTime;
        }

        struct tm timeInfo;
        if (localtime_r(&tempoAtual, &timeInfo)) {
            char buffer[20];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
            return String(buffer);
        }

        Serial.println("[[ERRO]]: Timestamp inválido!");
        return "";
    }


};