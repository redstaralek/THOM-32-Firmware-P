//==================================================================================
//========================== Verifica string vazia ou nula =========================
//==================================================================================
bool isNotNullOrEmptyStr(String str, uint maximo = 10000){

  return str.length() > 0 && str!= NULL && str[0] != NULL && str[0] != nan("") && str.length() < maximo;

}


bool isNotNullOrEmptyStr(const char* str) {
  while (*str != '\0') {
    if (!isspace(*str)) {
      return true;
    }
    str++;
  }
  return false;
}


//==================================================================================
//==================== Converte entrada de usuário p/ boolean  =====================
//==================================================================================
bool inputToBool(String resposta){

  if(resposta.length() > 0  && (resposta.indexOf('s') >=0 || resposta.indexOf('S') >=0))
    return true;
  else if(resposta.length() > 0 && (resposta.indexOf('n') >=0 || resposta.indexOf('N') >=0))
    return false;

  Serial.println(STR_ERROR_OPCAO_INVALIDA);

  return false;

}


//==================================================================================
//========================= Regressão p/ sensores lineares =========================
//==================================================================================
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)  {
  
  // f(x) = x*(Dy/Dx) + b,        com x = _x - min
  return ((x - in_min) * ((out_max - out_min) / (in_max - in_min))) + out_min;

}


//==================================================================================
//============================== Conversão de escalas ==============================
//==================================================================================
float fahrenheitParaCelsius(float valor){

  return ( (float) valor - 32.0f) / 1.8f;

}


//==================================================================================
//=============================== Arredonda 2 casas ================================
//==================================================================================
float round2(float value) {
    return roundf(value * 100) / 100;
}


//==================================================================================
//============================= Insere float em doc ================================
//==================================================================================
void addFloatToDoc(JsonDocument& doc, const char* key, float value) {
    char buffer[10];
    dtostrf(value, 0, 2, buffer);  // Converte flt p/ str c/ 2 dígitos
    doc[key] = atof(buffer);       // Converte str em flt e põe no doc
}



//==================================================================================
//===================== Verifica se foi reset manual ou por erro ===================
//==================================================================================
bool resetNormal(){

  return esp_reset_reason() == ESP_RST_UNKNOWN 
      || esp_reset_reason() == ESP_RST_POWERON;
      
}



long getMillis(){
  return esp_timer_get_time()/1000;
}

//==================================================================================
//======================= Verifica tempo desde início do ciclo =====================
//==================================================================================
long tempoDesdeReset(){
  return getMillis() - _tempos.reset;
}



//==================================================================================
//======================= Verifica tempo desde início do ciclo =====================
//==================================================================================
uint tempoDesdeUltimoCicloEnvio(){
  return getMillis() - _tempos.cicloInicio;
}


//==================================================================================
//============================= Filtro passa-baixa =================================
//==================================================================================
float filtroPassaBaixa(float valorAtual, float valorAnterior, float alpha) {
    return alpha * valorAtual + (1 - alpha) * valorAnterior;
}


bool noIntervalo(float valor, float valorMinimo, float valorMaximo){

    if(isnan(valor))                                    { return false; }

    if     ( isnan(valorMinimo) && !isnan(valorMaximo)) { return valor <= valorMaximo; }
    else if(!isnan(valorMinimo) &&  isnan(valorMaximo)) { return valor >= valorMinimo; }
    else if( isnan(valorMinimo) &&  isnan(valorMaximo)) { return true; }

    return valor >= valorMinimo && valor <= valorMaximo;

}


void printArray(const String array[], int arraySize) {
  Serial.print("[");
  for (int i = 0; i < arraySize; i++) {
    Serial.print(array[i]);
    if (i < arraySize - 1) Serial.print(", ");
  }
  Serial.print("]");
}


// Helper lambda for safe division [Added]
auto safeDiv = [](float numerator, int denominator, float defaultVal = -1.0f) {
    return (denominator > 0) ? round2(numerator / denominator) : defaultVal;
};