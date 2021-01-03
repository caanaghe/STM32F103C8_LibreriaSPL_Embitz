
// COMUNICACION ENTRE LOS 3 UARTS, EN EL TALLER ESTA MAS EXPLICITO..

// SERIAL 1 (RX Y TX)
// SERIAL 2 (TX)
// SERIAL 3 (RX)

// ASCII https://datasagar.com/everything-about-ascii-in-c/

#include "stm32f10x_conf.h"

void LED_Init(void);

void UART1_Init(void);
void UART2_Init(void);
void UART3_Init(void);

void RTC_Init(void);

void INTERRUPCION_UART_Init(void);
void Ejecucion_Interrupcion(void);
void USART1_IRQHandler(void);

uint16_t direcc= 0;  // Esta variable tiene cero para indicar que el conteo es ascendente y uno para indicar que el conteo es descendente.


int main(void)
{
    LED_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL PUERTO C CON EL LED

    UART1_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA LOS PUERTOS SERIALES.
    UART2_Init();
    UART3_Init();

    INTERRUPCION_UART_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA LA INTERRUPCION POR UART.

    RTC_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL RTC
    RTC_SetAlarm(2);// SE PROGRAMA ALARMA EN 2 SEG.
    RTC_ClearFlag(RTC_FLAG_ALR); // LIMPIAMOS LA BANDERA.
    while(RTC_GetFlagStatus(RTC_FLAG_RTOFF)== RESET); // ESPERAMOS QUE LA BANDERA SE SINCRONICE.
    uint32_t reloj= RTC_GetCounter(); // SE LEE COMO VA EL RELOJ DEL PUERTO GPIO.

    uint16_t contador= 0;// variables que se incrementa o decrementa.

  while(1)
    {

        if((RTC_GetCounter()- reloj)>= 2) // ESPERAMOS POR 2 SEGUNDOS.
          {
              reloj= RTC_GetCounter(); // CUANDO ENTRA RECONFIGURAMOS LA VARIABLE RELOJ PARA ENTRAR SOLO CUANDO PASAN 2 SEG.

              while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET); // ESPERAMOS QUE EL CANAL ESTE DESOCUPADO PARA ENVIAR.
              USART_SendData(USART2, contador); // ENVIAMOS EL VALOR DE CONTADOR DESDE USART2

              while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET); // VERIFICAMOS SI LE LLEGA EL MENSAJE A USART3 MEDIANTE LA BANDERA
              uint16_t dato = (uint16_t) USART_ReceiveData(USART3); // LEEMOS EL DATO QUE LLEGO

              if( (direcc == 0) && (contador < 9) ) // LA LOGICA PEDIDA, QUE SOLO INCREMENTE  SI SE CUMPLE EL PARAMETRO
                    {
                        dato++;
                    }
              else if( (direcc == 1) && (contador > 0) ) // LA LOGICA PEDIDA, QUE SOLO DECREMENTE SI SE CUMPLE EL PARAMETRO
                    {
                        dato--;
                    }
              contador = dato;

              while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // ESPERAMOS QUE EL CANAL ESTE DESOCUPADO MEDIANTE LA BANDERA.
              USART_SendData(USART1, contador+48); // VAMOS ENVIANDO EL VALOR DEL CONTADOR AL PC
          }
    }
}



/***********************************************
 * Inicialización del puerto C, pin 13
 ***********************************************/
void LED_Init(void)// ESTO ES HECHO PARA EL GPIO DEL LED PERO SE PUEDE HACER EL MISMO PROCEDIMIENTO PARA CUALQUIER GPIO.
{
    GPIO_InitTypeDef GPIOC_Struct; // CREO LA ESTRUCTURA DEINICIALIZACION, TENIENDO EN CUENTA QUE ES EL PUERTO C
    GPIOC_Struct.GPIO_Pin  = GPIO_Pin_13; // SELECCIONO EL PIN A USAR DEL PUERTO C
    GPIOC_Struct.GPIO_Speed = GPIO_Speed_2MHz; // SELECCIONO LA VELOCIDAD DEL PUERTO. PUEDEN SER 2M, 10M Y 50M.
    GPIOC_Struct.GPIO_Mode = GPIO_Mode_Out_PP; // SELECCIONO EL MODO DEL PUERTO, EN ESTE CASO ES SALIDA PUSH PULL.

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // HABILIDO EL PUERTO C
    GPIO_Init(GPIOC, &GPIOC_Struct); // INICIALIZO LA ESTRUCTURA.
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);// PRENDO EL LED.
}



/***********************************************
* Inicialización del puerto Serial1
***********************************************/
void UART1_Init(void)
{
    //* Configuración de los parámetros del puerto serial. ESTA CAMBIA RESPECTO AL USO, VER EL DOCUMENTO.
    // Inicializa la configuracion de los parámetros de Uart 9600 bps, 8 bits DE DATO, 2 stop, no paridad, TX Y RX.
    USART_InitTypeDef UART_Struct;
    UART_Struct.USART_BaudRate= 9600;
    UART_Struct.USART_WordLength = USART_WordLength_8b;
    UART_Struct.USART_StopBits = USART_StopBits_2;
    UART_Struct.USART_Parity = USART_Parity_No ;
    UART_Struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    UART_Struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_StructInit(&UART_Struct);

    // *Habilitar el reloj del puerto serial1, del puerto Gpio donde está ubicado y de la función alterna.
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

    //*Habilitación del puerto serial. TX Y RX
    USART_Cmd(USART1, ENABLE);
    USART_Init(USART1, &UART_Struct);
}



/***********************************************
* Inicialización del puerto Serial2
***********************************************/
void UART2_Init(void) // CONFIGURADO COMO SOLO TX
{
    //* Configuración de los parámetros del puerto serial. ESTA CAMBIA RESPECTO AL USO, VER EL DOCUMENTO.
    // Inicializa la configuracion de los parámetros de Uart 9600 bps, 8 bits DE DATO, 2 stop, no paridad
    USART_InitTypeDef UART_Struct;
    UART_Struct.USART_BaudRate= 9600;
    UART_Struct.USART_WordLength = USART_WordLength_8b;
    UART_Struct.USART_StopBits = USART_StopBits_2;
    UART_Struct.USART_Parity = USART_Parity_No ;
    UART_Struct.USART_Mode = USART_Mode_Tx;
    UART_Struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_StructInit(&UART_Struct);

    // *Habilitar el reloj del puerto serial 2, del puerto Gpio donde está ubicado y de la función alterna.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    // *Configuración del modo de función alterna de cada pin del puerto serial.
    GPIO_InitTypeDef GPIO_Struct;
    // TX: GPIOA PIN2 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_2;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Struct);

    //*Habilitación del puerto serial. TX Y RX
    USART_Cmd(USART2, ENABLE);
    USART_Init(USART2, &UART_Struct);
}



/***********************************************
* Inicialización del puerto Serial3
***********************************************/
void UART3_Init(void) // CONFIGURADO SOLO COMO RX
{
    //* Configuración de los parámetros del puerto serial. ESTA CAMBIA RESPECTO AL USO, VER EL DOCUMENTO.
    // Inicializa la configuracion de los parámetros de Uart 9600 bps, 8 bits DE DATO, 1 stop, no paridad
    USART_InitTypeDef UART_Struct;
    UART_Struct.USART_BaudRate= 9600;
    UART_Struct.USART_WordLength = USART_WordLength_8b;
    UART_Struct.USART_StopBits = USART_StopBits_2;
    UART_Struct.USART_Parity = USART_Parity_No ;
    UART_Struct.USART_Mode = USART_Mode_Rx;
    UART_Struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_StructInit(&UART_Struct);

    // *Habilitar el reloj del puerto serial, del puerto Gpio donde está ubicado y de la función alterna.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

    // *Configuración del modo de función alterna de cada pin del puerto serial.
    GPIO_InitTypeDef GPIO_Struct;
    // RX: GPIOB PIN11 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_11;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_Struct);

    //*Habilitación del puerto serial. TX Y RX
    USART_Cmd(USART3, ENABLE);
    USART_Init(USART3, &UART_Struct);
}



/***********************************************
 * Configuracion de la interrupcion por el puerto seria 1 uart.
 ***********************************************/
void INTERRUPCION_UART_Init(void)
{
    // CONFIGURA EL PUERTO QUE USAREMMOS PARA SER INTERRUPCION, EN ESTE CASO EL UART1 (GPIOA4)
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    // configurar el canal
    NVIC_InitTypeDef NVIC_Struct;
    NVIC_Struct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Struct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
    // inicializar la interrupcion
    NVIC_Init(&NVIC_Struct);
}



/***********************************************
 * Funcion que corre dentro de la funcion de la inteerupcion.
 ***********************************************/
void Ejecucion_Interrupcion(void)
{
    char dato= (char) USART_ReceiveData(USART1);

    // si recibe '>', apaga el LED y cambia direccion en 0
    if(dato == '>')
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        direcc = 0;
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
    // si recibe '<', enciende el LED y cambia direccion en 1
    if(dato == '<')
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        direcc = 1;
        GPIO_ResetBits(GPIOC,  GPIO_Pin_13);
    }
}



/***********************************************
 * Funcion de la interrupcion
 ***********************************************/
void USART1_IRQHandler(void)
{
    Ejecucion_Interrupcion();
    USART_ClearITPendingBit(USART1, USART_IT_RXNE); // lo que esta arriba de el es lo que se ejecuta cuando entre la interrupcion.
}



/***********************************************
 * Configuracion del RTC
 ***********************************************/
void RTC_Init(void)
{
    // CONFIGURACION DEL RTC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);// habilitacion del reloj
    PWR_BackupAccessCmd(ENABLE);// habilita acceso al rtc
    RCC_LSEConfig(RCC_LSE_ON);// habilita el reloj externo 32.768 HZ
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);// espera hasta que el reloj se estabilice
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);// selecciona el reloj externo como fuente
    RCC_RTCCLKCmd(ENABLE);// habilita el rtc
    RTC_WaitForSynchro();// espera por sincronizacion
    while(RTC_GetFlagStatus(RTC_FLAG_RTOFF)== RESET);
    RTC_SetPrescaler(32767); // ACTIVA EL PRESCALER
    RTC_SetCounter(0); // PONER EL CONTADOR EN CERO.
}


