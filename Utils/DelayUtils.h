//==================================================================================
//====================== Light sleep wake up em [PLUV e VEL] =======================
//==================================================================================
static void  lightSleepVelPluv(uint tempoADormir, volatile Tempos *tempos){
  
  // Print + tempo de exibição
  Serial.println(STR_DEBUG_ENTRA_LIGHT_SL);
  delay(200);             
  // Controle de light-sleep quantizado por N parcelas de "MICROS_LIGHT_SL_QUANTUM"
  long tempoDormidoNaEspera = 0;
  while(tempoDormidoNaEspera <= tempoADormir){
    // Eventos de Wake-Up!
    {
      esp_sleep_enable_timer_wakeup(MICROS_LIGHT_SL_QUANTUM);   // Acordará por temporizador (us) quantizado
      if(digitalRead(VEL_PIN) == HIGH)                          // Anemometro parado no ponto de high (vel=0)     -> ouve [PLUV].
        esp_sleep_enable_ext0_wakeup(PLV_GPIO, HIGH);   
      else
        esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMAP, 
                                     ESP_EXT1_WAKEUP_ANY_HIGH); // Acordará p/ pinos de WAKEUP_PIN_BITMAP em alto -> ouve [PLUV, VEL].
    }
    // Metragem do tempo
    {
      tempos->eventoSleep = getMillis();                           // Registra tempo anterior
      esp_light_sleep_start();                                  // Inicia light sleep
      long tempoDesteSleep = getMillis() - tempos->eventoSleep;    // Acordou -> qto tempo se passou?
      tempoDormidoNaEspera += tempoDesteSleep;                  // Incrementa acumulador local da espera
      Serial.println("[[DEBUG]]>>(+"+String(tempoDesteSleep)
                        +", de" + String(tempoDormidoNaEspera)+")");
    }
  }
  tempos->tempoDormido += tempoDormidoNaEspera;                 // Incrementa acumulador global do ciclo"
  esp_task_wdt_reset();
}


//==================================================================================
//====================== Verifica se existe terminal web  ==========================
//==================================================================================
static bool usbConectadoATerminalResponsivo(uint tempoVerificacaoUsb){

  Serial.setTimeout(tempoVerificacaoUsb);
  Serial.println(STR_DEBUG_PROBE_POINT + String(tempoVerificacaoUsb/1000) + "\n");
  return Serial.readString().length() > 0;

}


 
//==================================================================================
//========================== Pooling atento a comandos  ============================
//==================================================================================
void esperaOuvindoCmd(Leitura *leitura, LeituraTS *ts, volatile Tempos *tempos, Sensores *sens, uint tempoEspera){

  // Espera
  Serial.setTimeout(tempoEspera);
  Serial.flush();
  Serial.println(STR_INFO_TEMPO_COMANDO + String(tempoEspera/1000));
  // Interpreta comandos
  String valor = Serial.readStringUntil('\n');
  if(isNotNullOrEmptyStr(valor) && valor.indexOf("[[RESTART]]") >= 0){
    // Reset
    ResetUtils::resetaEstacao(ts, tempos, STR_INFO_CMD_RESTART);
  }else if(isNotNullOrEmptyStr(valor) && valor.indexOf("[[PRINT VAR]]") >= 0){
    // Print
    DebugUtils::printaLeituras(leitura, ts, tempos, _varMantidas); 
  }else if(isNotNullOrEmptyStr(valor) && valor.indexOf("[[CO2]]") >= 0){
    // Calibra CO2
    sens->ativaEOuCalibraScd(true);
  }else if(isNotNullOrEmptyStr(valor) && valor.indexOf("[[CO2 RESET]]") >= 0){
    // Calibra CO2
    sens->resetaScd();
  }
  // Passa

  esp_task_wdt_reset();
}


//==================================================================================
//====================== Pooling atento a comandos OU sleep ========================
//==================================================================================
void esperaOuvindoCmdSleep(Leitura *leitura, LeituraTS *ts, volatile Tempos *tempos, Sensores *sens, uint tempoEspera){

  Serial.flush();
  //Verifica retorno do console          -> usb conectado? 
  //Nosso console web sempre retorna um valor de confirmação.
  bool usbConnected = usbConectadoATerminalResponsivo(MILIS_VERIFICACAO_USB);

  //Caso não esteja -> está em produção  -> light sleep não-monitorado!
  if(!usbConnected)
    lightSleepVelPluv(tempoEspera - MILIS_VERIFICACAO_USB, tempos);

  //Caso esteja     -> terminal web      -> pooling monitorado!
  else if(tempoEspera > MILIS_VERIFICACAO_USB && usbConnected)
    esperaOuvindoCmd(leitura, ts, tempos, sens, tempoEspera - MILIS_VERIFICACAO_USB);
  
}