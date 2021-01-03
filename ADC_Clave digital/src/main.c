
/* ******************************************************************************************************
2.	Clave digital. Use un potenciómetro y un monitor serial como
    herramientas para entrar una clave de cuatro dígitos y hacer que se
    encienda el led de la tarjeta. El potenciómetro es la señal analógica
    para convertir en valor digital entre 0 y 9 que se va a enviar al monitor.
    Una vez se muestre en el monitor, el usuario puede oprimir el carácter ‘a’
    para indicar que ese mismo número mostrado en pantalla es una entrada.
    Para programar la clave, el usuario entra ‘bwxyz’ donde ‘w’, ‘x’, ‘y’, y ‘z’
    son los nuevos dígitos de la clave. En este caso 'b' es el carácter que
    reconoce el microcontrolador como comando para entrar una nueva clave.
    Los dígitos se presentan espaciados por un segundo.


    1. Se hace un divisor de voltaje para variar la entrada del ADC1 que esta en A0, asi da diferentes numero para la clave
    2. clave por defecto es 0000
    3. para ingresar la clave, coloque AO en el divisor dependiendo el numero que quiera y cuando el numero se muestre en la pantalla
        si envia A, agrega el numero a la clave de 4 digitos que luego comparara con la clave original.
        si envia B, va a actualizar la contraseña original
        si envia C, manda la contraseña realizada la comparacion con la original
********************************************************************************************************/

#include "stm32f10x_conf.h"

void reloj_56M(void);
void LED_Init(void);
void GPIO_B3_Init(void);
void UART1_Init(void);

void ADC1_Evento_un_canal(uint8_t ADC_Channel_X);

void INTERRUPCION_UART_Init(void);
void Ejecucion_Interrupcion(void);
void USART1_IRQHandler(void);

void RTC_Init(void);

void UART_SEND_numero(uint32_t numero);
void UART_SEND_numero1(uint32_t numero1);
void enviarpalabra(char *arreglo, uint16_t longitud);


uint16_t clave[4] = {0,0,0,0};// vector donde se guarda la clave.
uint16_t clave_digitada[4] = {0,0,0,0};// vector donde se guarda los digitos ingresados por teclado para la clave.
uint16_t adc_value;// variable para leer la conversión, en este caso viene siendo el valor a seleccionar de la clave
uint16_t num_clave;// esta variable dice cuando digitos de la clave lleva
uint16_t num_clave1;// esta variable dice la posicion del digito a cambiar de la clave original.


int main(void)
{
    reloj_56M();
    LED_Init();
    GPIO_B3_Init();
    UART1_Init();

    num_clave = 0;
    num_clave1 = 0;

    ADC1_Evento_un_canal(ADC_Channel_0);
    INTERRUPCION_UART_Init();

    RTC_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL RTC
    RTC_SetAlarm(2);// SE PROGRAMA ALARMA EN 2 SEG.
    RTC_ClearFlag(RTC_FLAG_ALR); // LIMPIAMOS LA BANDERA.
    while(RTC_GetFlagStatus(RTC_FLAG_RTOFF)== RESET); // ESPERAMOS QUE LA BANDERA SE SINCRONICE.
    uint32_t reloj= RTC_GetCounter(); // SE LEE COMO VA EL RELOJ DEL PUERTO GPIO.


  while(1)
  {
        if((RTC_GetCounter()- reloj)>= 1) // ESPERAMOS POR 2 SEGUNDOS.
          {
              reloj= RTC_GetCounter(); // CUANDO ENTRA RECONFIGURAMOS LA VARIABLE RELOJ PARA ENTRAR SOLO CUANDO PASAN 2 SEG.

              while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // ESPERAMOS QUE EL CANAL ESTE DESOCUPADO PARA ENVIAR.
              enviarpalabra("Digito de la clave: ", 20);

              adc_value = ADC_GetConversionValue(ADC1);
              UART_SEND_numero(adc_value);// envía el valor de la convesión a uart
              ADC_SoftwareStartConvCmd(ADC1, ENABLE);// AL AGREGAR ESTA LINEA, HACE QUE TOME MUESTRAS POR EVENTO REPETITIVAMENTE.

          }
        asm("nop");// necesario para no dejar solo el if dentro del loop.
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
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}



/***********************************************
 * CONFIGURA EL GPIO CONECTADO. Inicialización del puerto B, pin 3
 ***********************************************/
void GPIO_B3_Init(void) // ESTO ES HECHO PARA EL GPIO DEL LED PERO SE PUEDE HACER EL MISMO PROCEDIMIENTO PARA CUALQUIER GPIO.
{
    GPIO_InitTypeDef GPIOB_Struct; // CREO LA ESTRUCTURA DEINICIALIZACION, TENIENDO EN CUENTA QUE ES EL PUERTO B
    GPIOB_Struct.GPIO_Pin  = GPIO_Pin_6; // SELECCIONO EL PIN A USAR DEL PUERTO B
    GPIOB_Struct.GPIO_Speed = GPIO_Speed_50MHz; // SELECCIONO LA VELOCIDAD DEL PUERTO. PUEDEN SER 2M, 10M Y 50M.
    GPIOB_Struct.GPIO_Mode = GPIO_Mode_Out_PP; // SELECCIONO EL MODO DEL PUERTO, EN ESTE CASO ES SALIDA PUSH PULL.

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // HABILIDO EL PUERTO C
    GPIO_Init(GPIOA, &GPIOB_Struct); // INICIALIZO LA ESTRUCTURA.
    GPIO_ResetBits(GPIOA,  GPIO_Pin_6);
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
* Inicialización del ADC1 en modo evento para un solo canal. EN EL PUERTO A DESDE EL CANAL 0 AL 9.
***********************************************/
void ADC1_Evento_un_canal(uint8_t ADC_Channel_X)
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
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent; // llenar información de la estructura del adc
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;// initialize the ADC_ScanConvMode member
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE; // Initialize the ADC_ContinuousConvMode member
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Initialize the ADC_ExternalTrigConv member
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; // Initialize the ADC_DataAlign member
    ADC_InitStruct.ADC_NbrOfChannel = 1; // Initialize the ADC_NbrOfChannel member
    ADC_Init(ADC1, &ADC_InitStruct); //ADC1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_X, 1, ADC_SampleTime_239Cycles5); // ADC1, canal x: Entrada al pin PAx
    ADC_Cmd(ADC1, ENABLE);// habilita ADC1

    // calibracion del ADC
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_Cmd(ADC1, ENABLE);// habilita ADC1
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);// inicia la conversión en modo evento por software
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
 * Funcion que correra dentro de la funcion de la inteerupcion.
 ***********************************************/
void Ejecucion_Interrupcion(void)
{
    char dato= (char) USART_ReceiveData(USART1);

    if (num_clave < 4 && dato == 'A') // CASO DONDE SE ESTA LLENANDO LA CONTRASEÑA PARA SER VALIDADA.
    {
        clave_digitada[num_clave] = adc_value / 455;
        num_clave++;

        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("\n\n", 4);
        enviarpalabra("El Numero Seleccionado Fue: ", 28);
        UART_SEND_numero(adc_value);
        enviarpalabra("Para la posicion: ", 18);
        UART_SEND_numero1(num_clave);
        enviarpalabra("\n", 2);

        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("La clave es: ", 13);
        UART_SEND_numero1(clave_digitada[0]);
        UART_SEND_numero1(clave_digitada[1]);
        UART_SEND_numero1(clave_digitada[2]);
        UART_SEND_numero1(clave_digitada[3]);
        enviarpalabra("\n\n", 4);

        GPIO_ResetBits(GPIOC,  GPIO_Pin_13);
        for (int i = 0; i < 2000000; ++i) asm("nop");// retardo
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }

    if (num_clave == 4 && dato == 'A') // CASO DONDE LA CLAVE ESTA COMPLETA PERO SIGUE INTENTANDO LLENARLA
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("\n\n", 4);
        enviarpalabra("Los digitos de la clave ya estan completos, pulsa C para comprobar !!!", 72);
        enviarpalabra("\n\n", 4);
    }

    if (num_clave < 4 && dato == 'C') // CASO DONDE LA CLAVE NO ESTA COMPLETA HE INTENTA COMPROBAR
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("\n\n", 4);
        enviarpalabra("Aun no completas tu clave, vas en la posicion: ", 47);
        UART_SEND_numero1(num_clave);
        enviarpalabra("\n\n", 4);
    }

    if (num_clave == 4 && dato == 'C') // CASO DONDE LA CLAVE ESTA COMPLETA HE INTENTA COMPROBAR
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("\n\n", 4);
        enviarpalabra("La clave digitada fue: ", 23);
        UART_SEND_numero1(clave_digitada[0]);
        UART_SEND_numero1(clave_digitada[1]);
        UART_SEND_numero1(clave_digitada[2]);
        UART_SEND_numero1(clave_digitada[3]);
        enviarpalabra("\n", 2);

        if ( clave[0] == clave_digitada [0] && clave[1] == clave_digitada [1] && clave[2] == clave_digitada [2] && clave[3] == clave_digitada [3] )
        {
            while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
            enviarpalabra("Clave exitosa !!!", 17);
            enviarpalabra("\n\n", 4);

            clave_digitada[0]=0;
            clave_digitada[1]=0;
            clave_digitada[2]=0;
            clave_digitada[3]=0;

            num_clave = 0;

            GPIO_SetBits(GPIOA, GPIO_Pin_6);
        }
        else
        {
            while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
            enviarpalabra("Fallaste, intenta de nuevo !!!", 30);
            enviarpalabra("\n\n", 4);

            clave_digitada[0]=0;
            clave_digitada[1]=0;
            clave_digitada[2]=0;
            clave_digitada[3]=0;

            num_clave = 0;
        }
    }

    if(dato == 'B') // CASO DONDE SE REQUIERE ACTUALIZAR LA CONTRASEÑA.
    {
        clave[num_clave1] = adc_value / 455;
        num_clave1++;

        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("\n\n", 4);
        enviarpalabra("Cambiaste la clave en la posicion: ", 35);
        UART_SEND_numero1(num_clave1);
        enviarpalabra("\n", 2);
        enviarpalabra("Por el numero: ", 15);
        UART_SEND_numero(adc_value);

        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // REVISA EL ESTADO DE LA BANDERA
        enviarpalabra("La nueva clave es: ", 19);
        UART_SEND_numero1(clave[0]);
        UART_SEND_numero1(clave[1]);
        UART_SEND_numero1(clave[2]);
        UART_SEND_numero1(clave[3]);
        enviarpalabra("\n\n", 4);


        if(num_clave1 == 4)
        {
            num_clave1 = 0;
        }

        GPIO_ResetBits(GPIOC,  GPIO_Pin_13);
        for (int i = 0; i < 2000000; ++i) asm("nop");// retardo
        GPIO_SetBits(GPIOC, GPIO_Pin_13);

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
void UART_SEND_numero(uint32_t numero1)
{


    unsigned char tosend[]= {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
    uint32_t valor1;
    uint8_t valor2;
    uint8_t contador= 0;
    uint16_t numero;

    numero = numero1 / 455;

    if(numero== 0)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, numero+0x30);
        enviarpalabra("\n", 2);
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
        enviarpalabra("\n", 2);
    }
    return;
}



/***********************************************
 * envia un numero por el puerto serial
 ***********************************************/
void UART_SEND_numero1(uint32_t numero)
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


