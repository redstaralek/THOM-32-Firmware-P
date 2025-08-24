class ValidateUtils{
  //==================================================================================
  //=================== Verifica validade das credenciais de rede  ===================
  //==================================================================================
  public: static bool credenciaisValidas(ConfigEstacao *config){

    // Pelo menos um SSID válido
    bool ssidOk = false;
    for (const auto& ssid : config->ssid) {
      if (ssid.length() > 0) {
        ssidOk = true;
        break;
      }
    }

    // Pelo menos uma senha válida
    bool senhaOk = false;
    for (const auto& senha : config->senha) {
      if (senha.length() > 0) {
        senhaOk = true;
        break;
      }
    }

    // Os demais campos continuam únicos
    return (ssidOk && senhaOk)
        && isNotNullOrEmptyStr(config->url, 255)
        && isNotNullOrEmptyStr(config->token, 255)
        && isNotNullOrEmptyStr(config->modeloBiruta, 255);

  }
  
  
  //==================================================================================
  //================================== Check Dado ====================================
  //==================================================================================
  public: static bool checkDado(float val, float min, float max, bool verbose, char* warning){

    bool ok = noIntervalo(val, min, max);

    if(verbose && !ok)
      Serial.printf("%s (%.1f. Limites = [%.1f, %.1f])\n", warning, val, min, max);

    return ok;

  }


  //==================================================================================
  //============================ Check [Tmp, Hum, Pres]  =============================
  //==================================================================================
  public: static bool checkDados(float tmp, uint16_t hum, uint32_t prs, bool verbose){
    bool okTmp = checkDado(tmp, TMP_MIN, TMP_MAX, verbose, STR_WARNING_TMP);
    bool okHum = checkDado(hum, HUM_MIN, HUM_MAX, verbose, STR_WARNING_HUM);
    bool okPrs = checkDado(prs, PRS_MIN, PRS_MAX, verbose, STR_WARNING_PRS);
    return  okTmp && okHum && okPrs;
    
  }

};