
// ejemplo de encender y apagar el led de la tarjeta
// cada segundo usando RTC con interrupción por segundo y por alarma.
// Inicialmente interrumpe por segundos hasta los 5 segundos. Luego deshabilita
// la interrupción por segundos y habilita la de la alarma a los 10 segundos.
// en adelante, deshabilita la interrupción por alarma y sigue interrumpiendo por segundos.
// la información de interrupción y del contador rtc se muestra en el puerto serial,


/*
Codigo tipo Blink mediante una interrupcion manejada por software, es decir a partir de una
activacion cada determinado tiempo, este tiempo se puede manejar de dos maneras que es por cada
segundo se activa una bandera o programando la activacion de la bandera con una alarma cada determinado tiempo.
Tambien incluye manejo de Uart para visualizar la informacion. Todo en lenguaje C.
*/

#include "stm32f10x_conf.h"

void LED_Init(void);

void RTC_Init(void);

void UART1_Init(void);

void INTERRUP_RTC_Init(void);

void RTC_IRQHandler(void);

void enviarpalabra(char *arreglo, uint16_t longitud);

void UART_SEND_numero(uint32_t numero);

uint8_t bandera;


int main(void)
{
    LED_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL PUERTO C CON EL LED

    UART1_Init(); // Funcion para inicializar el uart1 que visualiza la informacion en el pc.

    RTC_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL RTC
    INTERRUP_RTC_Init();
    bandera= 0;

    enviarpalabra("Ejemplo RTC con interrupción \n", 30);


  while(1)
  {
    if(bandera== 1)
    {
        if (GPIO_ReadOutputDataBit (GPIOC, GPIO_Pin_13) == 0)// SI ESTA PRENDIDO EL LED, ENTRA Y LO APAGA Y SI ESTA APAGADO, ENTRA Y LO PRENDE.
        {
            GPIO_SetBits (GPIOC, GPIO_Pin_13);
            enviarpalabra("apaga led \n", 11);
        }
        else
        {
            GPIO_ResetBits (GPIOC, GPIO_Pin_13);
            enviarpalabra("enciende led \n", 14);
        }
        enviarpalabra("contador= ", 10);
        UART_SEND_numero(RTC_GetCounter());
        enviarpalabra("\n",1);
        bandera= 0;
    }

    asm("nop");// necesario para no dejar solo el if dentro del loop.

  }
}



/***********************************************
 * Inicialización del puerto C, pin 13
 ***********************************************/
void LED_Init(void)
{
    GPIO_InitTypeDef GPIOC_Struct;
    GPIOC_Struct.GPIO_Pin  = GPIO_Pin_13;
    GPIOC_Struct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIOC_Struct.GPIO_Mode = GPIO_Mode_Out_PP;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_Init(GPIOC, &GPIOC_Struct);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}



/***********************************************
 * Configuracion del RTC
 ***********************************************/
void RTC_Init(void) // CONFIGURACION DEL RTC
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);// habilitacion del reloj
    PWR_BackupAccessCmd(ENABLE);// habilita acceso al rtc

    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);

    RCC_LSEConfig(RCC_LSE_ON);// habilita el reloj externo 32.768 HZ
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);// espera hasta que el reloj se estabilice
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);// selecciona el reloj externo como fuente
    RTC_SetPrescaler(32767); // ACTIVA EL PRESCALER

    RCC_RTCCLKCmd(ENABLE);// habilita el rtc

    RTC_WaitForSynchro();// espera por sincronizacion
    RTC_WaitForLastTask();

    // interrupción por segundos y alarma
    RTC_ITConfig(RTC_IT_ALR | RTC_IT_SEC, ENABLE);
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_ALR | RTC_IT_OW);

    RTC_SetCounter(0); // PONER EL CONTADOR EN CERO.
}



/***********************************************
 * Configuracion de la interrupcion en RTC
 ***********************************************/
void INTERRUP_RTC_Init(void) // CONFIGURACION DE LA INTERRUPCION POR RTC
{
    NVIC_InitTypeDef NVIC_Struct;
    NVIC_Struct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Struct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_Struct);
}



/***********************************************
 * Funcion de la interrupcion en RTC
 ***********************************************/
void RTC_IRQHandler(void)
{
    uint32_t contador= RTC_GetCounter();

    if(RTC_GetITStatus(RTC_IT_SEC)== SET) // RTC POR SEGUNDOS
    {
        enviarpalabra("SEC \n", 5);
        RTC_ClearITPendingBit(RTC_IT_SEC);
        if(contador== 5)
        {
            RTC_ITConfig(RTC_IT_SEC, DISABLE);
            RTC_ITConfig(RTC_IT_ALR, ENABLE);
            RTC_SetAlarm(10);
        }
    }

    if(RTC_GetITStatus(RTC_IT_ALR)== SET) // RTC POR ALARMA
    {
        RTC_ITConfig(RTC_IT_SEC, ENABLE); // Habilita la bandera de segundos
        enviarpalabra("ALR \n", 5);
        RTC_ClearITPendingBit(RTC_IT_ALR);
    }

    if(RTC_GetITStatus(RTC_IT_OW)== SET)
    {
        enviarpalabra("OW  \n", 5);
        RTC_ClearITPendingBit(RTC_IT_OW);
    }



    bandera= 1;
}



/***********************************************
* Inicialización del puerto Serial1
***********************************************/
void UART1_Init(void)
{
    //* Configuración de los parámetros del puerto serial. ESTA CAMBIA RESPECTO AL USO, VER EL DOCUMENTO.
    // Inicializa la configuracion de los parámetros de Uart 9600 bps, 8 bits, 1 stop, no paridad
    GPIO_InitTypeDef GPIO_Struct;
    USART_InitTypeDef UART_Struct;
    UART_Struct.USART_BaudRate= 9600;
    USART_StructInit(&UART_Struct);

    // *Habilitar el reloj del puerto serial, del puerto Gpio donde está ubicado y de la función alterna.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    // *Configuración del modo de función alterna de cada pin del puerto serial.
    // TX: GPIOA PIN9 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_9;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Struct);
    // RX: GPIOA PIN9 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_10;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_Struct);

    //*Habilitación del puerto serial. TX Y RX
    USART_Cmd(USART1, ENABLE);
    USART_Init(USART1, &UART_Struct);
}



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



/***********************************************
 * envia un numero por el puerto serial
 ***********************************************/
void UART_SEND_numero(uint32_t numero)
{
    unsigned char tosend[]= {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
    uint32_t valor1;
    uint8_t valor2;
    uint8_t contador= 0;

    if(numero== 0)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, numero+0x30);
    }

    while(numero>0)
    {
        valor1= (uint32_t) (numero/ 10);
        valor2= (uint8_t) (numero- valor1*10);
        tosend[contador]= valor2+ 0x30;
        numero= valor1;
        contador++;
    }

    for(uint8_t i= 0; i<contador; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, tosend[contador- i- 1]);
    }
    return;
}


