//==================================================================================
//============================== Sensores Digitais =================================
//==================================================================================

#define STATE_SAVE_PERIOD	UINT32_C(5 * 60 * 1000)

class Sensores{
    
    public: Adafruit_BME680     *bme;
    public: SfeAS7331ArdI2C     *as7331;
    public: SparkFun_AS3935     *as3935;
    public: Adafruit_VEML7700   *veml;
    public: SCD4x               *scd;
    public: SGP30               *sgp;
    public: bool bmeOk   ;
    public: bool vemlOk  ;
    public: bool as7331Ok;
    public: bool as3935Ok;
    public: bool scdOk   ;
    public: bool sgpOk   ;
    
    //==================================================================================
    //============================ Set dos sensores I²C ================================
    //==================================================================================
    public: bool* set(Adafruit_BME680 *bme, Adafruit_VEML7700 *veml, SfeAS7331ArdI2C *as7331, SparkFun_AS3935 *as3935, SCD4x *scd, SGP30 *sgp){
        this->bme       = bme;
        this->veml      = veml;
        this->as7331    = as7331;
        this->as3935    = as3935;
        this->scd       = scd;
        this->sgp       = sgp;
        return this->inicializaSensores();
    }
    

    //==================================================================================
    //=========================== Inicializa Sensores I²C ==============================
    //==================================================================================
    private: bool* inicializaSensores(){

        bmeOk      = false;
        vemlOk     = false;
        as7331Ok   = false;
        as3935Ok   = false;
        scdOk      = false;
        sgpOk      = false;

        Wire.begin();
        // Wire.setClock(50000);
        delay(1000);

        //--------------------- BME680
        {
            bmeOk = this->inicializaEVerificaBme680();
            delay(500);
        }
        //--------------------- AS7331
        {
            as7331Ok = this->inicializaEVerificaAS7331();
            delay(500);
        }

        //--------------------- MAS440009 (VEML7700)
        {
            vemlOk = this->inicializaEVerificaVEML7700();
            delay(500);
        }

        //--------------------- AS3935
        {
            as3935Ok = this->inicializaEVerificaAS3935();
            delay(500);
        }

        //--------------------- SCD4X
        {
            scdOk = this->inicializaEVerificaSCD4X();
            delay(500);
        }

        // //--------------------- SGP30
        // {
        //     sgpOk = this->inicializaEVerificaSGP30();
        //     delay(500);
        // }

        return new bool[6] {bmeOk, as7331Ok, vemlOk, as3935Ok, scdOk, sgpOk};

    }


    private: bool inicializaEVerificaVEML7700(){
        int counter = 0;
        bool funcionando = false;
        while(!funcionando && counter < 5){

            if(!veml->begin()){
                Serial.println("ERRO - [VEML7700] não encontrado!");
            }else{
                Serial.println("OK - [VEML7700] encontrado em 0x10!");
                funcionando = true;
            }

            delay(1000);
            counter++;
        }
        veml->setIntegrationTime(VEML7700_IT_800MS);
        veml->powerSaveEnable(false);
        return funcionando;
    }


    private: bool inicializaEVerificaBme680(){
        int counter = 0;
        bool functionando = false;
        Serial.println("Procurando BME680");
        while(!functionando && counter < 5){

            if (!bme->begin(0x76)) {
                if (!bme->begin(0x77)) {
                    Serial.println("ERRO - [BME680] não encontrado!");
                }else{
                    functionando = true;
                    Serial.println("OK - [BME680] encontrado em 0x77!");
                }
            }else{
                functionando = true;
                Serial.println("OK - [BME680] encontrado em 0x76!");
            }
            delay(1000);
            counter++;
        }

        bme->setTemperatureOversampling(BME68X_OS_16X);
        bme->setHumidityOversampling(BME68X_OS_16X);
        bme->setPressureOversampling(BME68X_OS_16X);
        bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme->setGasHeater(300, 200);
        return functionando;
    }


    private: bool inicializaEVerificaAS7331(){
        
        // as7331->reset();
        // delay(4000);
        int counter = 0;
        bool funcionando = false;
        while(!funcionando && counter < 5){

            if(!as7331->begin()){
                Serial.println("ERRO - [AS7331] não encontrado!");
            }else{
                Serial.println("OK - [AS7331] encontrado em 0x74!");
                if(as7331->prepareMeasurement(MEAS_MODE_CMD) == false){
                    Serial.println("ERRO - [AS7331] não conseguiu gerar leituras!");
                }else{
                    funcionando = true;
                }
                delay(3000);
                leiturasAs7331Prontas();
            }

            delay(1000);
            counter++;
        }
        return funcionando;
    }


    private: bool inicializaEVerificaAS3935(){

        int counter = 0;
        bool funcionando = false;

        while(!funcionando && counter < 10){

            if(!as3935->begin()){
                Serial.println("ERRO - [AS3935] não encontrado!");
            }
            else{
                Serial.println("OK - [AS3935] encontrado em 0x03!");
                funcionando = true;
            }

            delay(1000);
            counter++;
        }
        
        as3935->setIndoorOutdoor(OUTDOOR);
        as3935->setNoiseLevel(2);
        as3935->watchdogThreshold(2);
        as3935->spikeRejection(2);
        delay(1000);
        return funcionando;

    }


    private: bool inicializaEVerificaSCD4X(){
        
        int counter = 0;
        bool funcionando = false;

        while(!funcionando && counter < 5){

            if(!scd->begin()){
                Serial.println("ERRO - [SCD4X] não encontrado!");
            }
            else{
                Serial.println("OK - [SCD4X] encontrado em 0x62!");
                funcionando = true;
            }

            delay(1000);
            counter++;
        }

        ativaEOuCalibraScd();

        return funcionando;

    }

    private: bool inicializaEVerificaSGP30(){
        
        int counter = 0;
        bool funcionando = false;

        while(!funcionando && counter < 5){
            
            if(!sgp->begin()){
                Serial.println("ERRO - [SGP30] não encontrado!");
            }
            else{
                Serial.println("OK - [SGP30] encontrado em 0x58!");
                funcionando = true;
            }

            delay(1000);
            counter++;
        }

        sgp->initAirQuality();
        delay(1000);
        return funcionando;

    }


    public: bool ativaEOuCalibraScd(bool calibra = false){
        
        bool sucessoCalib = false;

        if(calibra){
            Serial.setTimeout(1000*60);
            Serial.flush();
            Serial.println("[[INPUT]]>> INPUT: Insira o valor em PPM p/ [CO2] calibrado: ");
            int ppmRef = Serial.parseInt();
            scd->stopPeriodicMeasurement();
            delay(750);
            sucessoCalib = (scd->performForcedRecalibration(ppmRef) == 0x7fff);
            Serial.println("Calibragem Forçada="+String(sucessoCalib));
            inicializaEVerificaSCD4X();
        }
        delay(1000);
        scd->setTemperatureOffset(0);
        scd->setSensorAltitude(1160);

        Serial.println("bmeOk="+String(bmeOk));
        if(bmeOk && bme->performReading()){
            int bmePres = bme->pressure;
            Serial.println("bmePres="+String(bmePres));
            if(bmePres >= PRS_MIN){
                Serial.println("Definindo pressão SCD41 como "+String(bmePres)+"Pa.");
                bool pressaoSetada = scd->setAmbientPressure(bmePres);
                String sucessoStr = (pressaoSetada ? "Ok" : "Erro");
                Serial.println("Pressão do SCD41 definida com sucesso = "+sucessoStr+".");
            }
        }
        delay(1000);

        return sucessoCalib;
    }

    public: bool resetaScd(){
        if(scd->stopPeriodicMeasurement()){
            delay(750);
            bool sucesso = scd->performFactoryReset();
            Serial.println("Reset do SCD41 feito com sucesso = "+String(sucesso)+".");
            inicializaEVerificaSCD4X();
            return sucesso;
        }else{
            Serial.println("Não foi possível parar a leitura periódica. Tentar novamente.");
        }
        
    }


    //==================================================================================
    //========================== Utilitários dos Sensores ==============================
    //==================================================================================

    public: bool leiturasAs7331Prontas(){
        
        if(as7331->setStartState(true) != kSTkErrOk ){
            Serial.println("Erro ao iniciar leitura de AS7331.");
            return false;
        }else{
            delay(1000 + as7331->getConversionTimeMillis());
            if(as7331->readAllUV() != kSTkErrOk){
                Serial.println("Erro ao ler UV a partir do AS7331.");
                return false;
            }else{
                return true;
            }
        }

    }


    public: bool leiturasAs7331Validas(float uva, float uvb, float uvc){
        return leiturasAs7331Positivas(uva, uvb, uvc)
            && noIntervalo(uva, UVA_MIN, UVA_MAX)
            && noIntervalo(uvb, UVB_MIN, UVB_MAX)
            && noIntervalo(uvc, UVC_MIN, UVC_MAX);
    }
    
    public: bool leiturasAs7331Positivas(float uva, float uvb, float uvc){
        return uva > 0 || uvb > 0 || uvc > 0;
    }

    public: bool* garanteSensoresFuncionando(){
        bmeOk   = garanteBME688Funcionando();
        as7331Ok   = garanteAS7331Funcionando();
        vemlOk = garanteVEML7700Funcionando();
        as3935Ok   = garanteAS3935Funcionando();
        scdOk    = garanteSCD4XFuncionando();
        // sgpOk      = garanteSGP30Funcionando();
        Serial.println("[BME688, AS7331, VEML7700, AS3935, SCD4X, SGP30] = ["+String(bmeOk)+", "+String(as7331Ok)+", "+String(vemlOk)+", "+String(as3935Ok)+", "+String(scdOk)+", "+String(sgpOk)+"]");
        return new bool[6]{bmeOk, as7331Ok, vemlOk, as3935Ok, scdOk, sgpOk};
        
    }
    
    private: bool noIntervalo(float valor, float valorMinimo, float valorMaximo){

        if(isnan(valor))                                    { return false; }

        if     ( isnan(valorMinimo) && !isnan(valorMaximo)) { return valor <= valorMaximo; }
        else if(!isnan(valorMinimo) &&  isnan(valorMaximo)) { return valor >= valorMinimo; }
        else if( isnan(valorMinimo) &&  isnan(valorMaximo)) { return true; }

        return valor >= valorMinimo && valor <= valorMaximo;

    }

    public: bool garanteBME688Funcionando(){
        if(!bme->performReading() || bme->humidity <= 0 || bme->pressure <= 3000  || bme->pressure >= 100000){
            delay(1000);
            return inicializaEVerificaBme680();
        }
        return true;
    }

    public: bool garanteAS7331Funcionando(){
        if (as7331->readAllUV() != kSTkErrOk || isnan(as7331->getUVC()) || as7331->getUVC() > 20000 ) {
            Serial.println("Erro no AS7331 detectado. Reiniciando o sensor");
            return inicializaEVerificaAS7331();
        }
        return true;
    }

    public: bool garanteVEML7700Funcionando(){
        // Todo código que tenta verificar o sensor nessa lib acaba causando core-panic. Melhor só inicializar de uma vez.
        // return inicializaEVerificaVEML7700();
        return true;
    }

    public: bool garanteAS3935Funcionando() {

        if (as3935->readIndoorOutdoor() != OUTDOOR) {
            Serial.println("Erro no AS3935 detectado. Reiniciando o sensor");
            return inicializaEVerificaAS3935();
        }

        return true;
    }

    public: bool garanteSCD4XFuncionando(){
        
        Serial.println("SCD self test="+scd->performSelfTest());
        if(scd->performSelfTest()){
            Serial.println("Erro no Scd4x detectado. Reiniciando o sensor");
            scd->reInit();
            delay(1000);
            return inicializaEVerificaSCD4X();
        }else{
            Serial.println("Dado do Scd4x ainda não recebido...");
            // return inicializaEVerificaSCD4X();
        }
        return true;
    }


    public: bool garanteSGP30Funcionando(){
        
        SGP30ERR error = sgp->measureTest();
        if(error == SGP30_ERR_BAD_CRC){
            Serial.println("Erro no SGP30 detectado (CRC falhou). Reiniciando o sensor.");
            return inicializaEVerificaSGP30();
        }else if(error == SGP30_ERR_I2C_TIMEOUT){
            Serial.println("Erro no SGP30 detectado (timeout). Reiniciando o sensor.");
            return inicializaEVerificaSGP30();
        }else if(error == SGP30_SELF_TEST_FAIL){
            Serial.println("Erro no SGP30 detectado (falha de teste). Reiniciando o sensor.");
            return inicializaEVerificaSGP30();
        }
        return true;

    }



};
