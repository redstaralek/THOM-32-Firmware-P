
RunningMedian medianFilter(LEITURAS_AVG_N);

class MediaUtils{

  //==================================================================================
  //===================== Média para smoothing de erros/inconsist ====================
  //==================================================================================
  public: static uint16_t analog(byte pinToRead, float valorMinimo = nan(""), float valorMaximo = nan(""), float calibragem=1) {
    medianFilter.clear();
    float valor = 0;
    float valorFiltrado = 0;
    float alpha = 0.2;
    uint16_t counter = 0;
    for (uint8_t x = 0 ; x < LEITURAS_AVG_N ; x++){
      float _valor = analogRead(pinToRead)*calibragem;
      delay(MILISSEC_DELAY_READS);
      
      // Validação de erros/instabilidade na leitura analógica dos sensores
      if(_valor > 0 && !isnan(_valor) && noIntervalo(_valor, valorMinimo, valorMaximo)){
        valorFiltrado = filtroPassaBaixa(_valor, valorFiltrado, alpha);
        medianFilter.add(valorFiltrado);
        counter++;
      }
    }
    valor = medianFilter.getMedian();
    Serial.println("Leitura ADC (filtro + mediana): " + String(valor));
    Serial.println("Counter ADC: " + String(counter));
    return counter > 0 ? valor : nan("");
  }


  //==================================================================================
  //================================= Média angular ==================================
  //==================================================================================
  public: static uint16_t angular(float somaSeno, float somaCosseno, int qtd) {

    if(somaCosseno == 0 && somaSeno == 0)
      return 0; 
    
    float anguloAux = atan2(somaSeno/qtd, somaCosseno/qtd) * RADIAN;

    return anguloAux < 0 ? anguloAux + 360 : anguloAux;
    
  }
  
};
