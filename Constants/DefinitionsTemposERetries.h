//--------------------------------------------
//------------- TEMPOS/RETRIES ---------------
//--------------------------------------------
#define ANTI_BOUNCING_PLUV      25
#define ANTI_BOUNCING_GIRO      25          //VEL_MAX (km/h) ==> 3.6*(2*pi*0.18)*(1000/ANTI_BOUNCING_GIRO)
#define ESPERA_CONEXAO          60*2
#define MILISSEC_DELAY_READS    10
#define NU_RETRY_CONEXAO        5
#define NU_RETRY_INIT_SIM       5
#define NU_RETRY_CREDENCIAIS    3
#define MILISSEG_POOLING        30*1000  
#define MILISSEC_ENVIO          9*60*1000
#define SERIAL_RATE             4800
#define WDT_TIMEOUT             60*10        //10 minutos
#define HTTP_TIMEOUT            120000
#define MILIS_VERIFICACAO_USB   2000
#define NANO_FACTOR             1000000000
#define SEGS_DEEP_SL_CONFIG     86400
#define MICROS_LIGHT_SL_QUANTUM 5000000
#define FREQ_CPU_MIN_SENS       160    
#define FREQ_CPU_MAX_WIFI       240    
#define NTP_RETRY_N             2