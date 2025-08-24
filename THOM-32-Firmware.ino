//==================================================================================
//=============================== VARIÁVEIS GLOBAIS ================================
//==================================================================================
#include "./Headers/Definitions.h"
#include "./Headers/headersExternos.h"
#include "./Headers/headersModels.h"
//----------------------- Structs e vars globais
volatile uint16_t         _gir, _plv, _girAntes, _girMax;
volatile long             _tempoGirosMax, _tempoAntes;
static volatile Tempos    _tempos;
static LeituraTS          _timeSeries;
static ConfigEstacao      _config;
static Leitura            _leitura;
portMUX_TYPE              _muxPluv                = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE              _muxGiro                = portMUX_INITIALIZER_UNLOCKED;
bool                      _primeiro, _varMantidas = true;
//----------------------- Headers básicos em ordem
#include "./Headers/headersInternos.h"
//----------------------- Sensores Digitais
Adafruit_BME680           _bme;
Adafruit_VEML7700         _veml;
SfeAS7331ArdI2C           _as7331;
SparkFun_AS3935           _as3935(AS3935_ADDR);
static Sensores*          _sens                   = new Sensores();
SCD4x                     _scd;
SGP30                     _sgp;
bool                      _bmeOk, _as7331Ok, _vemlOk, _as3935Ok, _scdOk, _sgpOk;

//==================================================================================
//======================== INTERRUPCÕES EXT: PLUV E ANEM ===========================
//==================================================================================
void IRAM_ATTR isr_plv() {

  //--------------------- Zona crítica: Acesso unitário ao recurso compartilhado
  portENTER_CRITICAL(&_muxPluv); {
    _tempos.eventoPluv = getMillis();
    //------------------- Certeza que foi pulso novo? -> Evita oscilações/bouncing
    if (_tempos.eventoPluv - _tempos.ultimoEvPluv > ANTI_BOUNCING){
        _plv++;
        _tempos.ultimoEvPluv = _tempos.eventoPluv;
    }
  } portEXIT_CRITICAL(&_muxPluv);

}


void IRAM_ATTR isr_gir() {

  //--------------------- Zona crítica: Acesso unitário ao recurso compartilhado
  portENTER_CRITICAL(&_muxGiro); {
    _tempos.eventoGiro = getMillis();
    //------------------- Certeza que foi pulso novo? -> Evita oscilações/bouncing
    if (_tempos.eventoGiro - _tempos.ultimoEvGiro > ANTI_BOUNCING){
        _gir++;
        _tempos.ultimoEvGiro = _tempos.eventoGiro;
    }
  } portEXIT_CRITICAL(&_muxGiro);

}




//==================================================================================
//==================================== SETUP =======================================
//==================================================================================
void setup() {
  //--------------------- Watchdog Timer p/ tratar falhas e Core panic. Reseta estação se n sofrer refresh em X tempo
  {
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
  }

  //--------------------- Economia de energia (sem bluetooth, sem ADC, baixa freq que ainda possibilite wifi)
  {
    // setCpuFrequencyMhz(FREQ_CPU_MIN_SENS);
    setCpuFrequencyMhz(FREQ_CPU_MAX_WIFI);
    WiFiUtils::desligaWiFi();
    btStop();
  }

  //--------------------- Configuração de Pinagem
  {           
    pinMode(BAT_PIN, INPUT);             //  | I  | Ponta de bateria | Carga            | analógico          
    pinMode(DIR_PIN, INPUT);             //  | I  | Biruta           | Direção          | analógico          
    pinMode(VEL_PIN, INPUT_PULLDOWN);    //  | I  | Ventoinha        | Anemometria      | digital interrupt  
    pinMode(PLV_PIN, INPUT_PULLDOWN);    //  | I  | Pluv. bascular   | Pluviometria     | digital interrupt  
    adcAttachPin(DIR_PIN);
    adcAttachPin(BAT_PIN);
  }

  //--------------------- Inicia protocolos: [Serial, acesso EEPROM]
  {
    Serial.begin(SERIAL_RATE); 
    EepromUtils::iniciaEEPROM();
    delay(5000);
    Serial.flush();
  }

  //--------------------- Não perder o que estava sendo processado (caso reboot por falha de hardware)
  if(resetNormal()){
    Serial.println(STR_DEBUG_RESET_NORMAL);
    ResetUtils::resetaVariaveis(&_timeSeries, &_tempos);
    Serial.println(STR_DEBUG_CONTADORES_ESVAZIADOS);
  }else{
    Serial.println(STR_DEBUG_RESET_ESPECIAL);
    // Serial.println(STR_DEBUG_MOTIVO + String(_descricaoResetSoftware));
  }
  _tempos.reset = getMillis();
  
  //--------------------- Eventos assíncronos
  {
    // Garante não haver interrupts ativos de um reset falho
    detachInterrupt(digitalPinToInterrupt(VEL_PIN));
    detachInterrupt(digitalPinToInterrupt(PLV_PIN));
    // Insere interrupts de velocimetria e pluviometria
    attachInterrupt(digitalPinToInterrupt(VEL_PIN), isr_gir, FALLING);
    attachInterrupt(digitalPinToInterrupt(PLV_PIN), isr_plv, FALLING);
  }

  //--------------------- WiFi
  {
    // Colhe dados e configura, dps conecta (retries tratados internamente)
    WiFiUtils::configuraEstacaoRede(&_config);

    //Primeira conexão é obrigatória para sincronizar relógio interno com NTP
    // Conecta e recupera horario de servidor NTP
    if(!WiFiUtils::conectaWiFi(&_config) || !TimeUtils::conectaNtpEFallbacks()){
      ResetUtils::resetaEstacao(&_timeSeries, &_tempos, STR_ERROR_CONECTA_WIFI);
    }

    // Desconecta
    WiFiUtils::desligaWiFi();
  }

  //--------------------- Inicializa sensores
  {
    
    bool* retornoSensores = _sens->set(&_bme, &_veml, &_as7331, &_as3935, &_scd, &_sgp);
    _bmeOk                = retornoSensores[0];
    _as7331Ok             = retornoSensores[1];
    _vemlOk               = retornoSensores[2];
    _as3935Ok             = retornoSensores[3];
    _scdOk                = retornoSensores[4];
    _sgpOk                = retornoSensores[5];
    Serial.println("Retorno dos sensores [BME68X,  AS7331, VEML7700,AS3935, SCD4X, SGP30] = "+ String(_bmeOk)+", "+ String(_as7331Ok)+", "+ String(_vemlOk)+", "+ String(_as3935Ok)+", "+ String(_scdOk)+", "+ String(_sgpOk));

  }

  //--------------------- Controle
  _primeiro = true; 

  //--------------------- NTP
  {
    time_t agora = time(nullptr);
    Serial.printf("Tempo NTP (bruto): %ld\n", agora);

    struct tm timeInfo;
    localtime_r(&agora, &timeInfo);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    Serial.printf("Tempo NTP (ajustado): %s\n", buffer);
  }
  
}



//==================================================================================
//==================================== LOOP ========================================
//==================================================================================
void loop() {

  esp_task_wdt_reset();
  
  //--------------------- Colheita de dados
  {
    uint count = 0;
    while(!getDados()){
      delay(1000);
      count++;
      if(count > 4)
        ResetUtils::resetaEstacao(&_timeSeries, &_tempos, STR_ERROR_GET_DADOS);
    }
  }
   
  esp_task_wdt_reset();
  //--------------------- Espera: Pooling / Light-sleep      
  // esperaOuvindoCmdSleep() pode:
  //
  //   1.  Esperar X seg de timeout por input serial em POOLING (Ocorre caso
  //       Monitor-Web esteja ouvindo e responda à probe message -> DEBUG).
  //
  //   2.  Entrar em LIGHT-SLEEP por X seg acordando  apenas quando "ativadas 
  //       interrupções ou passados Y seg quantizados". Volta a dormir até 
  //       completar X seg (Ocorre caso a probe message não seja respondida
  //       pelo monitor -> situação normal de func. em produção, s/ monitor).
  esperaOuvindoCmdSleep(&_leitura, &_timeSeries, &_tempos, _sens, MILISSEG_POOLING);

  esp_task_wdt_reset();
  //--------------------- Envio de pacotes via WiFi
  if (tempoDesdeUltimoCicloEnvio()  >= MILISSEC_ENVIO || _primeiro) {
    // Conecta ao WiFi - retries tratados internamente
    WiFiUtils::conectaWiFi(&_config);
    // Envio de dados - retries tratados internamente
    bool sucesso = WiFiUtils::preparaEExecutaReq(&_config, &_timeSeries, &_tempos);
    // Pós envio - economia elétr + reset de variáveis
    WiFiUtils::desligaWiFi();
    
    // TODO: Desenvolver envio via SIM-Card como contingência do envio por WiFi

    ResetUtils::resetaVariaveis(&_timeSeries, &_tempos);
    _primeiro = false; 

    // Verifica saúde dos sensores após cada envio bem sucedido
    if(sucesso){
      bool* retornoSensores = _sens->garanteSensoresFuncionando();
      _bmeOk                = retornoSensores[0];
      _as7331Ok             = retornoSensores[1];
      _vemlOk               = retornoSensores[2];
      _as3935Ok             = retornoSensores[3];
      _scdOk                = retornoSensores[4];
    }
  }
  
}


//==================================================================================
//=============================== CÁLCULO DE VAR ===================================
//==================================================================================
static bool getDados() {
  
  //----------------------- Colhe e processa dados digitais
  Serial.println(STR_DEBUG_CALC_DADOS_DIGITAIS);
  {
    _leitura = Leitura();
    if (! _sens->bme->performReading()) {
      Serial.println("BME688 falhou");
      return false;
    }

    //----------- BME-688
    {
      _leitura.tmp        =  _sens->bme->temperature;
      _timeSeries.tmp     += _leitura.tmp;
      _leitura.hum        =  _sens->bme->humidity;
      _timeSeries.hum     += _leitura.hum;
      _leitura.prs        =  _sens->bme->pressure; 
      _timeSeries.prs     += _leitura.prs;
      _leitura.gas        =  _sens->bme->gas_resistance / 1000.0; 
      _timeSeries.gas     += _leitura.gas;
    }

    //----------- VEML7700
    if(_vemlOk){
      long _lxAux       =  _sens->veml->readLux(VEML_LUX_AUTO);
      Serial.println("Lx="+String(_lxAux));
      _leitura.lx      =  ((float)_lxAux)/1000;
      _timeSeries.lx   += _leitura.lx;
      _timeSeries.qtdLx++;
    }

    //----------- AS7331
    if(_sens->leiturasAs7331Prontas()){

      Serial.println("Leituras de 7331 prontas!");
      _leitura.uva        =  _sens->as7331->getUVA()/1000;
      _timeSeries.uva     += _leitura.uva;
      _leitura.uvb        =  _sens->as7331->getUVB()/1000;
      _timeSeries.uvb     += _leitura.uvb;
      _leitura.uvc        =  _sens->as7331->getUVC()/1000;
      _timeSeries.uvc     += _leitura.uvc;
      Serial.println("UVA, UVB, UVC (Bruto) = "      + String(_leitura.uva) + ", " + String(_leitura.uvb) + ", " + String(_leitura.uvc));
      _timeSeries.qtdUVBruto++;
      
    }

    //----------- AS3935
    if(_sens->as3935->readInterruptReg() == LIGHTNING_INT){
      _leitura.rdis         += _sens->as3935->distanceToStorm();
      _timeSeries.rdis      += _leitura.rdis;
      _leitura.rpot         += _sens->as3935->lightningEnergy();
      _timeSeries.rpot      += _leitura.rpot;
      _timeSeries.qtdR++;
      Serial.println("Raio detectado (a " + String(_leitura.rdis)+" km, pot=" + String(_leitura.rpot)+")");
    }

    //----------- SCD4X
    if(_sens->scd->getDataReadyStatus()){
      uint16_t  scdCo2 = _sens->scd->getCO2();
      float     scdTmp = _sens->scd->getTemperature();
      float     scdHum = _sens->scd->getHumidity();
      
      Serial.print("[CO2] = "+ String(scdCo2) + " PPM, ");
      Serial.print("tmp2 = " + String(scdTmp) + " °C, ");
      Serial.print("hum2 = " + String(scdHum) + "%\n");

      if(scdCo2 >= 300){
        _leitura.co2        =  scdCo2;
        _timeSeries.co2     += _leitura.co2;
        
        _timeSeries.qtdCo2++;
      }
    }

    // //----------- SGP30
    // if(_sens->sgp->measureAirQuality() == SGP30_SUCCESS){
    //   uint eCo2 = _sens->sgp->CO2;
    //   uint tvoc = _sens->sgp->TVOC;
      
    //   Serial.print("[eCO2] = "+ String(eCo2) + " PPM, ");
    //   Serial.print("[TVOC] = "+ String(tvoc) + " PPB. \n");

    //   if(eCo2 >= 390){
    //     _leitura.eCo2        =  eCo2;
    //     _timeSeries.eCo2     += _leitura.co2;

    //     _leitura.tvoc        =  tvoc;
    //     _timeSeries.tvoc     += _leitura.tvoc;
        
    //     _timeSeries.qtdTvoc++;
    //   }
    // }

    //----------- Internos (ESP)
    {
      _leitura.tmpCpu     =  fahrenheitParaCelsius(temprature_sens_read());
      _timeSeries.tmpCpu  += _leitura.tmpCpu;
      _leitura.hal        =  hallRead();
      _timeSeries.hal     += _leitura.hal;
    }

    //----------- Health Check (sensores)
    {
      _timeSeries.bmeOk   = _sens->bmeOk;
      _timeSeries.as7331Ok= _sens->as7331Ok;
      _timeSeries.vemlOk  = _sens->vemlOk;
      _timeSeries.as3935Ok= _sens->as3935Ok;
      _timeSeries.scdOk   = _sens->scdOk;
    }

  }

  //----------------------- Colhe e processa dados analógicos
  {

    //--------- Tensão na bateria ([2*] pelos dois resistores iguais ~> lei de Ohm)
    Serial.println(STR_DEBUG_CALC_BAT);
    float calibragemBat = 1.1027;//((float) 2300 / (float) 2050);
    _leitura.bat       =  2 * (_3V3 * MediaUtils::analog(BAT_PIN, BAT_ADC_MIN, BAT_ADC_MAX, calibragemBat) / ADC_MAX);
    // Serial.println(_leitura.bat);
    if (!isnan(_leitura.bat) && _leitura.bat >= BAT_MIN && _leitura.bat <= BAT_MAX){
      _timeSeries.bat  += _leitura.bat;
      _timeSeries.qtdBat  ++;
    }

    //--------- Cálculo direção eólica
    Serial.println(STR_DEBUG_CALC_DIR);
    float direcao         = 0;
    float mediaAnalogAnem = MediaUtils::analog(DIR_PIN, DIR_ADC_MIN, DIR_ADC_MAX);
    // Serial.println(mediaAnalogAnem);
    float vOutDirecao     = (_3V3 * mediaAnalogAnem / ADC_MAX);
    // Serial.println(vOutDirecao);

    //--------- Verifica modelo da biruta e escolhe entre SVDV10 ou DV10
    if(_config.modeloBiruta == "1"){
      // USINAINFO SVDV10
      //  Vide  https://www.usinainfo.com.br/estacao-meteorologica-arduino/anemometro-arduino-indicador-de-direcao-do-vento-para-estacao-meteorologica-svdv10-8353.html
      //        https://www.usinainfo.com.br/index.php?controller=attachment&id_attachment=884
      //
      // PINAGEM: 3 pinos.
      //  Vide https://produto.mercadolivre.com.br/MLB-3376279281-anemmetro-arduino-indicador-de-direco-do-vento-para-e-_JM#position=4&search_layout=stack&type=item&tracking_id=10b9bcc4-ceba-4abe-b52d-57838200dfb9
      //        1- Superior  -> Gnd
      //        2- Meio      -> Vcc (5V)
      //        3- Inferior  -> Dado

      float fatorDeflacao = 0.6666f;
      if      (vOutDirecao <= 0.27) { direcao = 315; }
      else if (vOutDirecao <= 0.32) { direcao = 270; }
      else if (vOutDirecao <= 0.38) { direcao = 225; }
      else if (vOutDirecao <= 0.45) { direcao = 180; }
      else if (vOutDirecao <= 0.57) { direcao = 135; }
      else if (vOutDirecao <= 0.75) { direcao = 90;  }
      else if (vOutDirecao <= 1.25) { direcao = 45;  }
      else                          { direcao = 0;   }
    }else if(_config.modeloBiruta == "2"){
      // USINAINFO DV10
      // Vide   https://www.usinainfo.com.br/estacao-meteorologica-arduino/indicador-de-direcao-do-vento-arduino-para-estacao-meteorologica-dv10-4638.html
      //        https://www.usinainfo.com.br/blog/indicador-de-direcao-do-vento-com-arduino-melhorando-sua-estacao-meteorologica/
      // PINAGEM: 2 pinos.
      //      1- Superior   -> Vcc
      //      2- Inferior   -> Sinal com resistor pulldown
      if      (vOutDirecao <= 0.26) { direcao = 315; }
      else if (vOutDirecao <= 0.30) { direcao = 270; }
      else if (vOutDirecao <= 0.35) { direcao = 225; }
      else if (vOutDirecao <= 0.42) { direcao = 180; }
      else if (vOutDirecao <= 0.51) { direcao = 135; }
      else if (vOutDirecao <= 0.66) { direcao = 90;  }
      else if (vOutDirecao <= 0.94) { direcao = 45;  }
      else                          { direcao = 0;   }
      
    }else{
      // Biruta inválida, reseta estação descartando variáveis (ciclo de coleta incompleto)
      // ResetUtils::resetaEstacao(&_timeSeries, &_tempos, MSG_RESET_BIRUTA_INVAL);
      Serial.println(MSG_RESET_BIRUTA_INVAL);
    }
    _timeSeries.sen += sin(direcao/RADIAN);
    _timeSeries.cos += cos(direcao/RADIAN);
  }

  //----------------------- Colhe e processa min/max
  {

    //--------- Cálculo velocidade eólica máx
    Serial.println(STR_DEBUG_CALC_VEL);
    float girosDif   = _gir - _girAntes;
    _girAntes      = _gir;

    if (girosDif > _girMax){  
      _girMax      = girosDif;
      _tempoGirosMax = getMillis() - _tempoAntes;
    }
    _tempoAntes = getMillis();

  }
  
  //----------------------- Finalizações
  {
    _timeSeries.qtd++;

    //--------- Check
    Serial.println(STR_DEBUG_FIM_COLETA);
    DebugUtils::printaLeituras(&_leitura, &_timeSeries, &_tempos, _varMantidas);
  }

  return ValidateUtils::checkDados(_leitura.tmp, _leitura.hum, _leitura.prs, true);

}
