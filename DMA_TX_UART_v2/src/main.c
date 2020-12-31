
// ejemplo de DMA aplicado al usart1 en tx, canal 4
// Almacena los datos en memoria y lleva el contenido de memoria de 10 datos para ser transferidos por uart a una pc.
// comunicacion directa entre la memoria y el periferico UART1.

#include "stm32f10x_conf.h"

void reloj_36M(void);
void LED_Init(void);
void UART1_Init(void);
void DMA_UART_TX_Init(uint32_t *memoria, uint16_t num_datos);
void enviarpalabra(char *arreglo, uint16_t longitud);


int main(void)
{
    reloj_36M(); // ESTA PARTE DEL SYSCLK SIEMPRE DEBE DE IR DE PRIMEROOOOOO.
    LED_Init();
    UART1_Init();


    //uint32_t memoria[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}; // estos son los valores de la memoria. las '' es para ser en ascii.
    uint16_t num_datos= 10;
    uint32_t memoria[num_datos];

    //  LLENAMOS LA MEMORIA.
    enviarpalabra("Llenamos la memoria: ", 21);
    for(uint16_t i= 0; i<num_datos; i++)
    {
        if (i <= 9)
        {
            memoria[i]= i+48; // 48 es para convertir el valor en ascii
            while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // ESPERAMOS QUE EL CANAL ESTE DESOCUPADO MEDIANTE LA BANDERA.
            USART_SendData(USART1, memoria[i]);
        }
        else
        {
            memoria[i]=0+48;
        }
    }

    enviarpalabra("\n", 1);
    enviarpalabra("Datos de la memoria: ", 21);
    DMA_UART_TX_Init(&memoria, num_datos); // LLAMAMOS LA CONFIGURACION DEL DMA y envia los datos de la memoria.
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4)== RESET);// espera que se haga toda la transferencia de datos.

    //  LLENAMOS DE NUEVO LA MEMORIA CON DATOS NUEVOS.
    enviarpalabra("\n", 1);
    enviarpalabra("Llenamos de nuevo la memoria: ", 30);
    for(uint16_t i= 0; i<num_datos; i++)
    {
        memoria[i]= num_datos-1-i+48; // 48 es para convertir el valor en ascii
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // ESPERAMOS QUE EL CANAL ESTE DESOCUPADO MEDIANTE LA BANDERA.
        USART_SendData(USART1, memoria[i]);
    }

    // VUELVE A ENVIAR LOS NUEVOS DATOS PRESENTES EN LA MEMORIA
    enviarpalabra("\n", 1);
    enviarpalabra("Nuevos datos de la memoria: ", 29);
    DMA_Cmd(DMA1_Channel4, DISABLE);// primero se deshabilita el dma para hacer el cambio
    DMA_SetCurrDataCounter(DMA1_Channel4, num_datos); // numero de datos.
    DMA_Cmd(DMA1_Channel4, ENABLE); // los envia.
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4)== RESET);// esperar la finalización de la transferencia

    GPIO_ResetBits(GPIOC, GPIO_Pin_13);// encender el led

  while(1)
  {

  }
}



/***********************************************
 * Inicialización del reloj para un sysclk en 36Mhz, con conexion en los perifericos: HCLK, PCLK1, PCLK2.
 ***********************************************/
void reloj_36M(void) // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
{
    // CONFIGURAR EL RELOJ
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
    RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
    RCC_PLLConfig(RCC_PLLSource_HSE_Div2,RCC_PLLMul_9); // 4*9=36
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
    //GPIO_ResetBits(GPIOC, GPIO_Pin_13);
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
 * Configuracion del DMA PARA UART TX, CANAL 4.
 ***********************************************/
 void DMA_UART_TX_Init(uint32_t *memoria, uint16_t num_datos)
 {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // ACTIVAMOS EL RELOJ PARA EL DMA
    DMA_DeInit(DMA1_Channel4); // DESINICIALIZA EL CANAL QUE VAMOS A CONFIGURAR.
    // CONFIGURACION DEL DMA POR DAFAULT PRIMERO
    DMA_InitTypeDef DMA_InitStruct; // CREA LA ESTRUCTURA
    DMA_StructInit(&DMA_InitStruct); // INICIALIZA POR DEFAULT, EL & DICE TOME EL VALOR Y AHI TOMA LO Q TENGA A LA DERECHA.
    // CONFIGURACION DEL DMA REQUERIDA
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) &USART1->DR; // inicialización de la dirección del periférico, en este caso el uart.
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)memoria; // dirección de la memoria que contiene los datos
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST; // desde donde provienen los datos y hacia donde van ??? IRIA COMO DESTINO.
    DMA_InitStruct.DMA_BufferSize = num_datos; // tamaño del buffer
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  // incrementar la posición del periférico? NO.
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;// incrementar la posición de la memoria? SI
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // cuantos bits son los datos del periférico? UNA PALABRA, QUE SERIA 32 BITS.
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; // bits de la memoria
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;// ciclico o normal para los datos en memoria, NORMAL.
    DMA_InitStruct.DMA_Priority = DMA_Priority_Low;// prioridad
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable; // es transferencia entre memorias?
    // INICIALIZA EL DMA Y EL USAR.
    DMA_Init(DMA1_Channel4, &DMA_InitStruct); // inicializa la estructura para el canal 4
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); // habilitación del dma en usart1
    // ENVIA TODO LOS DATOS
    DMA_Cmd(DMA1_Channel4, ENABLE); // HABILITO EL CANAL 4 CON DMA. YA DESDE AQUI COMIENZA A MANDAR TODOS LOS DATOS QUE SE LE PIDIO.
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

