/**********************************************************************
FUNCIONAMIENTO:

    Iniciar con un contador en cero.
    Cada segundo el contador se va a incrementar en uno.
    Este conteo se envía por i2c2 (maestro) y se recibe por i2c1(esclavo).
    Luego se lee en i2c1 y se envía por spi1 hacia spi2.
    Luego se lee por spi2 y se envía a pantalla.
    Durante el recorrido, en la recepción de i2c1 se le suma 1 y en la recepción de spi2, se le resta 1.

CONEXIONES:

     *Poner una resistencia en PB6 y PB7, cada una a Vcc (+3.3).
     I2C1_SCL (PB6) <- I2C2_SCL (PB10)
     I2C1_SDA (PB7) <- I2C2_SDA (PB11)

     SPI1_NSS (PA4)   -->  No conectar
     SPI1_SCK (PA5)   -->  SPI2_SCK (PB13)
     SPI1_MISO (PA6)  <--  SPI2_MISO (PB14)
     SPI1_MOSI (PA7)  -->  SPI2_MOSI (PB15)
     SPI2_NSS (PB12)  -->  GND

**********************************************************************/

#include "stm32f10x_conf.h"

#define I2CMASTER_ADDR 25 // LA DIRECCION DEL I2C2, PUEDE SER ENTRE LA 1 Y 127, PUEDE SER LA QUE QUIERA
#define I2CSLAVE_ADDR  36 // LA DIRECCION DEL I2C1
#define I2C1_CLOCK_FRQ 100000 // LA FRECUENCIA QUE UNO QUIERA

void Sysclk_72M(void);
void RTC_Init(void);

void UART_Init(void);
void UART_mensaje(char* mensaje, uint8_t largo);
void UART_numero(uint32_t numero);

void I2C1_Esclavo_init(void);
void I2C2_Maestro_init(void);
void I2C1_EV_IRQHandler(void); // FUNCIONES QUE CORRE EN LA INTERRUPCION
void I2C1_ClearFlag(void); // PARA BORRAR BANDERAS
void Send_I2C2(uint8_t address_slave, uint8_t data); // enviar de maestro a esclavo

void Spi1_Maestro_Conf(void);
void Spi2_Esclavo_Conf(void);

uint8_t dato_send_i2c; // En esta variable se guarda lo enviado entre I2c.
uint8_t dato_send_SPI; // En esta variable se guarda lo enviado entre spi.

int main(void)
{

    Sysclk_72M(); // INICIALIZACION DEL SYSCLOCK DE LA PASTILLA CON 72MHZ
    UART_Init(); // INICIALIZACION DEL UART1

    RTC_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL RTC
    RTC_SetAlarm(1);// SE PROGRAMA ALARMA EN 2 SEG.
    RTC_ClearFlag(RTC_FLAG_ALR); // LIMPIAMOS LA BANDERA.
    while(RTC_GetFlagStatus(RTC_FLAG_RTOFF)== RESET); // ESPERAMOS QUE LA BANDERA SE SINCRONICE.
    uint32_t reloj= RTC_GetCounter(); // SE LEE COMO VA EL RELOJ DEL PUERTO GPIO.

    Spi1_Maestro_Conf(); // INICIALIZACION DEL SPI1
    Spi2_Esclavo_Conf(); // INICIALIZACION DEL SPI2

    I2C2_Maestro_init(); // INICIALIZACION DEL I2C2 GENERICO
    I2C1_Esclavo_init(); // INICIALIZACION DEL I2C1 GENERICO

    uint32_t Cont = 0;  //VARIABLE QUE IRA CONTANDO


  while(1)
  {
      if ((RTC_GetCounter()- reloj)>= 1) // ESPERAMOS POR 1 SEGUNDOS.
          {
              reloj = RTC_GetCounter(); // CUANDO ENTRA RECONFIGURAMOS LA VARIABLE RELOJ PARA ENTRAR SOLO CUANDO PASAN 1 SEG.

              Send_I2C2(I2CSLAVE_ADDR, Cont); // Se manda el dato por el i2c2 hacia lo que tenga conectado, que en este caso es el i2c1 del mismo micro.

              while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE)== RESET);
              UART_mensaje("Llega el Dato al SP12 \r\n", 25);
              dato_send_SPI = SPI_I2S_ReceiveData(SPI2); // recibimos el dato enviado desde el SPI1 del mismo micro

              UART_mensaje("\r\n", 2);
              UART_mensaje("CONTADOR_SPI2: ", 14);
              UART_numero(dato_send_SPI-1);
              UART_mensaje("\r\n", 2);
              UART_mensaje("\r\n", 2);

              Cont++;
          }
  }
}


///===================================================== CONFIGURACION TIME ===========================================================

/***********************************************
 * Inicializacion del reloj Sysclk en 72 Mhz, igual en HCLK y PCLK2.
 * PCLK1 en la mitad
 ***********************************************/
void Sysclk_72M(void)
{
    // usa el reloj interno mientras se configura
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    // deshabilita el pll para poder configurarlo
    RCC_PLLCmd(DISABLE);
    //8Mhz/2*14= 56Mhz
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);
    // espera que se estabilice
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)== RESET)
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    // HCLK = SYSCLK
    RCC_HCLKConfig( RCC_SYSCLK_Div1);
    // PCLK2 = HCLK
    RCC_PCLK2Config( RCC_HCLK_Div1);
    // PCLK1 = HCLK/2
    RCC_PCLK1Config( RCC_HCLK_Div2);
}

/***********************************************
 * Configuracion del RTC
 ***********************************************/
void RTC_Init(void)
{ // ESTA ES LA FORMA NORMAL, LA OTRA FORMA DE USAR RTC ES LA USADA PARA INTERRUCIONES EXTERNAS.
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

///==================================================================================================================================



///===================================================== CONFIGURACION UART ===========================================================

/***********************************************
 * Inicializacion del puerto serial Serial1
 ***********************************************/
void UART_Init(void)
{
    GPIO_InitTypeDef GPIO_Struct;
    // Inicializa parametros de Uart 9600 bps, 8 bits, 1 stop, no paridad
    USART_InitTypeDef UART_Struct;
    USART_StructInit(&UART_Struct);
    UART_Struct.USART_BaudRate= 9600;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    // GPIOA PIN9 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_9;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Struct);

    // GPIOA PIN9 funcion alterna
    GPIO_Struct.GPIO_Pin = GPIO_Pin_10;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_Struct);

    USART_Cmd(USART1, ENABLE);
    USART_Init(USART1, &UART_Struct);
}

/***********************************************
 * envia un mensaje por el puerto serial
 ***********************************************/
void UART_mensaje(char* mensaje, uint8_t largo)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, '\r');

    for(uint8_t i= 0; i<largo; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, mensaje[i]);
    }
    return;
}

/***********************************************
 *  Enviar un numero por uart en ASCII
 ***********************************************/
void UART_numero(uint32_t numero)
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
//    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//    USART_SendData(USART1, ' ');
    return;
}

///==================================================================================================================================



///===================================================== CONFIGURACION I2C ===========================================================

/***********************************************
 * Inicializacion del I2C2 como maestro
 ***********************************************/
void I2C2_Maestro_init(void)
{
    I2C_DeInit(I2C2);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // Configura pines I2C2: SCL y SDA
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // configuracion I2C2 como maestro.
    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2CMASTER_ADDR;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C1_CLOCK_FRQ;

    // habilita I2C2
    I2C_Cmd(I2C2, ENABLE);
    // aplica configuración I2C2
    I2C_Init(I2C2, &I2C_InitStructure);
}

/***********************************************
 * Inicializacion del I2C1 como  esclavo, usa interrupción
 ***********************************************/
void I2C1_Esclavo_init(void)
{
    I2C_DeInit(I2C1);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // Configura pines I2C2: SCL y SDA
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // configuración I2C
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2CSLAVE_ADDR; // <---- AQUI ES DONDE SE COLOCA LA DIRECCION DEL ESCLAVO.
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C1_CLOCK_FRQ;

    // habilita I2C1
    I2C_Cmd(I2C1, ENABLE);
    // aplica configuración I2C1
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_StretchClockCmd(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    // configuración de interrupción
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = I2C1_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    //Configura interrupción en caso de error en I2C
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    // habilitación de banderas de interrupción
    I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);

    return;
}

/***********************************************
 * Subrutina de tratamiento a interrupción por eventos del i2c1 como esclavo
 * Es la funcion que se activa cuando ocurre la interrupcion
 ***********************************************/
void I2C1_EV_IRQHandler(void)
{
    uint32_t event;

    event = I2C_GetLastEvent(I2C1); // lee el último evento

    switch(event)
    {
        /// ============ RECIBIR EN EL ESCLAVO =============
        case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED: // el maestro ha enviado la dirección del esclavo para LEER un byte al esclavo. ESTO ES UNA BANDERA QUE LE ALERTA AL OTRO I2C Q VA A SER ESCLAVO Y VA A RECIBIR
        {
            break;
        }

        case I2C_EVENT_SLAVE_BYTE_RECEIVED: // aqui entra cuando el maestro envia algo.
        {
            // el maestro ha enviado un byte al esclavo
            if(I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF)== SET)
            {
                (void)(I2C1->SR1); // banderas para enviar o recibir algo
            }
            dato_send_i2c = I2C_ReceiveData(I2C1);

            UART_mensaje("Llega el Dato al I2C1 \r\n", 24);

            while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)== RESET);
            UART_mensaje("Se Envia el Dato Desde el SPI1 al SP12 \r\n", 43);
            SPI_I2S_SendData(SPI1, dato_send_i2c+1);

            break;
        }
        /// =================================================

        /// ============ TRANSMITIR POR ESCLAVO ============= ------- no se usa!!
        case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED: // el maestro ha enviado la dirección del esclavo para ENVIAR un byte del esclavo. ESTO ES UNA BANDERA QUE LE ALERTA AL OTRO I2C Q VA A SER ESCLAVO Y VA A TRANSMITIR
        {
            break;
        }

        case I2C_EVENT_SLAVE_BYTE_TRANSMITTING: // aqui entra cuando el slave manda el dato.
        {
            break;
        }
        /// =================================================

        case I2C_EVENT_SLAVE_ACK_FAILURE: // aqui entra cuando hay una falla o error de reconocimiento
        {
            //UART_mensaje("esclavo: I2C_EVENT_SLAVE_ACK_FAILURE\r\n",38);
            I2C1->SR1 &= 0x00FF;
            break;
        }

        case I2C_EVENT_SLAVE_STOP_DETECTED: //el maestro ha parado la comunicación
        {
            I2C1_ClearFlag();
            break;
        }

        default:
        {
            break;
        }
    }
}

/***********************************************
 *  El maestro Manda un dato por i2c al esclavo
 ***********************************************/
void Send_I2C2(uint8_t address_slave, uint8_t data)
{
  UART_mensaje("Se Envia el Dato Desde el I2C2 al I2C1 \r\n", 43);

  I2C_GenerateSTART(I2C2,ENABLE); // envía condición de inicio
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C2, address_slave, I2C_Direction_Transmitter); // voy a transmitirte info, envia una bandera
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C2,data); // ENVIO EL DATO AL I2C CONECTADO
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTOP(I2C2,ENABLE); // se le dice q ya termino de enviar
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  return;
}

/***********************************************
 * Limpiar el estado de las banderas
 ***********************************************/
void I2C1_ClearFlag(void)
{
    // ADDR-Flag clear
    while((I2C1->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR)
    {
        I2C1->SR1;
        I2C1->SR2;
    }

    // STOPF Flag clear
    while((I2C1->SR1&I2C_SR1_STOPF) == I2C_SR1_STOPF)
    {
        I2C1->SR1;
        I2C1->CR1 |= 0x1;
    }
    return;
}

///==================================================================================================================================



///===================================================== CONFIGURACION SPI ===========================================================

/***********************************************
 * Inicializacion del spi1 como maestro
 ***********************************************/
void Spi1_Maestro_Conf(void)
{
    //habilitación del reloj para spi1, funciones alternas y puerto A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
    // Configuración de los pines del spi1
    GPIO_InitTypeDef GPIO_Struct;
    // GPIOA PIN5 (SPI1_SCK), PIN7(SPI1_MOSI) Y PIN4 (SPI1_NSS) función alterna- salidas
    GPIO_Struct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_4;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Struct);

    // GPIOA PIN6 (SPI1_MISO) funcion alterna- entrada
    GPIO_Struct.GPIO_Pin = GPIO_Pin_6;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_Struct);

    // configuración de SPI1 como maestro
    SPI_InitTypeDef SPI_InitStruct;
    // configuración de SPI1 como maestro con una frecuencia de 7 Mhz, maestro y NSS por software
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_8; // 56Mz/ 8= 7 Mbps
    SPI_InitStruct.SPI_Mode= SPI_Mode_Master;
    SPI_InitStruct.SPI_NSS= SPI_NSS_Soft;
    SPI_Init(SPI1, &SPI_InitStruct);

    // habilitación spi1
    SPI_Cmd(SPI1, ENABLE);

    return;
}

/***********************************************
 * Inicializacion del spi2 como esclavo
 ***********************************************/
void Spi2_Esclavo_Conf(void)
{
    //habilitación del reloj para spi2, funciones alternas y puerto B
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
    // Configuración de los pines del spi2
    GPIO_InitTypeDef GPIO_Struct;
    // GPIOB PIN14 (SPI2_MISO) función alterna- salida
    GPIO_Struct.GPIO_Pin = GPIO_Pin_14;
    GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_Struct);

    // GPIOB PIN12 (SPI2_NSS), PIN13 (SPI2_SCK) y PIN15 (SPI2_MOSI) funcion alterna- entradas
    GPIO_Struct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_Struct);

    // configuración del SPI2 como esclavo
    SPI_InitTypeDef SPI_InitStruct;
    // configuración de SPI2 como esclavo con una frecuencia de 7 Mhz, maestro y NSS por software
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_4; // 28Mz/ 4= 7 Mbps
    SPI_InitStruct.SPI_Mode= SPI_Mode_Slave;
    SPI_InitStruct.SPI_NSS= SPI_NSS_Soft;
    SPI_Init(SPI2, &SPI_InitStruct);

    // habilitación SPIs
    SPI_Cmd(SPI1, ENABLE);
    SPI_Cmd(SPI2, ENABLE);

    return;
}



