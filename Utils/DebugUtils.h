
class DebugUtils{

  public: static void printaLeituras(Leitura *atual, LeituraTS *ts, volatile Tempos *tempos, bool var_mantidas) {
    
    Serial.println(STR_DEBUG_LEITURAS);

    if(ts->qtd > 0){
      //---------------------- [Tmp, Hum, Prs, UV, Ilum, Cpu temp, Bat]
      Serial.println(STR_DEBUG_PRINT_INSTANT);
      Serial.println(STR_DEBUG_PRINT_INST_TMP + String(atual->tmp));
      Serial.println(STR_DEBUG_PRINT_INST_HUM + String(atual->hum)); 
      Serial.println(STR_DEBUG_PRINT_INST_PRS + String(atual->prs)); 
      Serial.println(STR_DEBUG_PRINT_INST_UV  + String(atual->uva));
      Serial.println(STR_DEBUG_PRINT_INST_LX  + String(atual->lx));
      Serial.println(STR_DEBUG_PRINT_INST_BAT + String(atual->bat));
      Serial.println(STR_DEBUG_PRINT_INST_CO2 + String(atual->co2));
      Serial.println(STR_DEBUG_PRINT_INST_GAS + String(atual->gas));

      //---------------------- Json (médias, mins, max, )
      Serial.println(STR_DEBUG_PRINT_JSON);
      Serial.println("[[DEBUG]] " + SerializationUtils::getLeituraJson(ts, tempos, false));
    }else{
      Serial.println(STR_DEBUG_1A_LEITURA_NAO_COLHIDA);
    }
  }
  
};