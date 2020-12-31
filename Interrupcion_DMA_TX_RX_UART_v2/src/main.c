// ejemplo de dma aplicado al usart1 en tx y rx, canal 4 y 5, respectivamente
// hace la siguiente secuencia:
// memoria a periférico:
// 1. lleva el contenido de memoria de 10 datos para ser transferidos por uart a una pc
// 2. se modifica el contenido de la memoria y se envían los datos.
// periférico a memoria:
// 3. se espera por 10 datos enviados desde la pc
// memoria a periférico:
// 4. se modifica el contenido de los datos llegados a memoria y se envían a la pc
//    usando dma con interrupción por finalización de transferencia.

#include "stm32f10x_conf.h"

void reloj_36M(void);
void LED_Init(void);
void UART1_Init(void);
void enviarpalabra(char *arreglo, uint16_t longitud);
void DMA_UART_TX_Init(uint32_t *memoria, uint16_t num_datos);
void DMA_UART_RX_Init(uint32_t *memoria, uint16_t num_datos);
void INTERRUPCION_DMA_TX_Init(void);
void DMA1_Channel4_IRQHandler(void);


int main(void)
{
    reloj_36M();
    LED_Init();
    UART1_Init();

    // 1. memoria que tiene los datos para ser transferidos.
    enviarpalabra("Datos de la memoria: ", 21);
    uint32_t memoria[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint16_t num_datos= 10;

    DMA_UART_TX_Init(&memoria, num_datos); // CONFIGURACION DEL DMA COMO TX.
    DMA_ClearFlag(DMA1_FLAG_TC4); // LIMPIA LA BANDERA.
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4)== RESET); // espera que se haga toda la transferencia de datos

    // 2. cambiar los datos iniciales
    enviarpalabra("\n", 1);
    enviarpalabra("Llenamos la memoria con datos nuevos: ", 38);
    for(uint16_t i= 0; i<num_datos; i++)
    {
        memoria[i]= num_datos-1-i+48;// 48 es para convertir el valor en ascii
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // ESPERAMOS QUE EL CANAL ESTE DESOCUPADO MEDIANTE LA BANDERA.
        USART_SendData(USART1, memoria[i]);
    }

    // VUELVE A ENVIAR LOS NUEVOS DATOS PRESENTES EN LA MEMORIA
    enviarpalabra("\n", 1);
    enviarpalabra("Nuevos datos de la memoria: ", 29);
    DMA_Cmd(DMA1_Channel4, DISABLE);// primero se deshabilita el dma para hacer el cambio
    DMA_SetCurrDataCounter(DMA1_Channel4, num_datos); // numero de datos.
    DMA_Cmd(DMA1_Channel4, ENABLE); // los envia.
    DMA_ClearFlag(DMA1_FLAG_TC4); // limpiamos la bandera.
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4)== RESET);// esperar la finalización de la transferencia

    // 3. ahora se cambia la configuración para enviar datos desde la pc.
    // PRIMERO ES NECESARIO DESHABILITAR EL PUERTO QUE SE VENIA USANDO, EN ESTE CASO EL 4 DE TX.
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_DeInit(DMA1_Channel4);
    // envio de datos desde el pc a la memoria del micro.
    enviarpalabra("\n", 1);
    DMA_UART_RX_Init(&memoria, num_datos); // CONFIGURACION DEL DMA COMO TX.
    DMA_ClearFlag(DMA1_FLAG_TC5); // LIMPIA LA BANDERA.
    while(DMA_GetFlagStatus(DMA1_FLAG_TC5)== RESET); // espera que se haga toda la transferencia de datos, en este caso que le lleguen todos los datos que se propuso de bufer (num_datos).

    enviarpalabra("\n", 1);
    enviarpalabra("Datos por teclado en la memoria: ", 34);

    // Visualizar lo escrito por teclado, mediante DMA por Tx
    // PRIMERO ES NECESARIO DESHABILITAR EL PUERTO QUE SE VENIA USANDO, EN ESTE CASO EL 5 DE RX.
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_DeInit(DMA1_Channel5);

    DMA_UART_TX_Init(&memoria, num_datos); // CONFIGURACION DEL DMA COMO TX.
    DMA_ClearFlag(DMA1_FLAG_TC4); // LIMPIA LA BANDERA.
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4)== RESET); // espera que se haga toda la transferencia de datos.

    // configuración de interrupción por terminación de transferencia
    INTERRUPCION_DMA_TX_Init();

    enviarpalabra("\n", 1);

  while(1)
  {

  }
}


/***********************************************
 *  CONFIGURA EL SYSCLOCK EN 8Mh CON CONEXION EN LOS PERIFERICOS: HCLK, PCLK1, PCLK2.
 ***********************************************/
void reloj_36M(void)
{
    // CONFIGURAR EL RELOJ
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
    RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
    RCC_PLLConfig(RCC_PLLSource_HSE_Div2,RCC_PLLMul_2); // 4*2=8
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
 * CONFIGURA EL GPIO CONECTADO AL LED. Inicialización del puerto C, pin 13
 ***********************************************/
void LED_Init(void) // ESTO ES HECHO PARA EL GPIO DEL LED PERO SE PUEDE HACER EL MISMO PROCEDIMIENTO PARA CUALQUIER GPIO.
{
    GPIO_InitTypeDef GPIOC_Struct; // CREO LA ESTRUCTURA DEINICIALIZACION, TENIENDO EN CUENTA QUE ES EL PUERTO C
    GPIOC_Struct.GPIO_Pin  = GPIO_Pin_13; // SELECCIONO EL PIN A USAR DEL PUERTO C
    GPIOC_Struct.GPIO_Speed = GPIO_Speed_2MHz; // SELECCIONO LA VELOCIDAD DEL PUERTO. PUEDEN SER 2M, 10M Y 50M.
    GPIOC_Struct.GPIO_Mode = GPIO_Mode_Out_PP; // SELECCIONO EL MODO DEL PUERTO, EN ESTE CASO ES SALIDA PUSH PULL.

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // HABILIDO EL PUERTO C
    GPIO_Init(GPIOC, &GPIOC_Struct); // INICIALIZO LA ESTRUCTURA.
    //GPIO_ResetBits(GPIOC, GPIO_Pin_13);// PRENDO EL LED.
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
 * Configuracion del DMA PARA UART RX, CANAL 5. PARA RECIBIR DATOS DEL PC.
 ***********************************************/
 void DMA_UART_RX_Init(uint32_t *memoria, uint16_t num_datos)
 {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // ACTIVAMOS EL RELOJ PARA EL DMA
    DMA_DeInit(DMA1_Channel5); // DESINICIALIZA EL CANAL QUE VAMOS A CONFIGURAR.
    // CONFIGURACION DEL DMA POR DAFAULT PRIMERO
    DMA_InitTypeDef DMA_InitStruct; // CREA LA ESTRUCTURA
    DMA_StructInit(&DMA_InitStruct); // INICIALIZA POR DEFAULT, EL & DICE TOME EL VALOR Y AHI TOMA LO Q TENGA A LA DERECHA.
    // CONFIGURACION DEL DMA REQUERIDA
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) &USART1->DR; // inicialización de la dirección del periférico, en este caso el uart.
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)memoria; // dirección de la memoria que contiene los datos
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC; // desde donde provienen los datos y hacia donde van ??? IRIA COMO DESTINO.
    DMA_InitStruct.DMA_BufferSize = num_datos; // tamaño del buffer
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  // incrementar la posición del periférico? NO.
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;// incrementar la posición de la memoria? SI
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // cuantos bits son los datos del periférico? UNA PALABRA, QUE SERIA 32 BITS.
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; // bits de la memoria
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;// ciclico o normal para los datos en memoria, NORMAL.
    DMA_InitStruct.DMA_Priority = DMA_Priority_Low;// prioridad
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable; // es transferencia entre memorias?
    // INICIALIZA EL DMA Y EL USAR.
    DMA_Init(DMA1_Channel5, &DMA_InitStruct); // inicializa la estructura para el canal 4
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); // habilitación del dma en usart1
    // ENVIA TODO LOS DATOS
    DMA_Cmd(DMA1_Channel5, ENABLE); // HABILITO EL CANAL 4 CON DMA. YA DESDE AQUI COMIENZA A MANDAR TODOS LOS DATOS QUE SE LE PIDIO.
 }



/***********************************************
 * Configuracion de la interrupcion por el puerto seria 1 uart.
 ***********************************************/
void INTERRUPCION_DMA_TX_Init(void)
{
    // CONFIGURA EL PUERTO QUE USAREMMOS PARA SER INTERRUPCION, EN ESTE CASO EL CANAL 4 DEL DMA.
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    // configurar el canal
    NVIC_InitTypeDef NVIC_Struct;
    NVIC_Struct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Struct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
    // inicializar la interrupcion
    NVIC_Init(&NVIC_Struct);
}



/***********************************************
 * Funcion de la interrupcion
 ***********************************************/
void DMA1_Channel4_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC4))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        GPIO_SetBits(GPIOC, GPIO_Pin_13);// APAGO EL LED.
    }

}




