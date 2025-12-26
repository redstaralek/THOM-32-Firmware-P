//--------------------------------------------
//------------------ Gerais ------------------
//--------------------------------------------
#define RADIAN                          57.2958f  
#define EEPROM_SIZE                     2048
#define ENDERECO_CONFIG_1               0

#define JSON_CONFIG_SIZE                512
#define JSON_AGORA_BUFFER_SIZE          512
#define JSON_ADICIONAL_ENVELOPE_SIZE    128
#define LEITURAS_BUFFER_SIZE            30000
#define JSON_ENVELOPE_SIZE              (LEITURAS_BUFFER_SIZE + JSON_AGORA_BUFFER_SIZE + JSON_ADICIONAL_ENVELOPE_SIZE)
#define JSON_ENVIO_OBJ_MAX_SIZE         (1.5 * (JSON_ENVELOPE_SIZE + 1000))
#define LEITURAS_AVG_N                  200
#define LEITURAS_CHECK_N                4
#define VERSAO_FIRMWARE                 "1.2.0"
#define NTP_SERVERS                     {"pool.ntp.org", "south-america.pool.ntp.org", "time.google.com", "time.cloudflare.com", "time.windows.com", "ntp.ubuntu.com", "nts.ntp.se", "nts1.time.nl", "asia.pool.ntp.org"}
#define GMT_OFFSET_SEC                  0           //UTC
#define DAYLIGHT_OFFSET_SEC             0           //UTC
#define DATE_FORMAT                     "%Y-%m-%d %H:%M:%S"
#define CONTENT_TYPE                    "application/json"
#define HTTP_STS_OK                     200
#define HTTP_STS_ERRO_INT               500

//--------------------------------------------
//------------------ MinMax ------------------
//--------------------------------------------
#define TMP_MIN                         -40
#define TMP_MAX                         85

#define HUM_MIN                         5
#define HUM_MAX                         100

#define HAL_MIN                         -999
#define HAL_MAX                         999

#define PRS_MIN                         3000
#define PRS_MAX                         100000 

#define BAT_MAX                         4.5f
#define BAT_MIN                         2.5f

#define TMP_CPU_MIN                     -40
#define TMP_CPU_MAX                     250   

#define UV_MIN                          0.0f
#define UV_MAX                          11.5f  

#define UVA_MIN                         0.0f
#define UVA_MAX                         10.0f  

#define UVB_MIN                         0.0f
#define UVB_MAX                         1.0f  

#define UVC_MIN                         0.0f
#define UVC_MAX                         0.5f  

#define LX_MAX                          200.0f  
#define LX_MIN                          0.000f  

//--------------------------------------------
//-------------------- I²C -------------------
//--------------------------------------------
#define NOISE_INT                       0x01
#define DISTURBER_INT                   0x04
#define LIGHTNING_INT                   0x08

//--------------------------------------------
//-------------------- SIM -------------------
//--------------------------------------------
#define SIM800_RX_PIN                   16
#define SIM800_TX_PIN                   17
#define BAUD_RATE                       9600
