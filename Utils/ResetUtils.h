class ResetUtils{
  
  //==================================================================================
  //========================= Reseta variável de acúmulo =============================
  //==================================================================================
  private: static void _resetaTimeSeries(LeituraTS *ts){  

    //---------------------- Acumulados (p/ cálculo de média)
    Serial.println(STR_DEBUG_RESET_MED);
    ts->qtdBat         = 0;              
    ts->qtdUVBruto     = 0;
    ts->qtdLx          = 0;
    ts->qtdR           = 0;     
    ts->qtdCo2         = 0;
    ts->qtdTvoc        = 0;
    ts->qtd            = 0;   
    ts->tmp            = 0;      
    ts->hum            = 0;     
    ts->prs            = 0;      
    ts->hal            = 0;
    ts->uva            = 0;
    ts->uvb            = 0;
    ts->uvc            = 0;
    ts->lx             = 0;      
    ts->bat            = 0;  
    ts->gas            = 0;
    ts->rdis           = 0;  
    ts->rpot           = 0;  
    ts->co2            = 0;
    ts->eCo2           = 0;
    ts->tvoc           = 0;
    ts->tmpCpu         = 0;  
    ts->sen            = 0;     
    ts->cos            = 0;
    // bmeOk, as7331Ok, vemlOk, as3935Ok e scdOk   
    // são alterados no reset dos sensores

  }


  //==================================================================================
  //=================== Limpa variáveis metereológicas e de controle =================
  //==================================================================================
  public: static void resetaVariaveis(LeituraTS *ts, volatile Tempos *tempos) {

    //---------------------- Reseta série temporal
    _resetaTimeSeries(ts);

    //---------------------- PLUVIOM. ->  Zona crítica: Acesso unitário ao recurso
    Serial.println(STR_DEBUG_RESET_PLUV);
    portENTER_CRITICAL(&_muxPluv); {
      _plv                = 0;     
      tempos->ultimoEvPluv = getMillis(); 
      tempos->eventoPluv   = getMillis();
    } portEXIT_CRITICAL(&_muxPluv);

    //---------------------- ANEMOM.  ->  Zona crítica: Acesso unitário ao recurso
    Serial.println(STR_DEBUG_RESET_ANEM);
    portENTER_CRITICAL(&_muxGiro); {
      _gir                 = 0; 
      tempos->ultimoEvGiro = getMillis();
      tempos->eventoGiro   = getMillis();
    } portEXIT_CRITICAL(&_muxGiro);
    _girAntes              = 0; 
    _girMax                = 0; 
    _tempoGirosMax         = 0;
    _tempoAntes            = getMillis();

    //---------------------- Reseta controles
    Serial.println(STR_DEBUG_RESET_AUX);
    tempos->cicloInicio    = getMillis();
    tempos->tempoDormido   = 0;
    _varMantidas           = false;
    // strcpy(_descricaoResetSoftware, "");

  }


  //==================================================================================
  //======================== Funções de tolerância a falhas ==========================
  //==================================================================================
  public: static void resetaEstacao(LeituraTS *ts, volatile Tempos *tempos, String mensagem="", bool mantemVariaveis=false){

    Serial.println(mensagem);

    // strncpy(_descricaoResetSoftware, mensagem.c_str(), sizeof(_descricaoResetSoftware)-1);
    // _descricaoResetSoftware[sizeof(_descricaoResetSoftware) - 1] = '\0';
    
    // Serial.println(_descricaoResetSoftware);

    if(!mantemVariaveis)
      resetaVariaveis(ts, tempos);
    if(!isNotNullOrEmptyStr(mensagem))
      Serial.println(STR_DEBUG_RESET_MSG);
    esp_restart();

  }


  //==================================================================================
  //============================ Deep sleep  temporizado =============================
  //==================================================================================
  public: static void deep_sleep(long tempo, String mensagem=""){

    // if(!isNotNullOrEmptyStr(mensagem)){
    //   Serial.println(STR_DEBUG_RESET_MSG);
    //   strncpy(_descricaoResetSoftware, mensagem.c_str(), sizeof(_descricaoResetSoftware)-1);
    //   _descricaoResetSoftware[sizeof(_descricaoResetSoftware) - 1] = '\0';
    // }
    esp_sleep_enable_timer_wakeup(tempo * NANO_FACTOR);
    Serial.flush();
    esp_deep_sleep_start();
    
  }

};



