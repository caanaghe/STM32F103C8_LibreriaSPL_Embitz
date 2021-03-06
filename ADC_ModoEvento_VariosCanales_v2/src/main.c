
/*
   programa ejemplo que convierte dos muestras an PA0 y PA1, o sea,
   canales 0 y 1 del ADC1.
   Se usa el modo de rastreo o escaneo de dos canales con DMA en forma de una
   muestra en vez de forma por *eventos* continuamente.

*/

#include "stm32f10x_conf.h"

void reloj_56M(void);
void UART1_Init(void);
void ADC1_Evento_Varios_canal(uint8_t* Canales, uint8_t num_canales);
void UART_numeroADC(uint32_t adc_value);
void DMA_ADC_Init(uint32_t *destination, uint16_t num_canales);
void enviarpalabra(char *arreglo, uint16_t longitud);
void adc1_unamuestra_varioscanales(uint8_t* canales, uint8_t cantidad);


int main(void)
{
    reloj_56M();
    UART1_Init();

    uint8_t Canales[]= {ADC_Channel_0, ADC_Channel_1, ADC_Channel_8, ADC_Channel_9};
    uint16_t num_canales = 4;
    ADC1_Evento_Varios_canal(Canales, num_canales);

    uint16_t destination[10];// arreglo donde van a quedar los datos de los canales convertidos
    DMA_ADC_Init(&destination, num_canales);

    ADC_SoftwareStartConvCmd(ADC1 , ENABLE);// inicia conversi�n

  while(1)
  {
    enviarpalabra("\n", 2);
    enviarpalabra("A0 = ", 5);
    UART_numeroADC(destination[0]);
    enviarpalabra("A1 = ", 5);
    UART_numeroADC(destination[1]);
    enviarpalabra("A3 = ", 5);
    UART_numeroADC(destination[2]);
    enviarpalabra("A4 = ", 5);
    UART_numeroADC(destination[3]);
    for (int i = 0; i < 2000000; ++i) asm("nop");// retardo
    ADC_SoftwareStartConvCmd(ADC1 , ENABLE);// inicia conversi�n
  }
}


/***********************************************
 *  CONFIGURA EL SYSCLOCK EN 56Mh CON CONEXION EN LOS PERIFERICOS: HCLK, PCLK1, PCLK2.
 ***********************************************/
void reloj_56M(void)
{
    // CONFIGURAR EL RELOJ
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no est� en uso
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
* Inicializaci�n del puerto Serial1
***********************************************/
void UART1_Init(void)
{
    // *Habilitar el reloj del puerto serial, del puerto Gpio donde est� ubicado y de la funci�n alterna.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    // *Configuraci�n del modo de funci�n alterna de cada pin del puerto serial.
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
    //*Habilitaci�n del puerto serial. TX Y RX
    USART_Init(USART1, &UART_Struct);
    USART_Cmd(USART1, ENABLE);
}



/***********************************************
* Inicializaci�n del ADC1 en modo evento para varios canales. EN EL PUERTO A DESDE EL CANAL 0 AL 9.
***********************************************/
void ADC1_Evento_Varios_canal(uint8_t* canales, uint8_t cantidad)
{
    uint8_t ii;

    RCC_ADCCLKConfig(RCC_PCLK2_Div4);//  frecuencia max para el ADC es 14MHz, reloj para ADC (max 14MHz --> 56Mhz/4=14MHz)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); // Habilitacion los puertos (GPIOA Y GPIOB) y del ADC.

    // configuraci�n de los pines del ADC (PA0 -> canal 0 a PA7 -> canal 7 y del PB0 -> canal 8 y PB1 -> canal 9) como entradas analo�gicas.
    GPIO_InitTypeDef GPIO_InitStruct; // estrutura para configurar los pines
    GPIO_StructInit(&GPIO_InitStruct); // inicializaci�n de la estructura
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;

    //  Inicializa todos los pines del puerto A  que fueron ingresados en Canales.
    for(ii= 0; ii< cantidad; ii++)
    {
        if(canales[ii]<8)
        {
            GPIO_InitStruct.GPIO_Pin = 1<<canales[ii];// el canal se convierte en el # del pin
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
    }

    //  Inicializa todos los pines del puerto B que fueron ingresados en Canales.
    for(ii= 0; ii< cantidad; ii++)
    {
        if((canales[ii]>7) && (canales[ii] < 10))
        {
            GPIO_InitStruct.GPIO_Pin = 1<<(canales[ii]-8);// el canal se convierte en el # del pin
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            GPIO_Init(GPIOB, &GPIO_InitStruct);
        }
    }

    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;// configuraci�n del ADC1
    ADC_InitStruct.ADC_ScanConvMode = ENABLE; // multiples canales
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;// modo de conversi�n cont�nuo
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // sin inicio de conversi�n externo
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; // alineamiento de presentaci�n de datos hacia la derecha
    ADC_InitStruct.ADC_NbrOfChannel = cantidad; // 8 canales de conversi�n
    ADC_Init(ADC1, &ADC_InitStruct); // carga informaci�n de configuraci�n

    // configuraci�n de cada canal
    for(ii= 0; ii< cantidad; ii++)
    {
        ADC_RegularChannelConfig(ADC1, canales[ii], ii+1, ADC_SampleTime_239Cycles5);
    }

    // habilitaci�n de ADC1
    ADC_Cmd(ADC1, ENABLE);

    // calibraci�n
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}



/***********************************************
 *  FUNCION PARA ENVIAR EL NUMERO ADC (ENTRE 0 Y 4095) DE CUALQUIER TAMA�O POR UART CONVERTIDO A ASCII.
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



/***********************************************
 * Configuracion del DMA PARA ADC.
 ***********************************************/
 void DMA_ADC_Init(uint32_t *destination, uint16_t num_canales)
 {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // activaci�n del reloj del DMA
    DMA_DeInit(DMA1_Channel1); // DESINICIALIZA EL CANAL QUE VAMOS A CONFIGURAR.
    // CONFIGURACION DEL DMA POR DAFAULT PRIMERO
    DMA_InitTypeDef DMA_InitStruct; // CREA LA ESTRUCTURA PARA DMA
    DMA_StructInit(&DMA_InitStruct); // INICIALIZA POR DEFAULT, EL & DICE TOME EL VALOR Y AHI TOMA LO Q TENGA A LA DERECHA.
    // CONFIGURACION DEL DMA REQUERIDA
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable; // este canal se va a usar para transferencia desde perif�rico a memoria
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular; //selecci�n de modo circular
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium; //prioridad media
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //tama�o del dato de la fuente y el destino= 16bit
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable; //habilitaci�n de incremento autom�tico en destino
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC; //La posici�n asignada al registro perif�rico ser� la fuente
    DMA_InitStruct.DMA_BufferSize = num_canales; //tama�o de los datos que se transfieren
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR; //direcci�n de inicio de la fuente y el destino
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)destination;
    // INICIALIZA EL DMA Y EL USAR.
    DMA_Init(DMA1_Channel1, &DMA_InitStruct); //programa registros del DMA

    DMA_Cmd(DMA1_Channel1, ENABLE); //habilita transferencia en el canal de DMA1
    ADC_DMACmd(ADC1, ENABLE); // habilitaci�n de DMA para ADC
 }



//***************************************************************************************************************************************
/***********************************************
* Funcion para enviar palabra al uart.
***********************************************/
void enviarpalabra(char *arreglo, uint16_t longitud)
{
    for(uint16_t i= 0; i< longitud; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, arreglo[i]);
    }
}
//***************************************************************************************************************************************
