/********************************************************************************
  * @file    main.c
  * @author  A. Riedinger & G. Stang. 
  * @version 0.1
  * @date    24-10-21. 
  * @brief   Se genera una señal por el DAC con un muestreo determinado para
             ensayar el aliasing cuando no se cumple Nyquist.

  * SALIDAS:
  	  *	LCD  Conexion Estandar TPs

  * ENTRADAS:
  	  * TEC-MAT
********************************************************************************/

/*------------------------------------------------------------------------------
LIBRERIAS:
------------------------------------------------------------------------------*/
#include "functions.h"

/*------------------------------------------------------------------------------
DEFINICIONES LOCALES:
------------------------------------------------------------------------------*/
/*Pines del ADC:*/
#define ADC_Port GPIOC
#define ADC_Pin  GPIO_Pin_0

/*Pines del DAC:*/
#define DAC_Port GPIOA
#define DAC_Pin  GPIO_Pin_5

/*Parametros de configuración del TIM3:*/
#define TimeBase 200e3

/*Frecuencia de muestreo:*/
#define FS  5000 //[Hz]

/*Funcion para procesar los datos del ADC:*/
void ADC_PROCESSING(void);

/*------------------------------------------------------------------------------
VARIABLES GLOBALES:
------------------------------------------------------------------------------*/
/*Variable para organizar el Task Scheduler:*/
uint8_t adcReady = 0;

/*Variables para la visualizacion del aliasing:*/
float   aliasIn = 0.0f;
float   aliasOut = 0.0f;

/*Variables para la conversion DAC de la senal:*/
int32_t signalIn = 0;
int32_t signalOut = 0;

int main(void)
{
/*------------------------------------------------------------------------------
CONFIGURACION DEL MICRO:
------------------------------------------------------------------------------*/
	SystemInit();

    /*Inicializacion del ADC:*/
    INIT_ADC(ADC_Port, ADC_Pin);

    /*Inicializacion del DAC:*/
    INIT_DAC_CONT(DAC_Port, DAC_Pin);

	/*Inicialización del TIM3:*/
	INIT_TIM3();
	SET_TIM3(TimeBase, FS);

	INIT_DO(GPIOB, GPIO_Pin_14);
	INIT_DO(GPIOB, GPIO_Pin_7);

/*------------------------------------------------------------------------------
BUCLE PRINCIPAL:
------------------------------------------------------------------------------*/
    while(1)
    {
        /*Task Scheduler:*/
        if (adcReady == 1)
            ADC_PROCESSING();
    }
}
/*------------------------------------------------------------------------------
INTERRUPCIONES:
------------------------------------------------------------------------------*/
/*Interrupcion al vencimiento de cuenta de TIM3 cada 1/FS=714.28 useg:*/
void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        /*Set de la variable del TS:*/ 
        adcReady = 1; 
        GPIO_ResetBits(GPIOB, GPIO_Pin_7);

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

/*------------------------------------------------------------------------------
TAREAS:
------------------------------------------------------------------------------*/
/*Procesamiento de los datos del ADC:*/
void ADC_PROCESSING()
{
    /*Reset de la variable del TS:*/
    adcReady = 0;

    /*Lectura y conversion del dato analogico:*/
    signalIn = READ_ADC(ADC_Port, ADC_Pin) - 32;

    /*Normalizacion del aliasing:*/
    aliasIn  = ((float)signalIn)/64;
    aliasOut = aliasIn;

    /*Desnormalizacion:*/
    signalOut= (aliasOut*4096) + 2048;

    /*Generacion de la senal por DAC:*/
    DAC_CONT(DAC_Port, DAC_Pin, (uint16_t) signalOut);
//    DAC_CONT(DAC_Port, DAC_Pin, 2047);
}
