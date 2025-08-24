//--------------------------------------------
//---------------- ELÉTRICO ------------------
//--------------------------------------------
#define RES_ACOP_LDR            250
#define RES_BAT                 325
#define RES_VOUT_LDR_MIN        0.01f
#define LDR_RES_MIN             30.0f


//--------------------------------------------
//---------------- PINAGEM -------------------
//--------------------------------------------

// Pins (dig input)
#define PLV_PIN                 25
#define PLV_GPIO                GPIO_NUM_25
#define VEL_PIN                 39
#define VEL_GPIO                GPIO_NUM_39
#define WAKEUP_PIN_BITMAP       0x8002000000
//     ,--> gpio39
//     |                ,--> gpio25
//     |                |
//     V                V
//    [1]0000000 000000[1]0 00000000 00000000 00000000
//    ==> convertendo p/ hexadec = [[[[ 8002000000 ]]]]

// Pins (anlg input, ADC2)
#define BAT_PIN                 33
#define DIR_PIN                 32

// Pins (dig output)
#define ATV_PIN                  4
#define LED_PIN                  2


//--------------------------------------------
//-------------------- ADC -------------------
//--------------------------------------------

//  3.3 ---- 4095
//   v  ----  A      ==> A = 4095*v/3.3

#define _3V3                    3.3f
#define ADC_RES                 12
#define ADC_MAX                 ((1 << ADC_RES) - 1)

#define ML8511_ADC_MIN          (ADC_MAX * 1.000f / _3V3)    //https://cdn.sparkfun.com/datasheets/Sensors/LightImaging/ML8511_3-8-13.pdf
#define ML8511_ADC_MAX          (ADC_MAX * 2.000f / _3V3)    //https://cdn.sparkfun.com/datasheets/Sensors/LightImaging/ML8511_3-8-13.pdf

#define LDR_ADC_MIN             0
#define LDR_ADC_MAX             ADC_MAX

#define DIR_ADC_MIN             0
#define DIR_ADC_MAX             (ADC_MAX * 1.75f / _3V3)     // Limite teórico de 1.59, +0.15 de garantia. https://www.usinainfo.com.br/blog/indicador-de-direcao-do-vento-com-arduino-melhorando-sua-estacao-meteorologica/

#define BAT_ADC_MIN             (ADC_MAX * 3.00f / _3V3 )/2  // 3.0volts -> "A/2" por conta do resistor
#define BAT_ADC_MAX             (ADC_MAX * 4.30f / _3V3 )/2  // 4.3volts -> "A/2" por conta do resistor
#define AS3935_ADDR             0x03
