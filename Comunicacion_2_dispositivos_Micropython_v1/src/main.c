/*
    Realiza la comunicacion entre el stm32 y un modulo wifi con esp8266 mediante UART,
    la comunicacion se realiza para mandar comandos de micropython desde el stm32 al modulo wifi,
    programando asi el modulo wifi desde el stm32, el resultado se envia y visualiza por otro UART.
*/

#include "stm32f10x_conf.h"

void reloj_56M(void);
void GPIO_A1_Init(void);
void UART1_Init(void);
void UART2_Init(void);
void enviarpalabra(char *arreglo, uint16_t longitud);

int main(void)
{
    reloj_56M();
    UART1_Init();
    UART2_Init();
    uint32_t ii;
    char caracter_wifi, caracter_hercules;

    // configuración del pin A1 como salida para conectarlo a RST del wifi
    GPIO_A1_Init();


    // poner el pin de RST del wifi en cero
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    for(ii= 0; ii< 200000; ii++)
    {
        asm("nop");
    }
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
    enviarpalabra("Iniciando...\n", 13);


  while(1)
  {
      // si hay caracteres desde hercules, se envían al wifi
      if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)== SET)
      {
          caracter_hercules= (char) USART_ReceiveData(USART1);
          while(USART_GetFlagStatus(USART2, USART_FLAG_TXE)== RESET);
          USART_SendData(USART2, caracter_hercules);
      }
      // si hay carecteres desde wifi, se envían a hercules
      if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE)== SET)
      {
          caracter_wifi= (char) USART_ReceiveData(USART2);
          while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET);
          USART_SendData(USART1, caracter_wifi);
      }
  }
}


/***********************************************
 * CONFIGURA EL GPIO CONECTADO. Inicialización del puerto A, pin 1
 ***********************************************/
void GPIO_A1_Init(void) // ESTO ES HECHO PARA EL GPIO DEL LED PERO SE PUEDE HACER EL MISMO PROCEDIMIENTO PARA CUALQUIER GPIO.
{
    GPIO_InitTypeDef GPIOA_Struct; // CREO LA ESTRUCTURA DEINICIALIZACION, TENIENDO EN CUENTA QUE ES EL PUERTO C
    GPIOA_Struct.GPIO_Pin  = GPIO_Pin_1; // SELECCIONO EL PIN A USAR DEL PUERTO C
    GPIOA_Struct.GPIO_Speed = GPIO_Speed_50MHz; // SELECCIONO LA VELOCIDAD DEL PUERTO. PUEDEN SER 2M, 10M Y 50M.
    GPIOA_Struct.GPIO_Mode = GPIO_Mode_Out_PP; // SELECCIONO EL MODO DEL PUERTO, EN ESTE CASO ES SALIDA PUSH PULL.

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // HABILIDO EL PUERTO C
    GPIO_Init(GPIOA, &GPIOA_Struct); // INICIALIZO LA ESTRUCTURA.
}


/***********************************************
* Inicialización del puerto Serial1
***********************************************/
void UART1_Init(void)
{
    //* Configuración de los parámetros del puerto serial. ESTA CAMBIA RESPECTO AL USO, VER EL DOCUMENTO.
    // Inicializa la configuracion de los parámetros de Uart 9600 bps, 8 bits DE DATO, 2 stop, no paridad, TX Y RX.
    USART_InitTypeDef UART_Struct;
    USART_StructInit(&UART_Struct);
    UART_Struct.USART_BaudRate= 115200;
    UART_Struct.USART_WordLength = USART_WordLength_8b;
    UART_Struct.USART_StopBits = USART_StopBits_2;
    UART_Struct.USART_Parity = USART_Parity_No ;
    UART_Struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    UART_Struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

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
    USART_StructInit(&UART_Struct);
    UART_Struct.USART_BaudRate= 115200;
    UART_Struct.USART_WordLength = USART_WordLength_8b;
    UART_Struct.USART_StopBits = USART_StopBits_2;
    UART_Struct.USART_Parity = USART_Parity_No ;
    UART_Struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    UART_Struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    // *Habilitar el reloj del puerto serial 2, del puerto Gpio donde está ubicado y de la función alterna.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    // *Configuración del modo de función alterna de cada pin del puerto serial.
    GPIO_InitTypeDef GPIO_Struct;
    // RX: GPIOA PIN2 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_2;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Struct);
    // TX: GPIOA PIN3 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_3;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_Struct);

    //*Habilitación del puerto serial. TX Y RX
    USART_Init(USART2, &UART_Struct);
    USART_Cmd(USART2, ENABLE);
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
    //RCC_PCLK1Config( RCC_HCLK_Div2);  // PCLK1 = HCLK/2
}



/***********************************************
* Funcion para enviar palabra al uart.
***********************************************/
void enviarpalabra(char *arreglo, uint16_t longitud)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, '\r');

    for(uint16_t i= 0; i< longitud; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, arreglo[i]);
    }
}


