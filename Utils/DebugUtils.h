
class DebugUtils{

  public: static void printaLeituras(Leitura *atual, LeituraTS *ts, volatile Tempos *tempos, bool var_mantidas) {
    
    Serial.println(STR_DEBUG_LEITURAS);

    if(ts->qtd > 0){
      //---------------------- [Tmp, Hum, Prs, UV, Ilum, Cpu temp, Bat]
      Serial.print(STR_DEBUG_PRINT_INSTANT);
      Serial.printf("[%s%.2f,",  STR_DEBUG_PRINT_INST_TMP, atual->tmp);
      Serial.printf("%s%d,",    STR_DEBUG_PRINT_INST_HUM, atual->hum);
      Serial.printf("%s%d,",    STR_DEBUG_PRINT_INST_PRS, atual->prs);
      Serial.printf("%s%.2f,",  STR_DEBUG_PRINT_INST_UV,  atual->uva);
      Serial.printf("%s%.2f,",  STR_DEBUG_PRINT_INST_LX,  atual->lx );
      Serial.printf("%s%.2f,",  STR_DEBUG_PRINT_INST_BAT, atual->bat);
      Serial.printf("%s%.0f,",  STR_DEBUG_PRINT_INST_CO2, atual->co2);
      Serial.printf("%s%.0f]\n", STR_DEBUG_PRINT_INST_GAS, atual->gas);
 
      //---------------------- Json (médias, mins, max, )
      Serial.println(STR_DEBUG_PRINT_JSON);
      SerializationUtils::printLeituraJson(ts, tempos);
    }else{
      Serial.println(STR_DEBUG_1A_LEITURA_NAO_COLHIDA);
    }
  }
  
};