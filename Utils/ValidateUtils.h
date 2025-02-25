class ValidateUtils{
  //==================================================================================
  //=================== Verifica validade das credenciais de rede  ===================
  //==================================================================================
  public: static bool credenciaisValidas(ConfigEstacao *config){

    return isNotNullOrEmptyStr(config->url,          255)
        && isNotNullOrEmptyStr(config->token,        255) 
        && isNotNullOrEmptyStr(config->senha,        255) 
        && isNotNullOrEmptyStr(config->ssid,         255) 
        && isNotNullOrEmptyStr(config->modeloBiruta, 255);

  }


  //==================================================================================
  //======================= Verifica usuário para redes PEAP  ========================
  //==================================================================================
  public: static bool possuiUser(ConfigEstacao *config){

    return isNotNullOrEmptyStr(config->user, 255);

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