
// PRENDE Y APAGA EL LED MEDIANTE UART, USANDO EL PROGRAMA HERCULES.

#include "stm32f10x_conf.h"

void LED_Init(void);
void UART1_Init(void);

int main(void)
{
    LED_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL PUERTO C CON EL LED
    UART1_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL PUERTO SERIA 1 (PA9 Y PA10)

    char dato;

  while(1)
{
        if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) // REVISA EL ESTADO DE LA BANDERA
        {
            dato= (char) USART_ReceiveData(USART1);

            // si recibe 'A', apaga el LED y transmite 'a'
            if(dato == 'A')
            {
                while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
                USART_SendData(USART1, 'a'); // ENVIA EL DATO CON MAX 8 BITS
                GPIO_SetBits(GPIOC, GPIO_Pin_13);
            }
            // si recibe 'B', enciende el LED y transmite 'b'
            if(dato == 'B')
            {
                while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
                USART_SendData(USART1, 'b'); // ENVIA EL DATO CON MAX 8 BITS
                GPIO_ResetBits(GPIOC,  GPIO_Pin_13);
            }
        }
    }
}


/***********************************************
 * Inicialización del puerto C, pin 13
 ***********************************************/
void LED_Init(void)
{// ESTO ES HECHO PARA EL GPIO DEL LED PERO SE PUEDE HACER EL MISMO PROCEDIMIENTO PARA CUALQUIER GPIO.
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
