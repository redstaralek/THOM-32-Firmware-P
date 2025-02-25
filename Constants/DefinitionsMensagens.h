//--------------------------------------------
//------------ MENSAGENS  RESETS -------------
//--------------------------------------------

// Resets (firmware, vars, deepsleep, ...)
#define MSG_RESET_FALHA_CONEXAO "A Estação falhou sucessivas vezes em se conectar à rede"
#define MSG_RESET_VAR_500       "Status 500 ao salvar leitura"
#define MSG_RESET_BAD_CONFIG    "Estação configurada incorretamente"
#define MSG_RESET_BIRUTA_INVAL  "Modelo de biruta inválido! Esperava \"1\" ou \"2\"."
#define MSG_RESET_BATERIA_BAIXA "Bateria muito baixa"


//--------------------------------------------
//----------- MENSAGENS CONSOLE --------------
//--------------------------------------------

// INPUTS
#define STR_INPUT_S_N                       "[[INPUT]]>> INPUT: Gostaria de configurar a rede? [s/n]\t(tempo p/ resposta: 10s)\n"
#define STR_INPUT_JSON                      "[[INPUT]]>> INPUT: Insira o json de configuração......:\t(tempo p/ resposta: 60s)\n"

// ERRORS
#define STR_DADOS_ERROR_VAZIOS              "[[ERROR]]>> ERROR: Algum dos dados encontrados está vazio. Informe-os novamente..."
#define STR_ERROR_REINIC                    "[[ERROR]]>> ERROR: As tentativas de configuração falharam! Reiniciando estação...."
#define STR_ERROR_CONECTA_WIFI              "[[ERROR]]>> ERROR: As tentativas de conexão WiFi falharam! Reiniciando estação...."
#define STR_ERROR_ENVIA_DADOS               "[[ERROR]]>> ERROR: As tentativas de enviar dados falharam! Reiniciando estação...."
#define STR_ERROR_VALIDA_WIFI               "[[ERROR]]>> ERROR: Os dados wiFi falharam sucessivamente.  Reiniciando estação...."
#define STR_ERROR_GET_DADOS                 "[[ERROR]]>> ERROR: [BME-680] não retornou dados válidos....."
#define STR_ERROR_SENSORES                  "[[ERROR]]>> ERROR: [BME-680] falhou. Reiniciando estação...."
#define STR_INFO_STATUS_FALHA               "[[ERROR]]>> ERROR: STATUS DA RESP........:"
#define STR_ERROR_FALHA_CONECT              "[[ERROR]] Falha ao conectar "
#define STR_ERROR_NTC                       "[[INFO]]>> INFO: Conectando ao servidor NTP!....................................."
#define STR_ERROR_OPCAO_INVALIDA            "[[INFO]]>> INFO: Opção inválida, esperava-se \"s\"/\"n\". Prosseguindo com padrão (\"n\")."

// WARNINGS
#define STR_WARNING_TMP                     "[[WARNING]]>> WARNING: Dado de temperatura inválido..................................."
#define STR_WARNING_HUM                     "[[WARNING]]>> WARNING: Dado de humidade inválido......................................"
#define STR_WARNING_PRS                     "[[WARNING]]>> WARNING: Dado de pressão inválido......................................."
#define STR_WARNING_SALVA_L                 "[[WARNING]]>> WARNING: Salvando leituras na memória (wiFi indisponível)..............."

// INFOS
#define STR_INFO_RECUP_MEM                  "[[INFO]]>> INFO: Nenhum dado informado, recuperando última rede salva na memória.\n"
#define STR_INFO_CONFIG                     "[[INFO]]>> INFO: A SEGUINTE REDE FOI CONFIGURADA COM SUCESSO:...................."
#define STR_INFO_ENVIA_L                    "[[INFO]]>> INFO: [Leitura atual + memória] enviadas para API (wiFi disponível)..."
#define STR_INFO_CMD_RESTART                "[[INFO]]>> INFO: A estação recebeu o comando serial para restart. Reiniciando...."
#define STR_INFO_SSID                       "[[INFO]]NOME/SSID.......................:  "
#define STR_INFO_SSID2                      "[[INFO]]NOME/SSID (CONTINGÊNCIA)........:  "
#define STR_INFO_TOKEN                      "[[INFO]]TOKEN DA ESTAÇÃO................:  "
#define STR_INFO_BIRUTA                     "[[INFO]]MODELO BIRUTA...................:  "
#define STR_INFO_STATUS                     "[[INFO]]>> INFO: STATUS DA RESP........:"
#define STR_INFO_RESP                       "[[INFO]]>> INFO: RESP DA API...........:"
#define STR_INFO_CONECTADO_WIFI             "[[INFO]] Conexão estabelecida "
#define STR_INFO_NOVA_TENTATIVA_WIFI        "[[INFO]]>> INFO: Nova tentativa de conexão Wi-Fi "
#define STR_INFO_NOVA_TENTATIVA_WIFI_CONTINGENCIA        "[[INFO]]>> INFO: Nova tentativa de conexão Wi-Fi (rede de contingência) "
#define STR_INFO_ESPERANDO_WIFI             "[[INFO]]>> INFO: Esperando conexão à rede WiFi... RSSI: "
#define STR_INFO_BUSCANDO_WIFI_MEMORIA      "[[INFO]]>> INFO: Reconfiguração da rede WiFi não foi solicitada. Dados serão buscados na memória."
#define STR_INFO_TEMPO_COMANDO              "[[INFO]]>> INFO: espera ocupada, envie os comandos desejados neste intervalo. Tempo (seg): "

// DEBUGS
#define STR_DEBUG_LEITURAS                  "[[DEBUG]]>> DEBUG: DADOS DAS LEITURAS:............................................."
#define STR_DEBUG_RESET_MIN                 "[[DEBUG]]>> Resetando mínimos"
#define STR_DEBUG_RESET_MAX                 "[[DEBUG]]>> Resetando máximos"
#define STR_DEBUG_RESET_MED                 "[[DEBUG]]>> Resetando médias da série temporal"
#define STR_DEBUG_RESET_ANEM                "[[DEBUG]]>> Resetando anemometria"
#define STR_DEBUG_RESET_PLUV                "[[DEBUG]]>> Resetando pluviometria"
#define STR_DEBUG_RESET_MSG                 "[[DEBUG]]>> Resetando mensagem"
#define STR_DEBUG_RESET_AUX                 "[[DEBUG]]>> Resetando variáveis auxiliares"
#define STR_DEBUG_RESET_NORMAL              "[[DEBUG]]>> Resetando variáveis em Setup(). Reset normal."
#define STR_DEBUG_CONTADORES_ESVAZIADOS     "[[DEBUG]]>> Contadores esvaziados: "
#define STR_DEBUG_RESET_ESPECIAL            "[[DEBUG]]>> Reset por motivo especial. Contadores mantidos:"
#define STR_DEBUG_MOTIVO                    "[[DEBUG]]>> Motivo: "
#define STR_DEBUG_DESATIVA_BARR_E_ADC       "[[DEBUG]]>> Iteração + desativa ADC e barramento de dados"
#define STR_DEBUG_FIM_COLETA                "[[DEBUG]]>> Fim coleta de dados..."
#define STR_DEBUG_ENTRA_LIGHT_SL            "[[DEBUG]]>> DEBUG: Entrando em light-sleep:"
#define STR_DEBUG_CALC_DADOS_DIGITAIS       "[[DEBUG]]>> Colhe e processa dados digitais (tmp, hum, prs e suas redundâncias, caso haja)"
#define STR_DEBUG_CALC_LUX                  "[[DEBUG]]>> Cálc. Luximétrico [LDR]"
#define STR_DEBUG_CALC_UV                   "[[DEBUG]]>> Cálc. Irradiométrico (UVA) [AS7331]"
#define STR_DEBUG_CALC_BAT                  "[[DEBUG]]>> Cálc. Bateria"
#define STR_DEBUG_CALC_DIR                  "[[DEBUG]]>> Cálculo direção eólica"
#define STR_DEBUG_CALC_VEL                  "[[DEBUG]]>> Cálculo velocidade eólica"
#define STR_DEBUG_CALC_MAXS                 "[[DEBUG]]>> Máximos   [Tmp, hum, prs, uva, lx]"
#define STR_DEBUG_CALC_MINS                 "[[DEBUG]]>> Mínimos   [Tmp, hum, prs, uva, lx]"
#define STR_DEBUG_1A_LEITURA_NAO_COLHIDA    "[[DEBUG]]>> Primeira leitura ainda não colhida. Aguarde e solicite novamente"

// DEBUGS DE PRINT VARS
#define STR_DEBUG_PRINT_INSTANT             "[[DEBUG]]>> Leituras atuais [Tmp, Hum, Prs, UV, Ilum, Cpu temp, Bat volt]."
#define STR_DEBUG_PRINT_JSON                "[[DEBUG]]>> Json de dados gerais [médias, hardware, minmax, etc]"
#define STR_DEBUG_PRINT_INST_TMP            "[[DEBUG]] Inst->Tmp......:"
#define STR_DEBUG_PRINT_INST_HUM            "[[DEBUG]] Inst->Hum......:"
#define STR_DEBUG_PRINT_INST_PRS            "[[DEBUG]] Inst->Prs......:"
#define STR_DEBUG_PRINT_INST_UV             "[[DEBUG]] Inst->UVA......:"
#define STR_DEBUG_PRINT_INST_LX             "[[DEBUG]] Inst->lux......:"
#define STR_DEBUG_PRINT_INST_BAT            "[[DEBUG]] Inst->Bat......:"
#define STR_DEBUG_PRINT_INST_CO2            "[[DEBUG]] Inst->CO2......:"
#define STR_DEBUG_PRINT_INST_GAS            "[[DEBUG]] Inst->GasRes...:"

// PROBE POINT
#define STR_DEBUG_PROBE_POINT               "[[USB_CONECTADO?]] ~> tempo p/ resp (seg): "

