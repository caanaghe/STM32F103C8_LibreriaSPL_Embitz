
/**********************************************************************
   Programa ejemplo para convertir la señal conectada a PA0 a digital
   Envía el resultado de la conversión por Uart

   Codigo tipo ADC, realiza la conversion analoga a digital en
   un solo canal (1 gpio a la vez) de modo continuo y visualiza un
   numero entre 0 y 4095 dependiendo del valor de la conversion.
**********************************************************************/

#include "stm32f10x_conf.h"

void reloj_56M(void);
void UART1_Init(void);
void adc1_continuo_un_canal(uint8_t ADC_Channel_X);
void UART_numeroADC(uint32_t adc_value);

int main(void)
{
    reloj_56M();
    UART1_Init();
    adc1_continuo_un_canal(ADC_Channel_0);

    uint16_t adc_value;// variable para leer la conversión.

  while(1)
  {
      adc_value = ADC_GetConversionValue(ADC1);
      UART_numeroADC(adc_value);// envía el valor de la convesión a uart
      for (int i = 0; i < 2000000; ++i) asm("nop");// retardo
  }
}


/***********************************************
 *  CONFIGURA EL SYSCLOCK EN 56Mh CON CONEXION EN LOS PERIFERICOS: HCLK, PCLK1, PCLK2.
 ***********************************************/
void reloj_56M(void)
{
    // CONFIGURAR EL RELOJ
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
    RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
    RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_14); //8Mhz/2*14= 56Mhz
    RCC_PLLCmd(ENABLE); // habilita el PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)== RESET)
    {
    }
    // ESTAS SON LAS CONFIGURACIONES DE CADA PREESCALER DE LOS PUERTOS
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // PREESCALER DEL INICIO.
    RCC_HCLKConfig( RCC_SYSCLK_Div1); // HCLK = SYSCLK
    RCC_PCLK2Config( RCC_HCLK_Div1);  // PCLK2 = HCLK
    RCC_PCLK1Config( RCC_HCLK_Div2);  // PCLK1 = HCLK/2
}



/***********************************************
* Inicialización del puerto Serial1
***********************************************/
void UART1_Init(void)
{
    // *Habilitar el reloj del puerto serial, del puerto Gpio donde está ubicado y de la función alterna.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    // *Configuración del modo de función alterna de cada pin del puerto serial.
    GPIO_InitTypeDef GPIO_Struct;
    // TX: GPIOA PIN9 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_9;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Struct);
    // RX: GPIOA PIN10 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_10;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_Struct);

    // Inicializa parametros de Uart 9600 bps, 8 bits, 1 stop, no paridad
    USART_InitTypeDef UART_Struct;
    USART_StructInit(&UART_Struct);
    UART_Struct.USART_BaudRate= 9600;
    //*Habilitación del puerto serial. TX Y RX
    USART_Init(USART1, &UART_Struct);
    USART_Cmd(USART1, ENABLE);
}



/***********************************************
* Inicialización del ADC1 en modo continuo para un solo canal. EN EL PUERTO A DESDE EL CANAL 0 AL 9.
***********************************************/
void adc1_continuo_un_canal(uint8_t ADC_Channel_X)
{
    // configuracion del PUERTO A (GPIOA)
    GPIO_InitTypeDef GPIOA_Struct;
    GPIOA_Struct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIOA_Struct.GPIO_Mode = GPIO_Mode_AIN;
    // DEPENDIENDO DEL CANAL ENTRADO COMO PARAMETEO, SE CONFIGURA EL GPIOX.
    switch(ADC_Channel_X)
    {
    case ADC_Channel_0:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_0;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_1:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_1;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_2:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_2;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_3:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_3;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_4:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_4;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_5:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_5;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_6:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_6;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_7:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_7;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    case ADC_Channel_8:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_0;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            GPIO_Init(GPIOB, &GPIOA_Struct);
            break;
    case ADC_Channel_9:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_1;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            GPIO_Init(GPIOB, &GPIOA_Struct);
            break;
    default:
            GPIOA_Struct.GPIO_Pin = GPIO_Pin_0;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIOA_Struct);
            break;
    }
    // configuración del ADC
    RCC_ADCCLKConfig (RCC_PCLK2_Div4);// reloj para ADC (max 14MHz --> 56Mhz/4=14MHz)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);// habilita el reloj del ADC1
    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE; // convierte continuamente
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;// inicio de conversión por software
    ADC_Init(ADC1, &ADC_InitStruct); //ADC1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_X, 1, ADC_SampleTime_13Cycles5); // ADC1, canal X: Entrada al pin PAX
    ADC_Cmd(ADC1, ENABLE);// habilita ADC1

    // calibracion del ADC
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_Cmd(ADC1, ENABLE);// habilita ADC1
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);// inicia la conversión en modo continuo por software
}



/***********************************************
 *  FUNCION PARA ENVIAR EL NUMERO ADC (ENTRE 0 Y 4095) DE CUALQUIER TAMAÑO POR UART CONVERTIDO A ASCII.
 ***********************************************/
void UART_numeroADC(uint32_t adc_value)
{
    uint16_t arreglo[]={'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'}; // arreglo donde se va a guardar el dato subdividido
    uint32_t a= adc_value;
    uint32_t b, c;
    uint8_t apuntador= 0;

    /// Subdividir el numero (adc_value) por cada entero.
    if(a== 0)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET);
        USART_SendData(USART1, '0');
    }
    while(a>0)
    {
        b= a/10;
        c= (uint32_t) (a- b*10);
        arreglo[apuntador]= (uint16_t) c + 48;
        apuntador++;
        a= b;
    }

    /// Mostrar en pantalla el vector.
    for(uint8_t i=0; i<apuntador; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET);
        USART_SendData(USART1, arreglo[apuntador-i-1]);
    }
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET);
    USART_SendData(USART1, '\n');
}



