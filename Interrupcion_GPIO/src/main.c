
// PRENDE Y APAGA EL LED, CUANDO UNDES EL GPIOA4 CON GND O TIERRA.
// PARA USAR LA INTERRUPCION EXTERNA, SE DEBE COLOCAR UN PUERTO GPIO COMO FUNCION ALTERNA.

#include "stm32f10x_conf.h"

void LED_Init(void);
void GPIOA4_Init(void);

void INTERRUPCION_Init(void);
void EXTI4_IRQHandler(void);

uint8_t estado_led = 0; // Bandera usada para hacer un anti rebote.

int main(void)
{
    LED_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL PUERTO C CON EL LED

    GPIOA4_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA EL PUERTO A PIN 4

    INTERRUPCION_Init(); // LLAMAMOS LA FUNCION QUE CONFIGURA LA INTERRUPCION EXTERIOR.

    estado_led = 0;// es la bandera usada

  while(1)
  {
        if(estado_led ==1 )
        {
            if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)==0)
            {
                GPIO_SetBits(GPIOC, GPIO_Pin_13);
            }
            else
            {
                GPIO_ResetBits(GPIOC, GPIO_Pin_13);
            }
            estado_led = 0;
        }
        for(uint32_t i=0; i<2000000; i++) // este es el antirebote.
        {
            asm("nop");
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
 * Inicialización del puerto A, pin 4
 ***********************************************/
void GPIOA4_Init(void)
{// ESTO ES HECHO PARA EL GPIO PUERTO A PIN 4 (GPIOA4) PERO SE PUEDE HACER EL MISMO PROCEDIMIENTO PARA CUALQUIER GPIO.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);// funciones alternas
    GPIO_InitTypeDef GPIOA_Struct; // CREO LA ESTRUCTURA DEINICIALIZACION, TENIENDO EN CUENTA QUE ES EL PUERTO C
    GPIOA_Struct.GPIO_Pin  = GPIO_Pin_4; // SELECCIONO EL PIN A USAR DEL PUERTO A
    GPIOA_Struct.GPIO_Speed = GPIO_Speed_2MHz; // SELECCIONO LA VELOCIDAD DEL PUERTO. PUEDEN SER 2M, 10M Y 50M.
    GPIOA_Struct.GPIO_Mode = GPIO_Mode_IPU; // SELECCIONO EL MODO DEL PUERTO, ENTRADA PULL UP.

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // HABILIDO EL PUERTO A
    GPIO_Init(GPIOA, &GPIOA_Struct); // INICIALIZO LA ESTRUCTURA.
}



/***********************************************
 * Configuracion de la interrupcion
 ***********************************************/
void INTERRUPCION_Init(void)    // configuracion de interrupcion del pin GPIOA04:
{
    // CONFIGURA EL PUERTO QUE USAREMMOS PARA SER INTERRUPCION, EN ESTE CASO EL GPIO A PIN 4 (GPIOA4)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
    // configurar la estructura de la interrupcion
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);
    // configurar el canal
    NVIC_InitTypeDef NVIC_Struct;
    NVIC_Struct.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Struct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
    // inicializar la interrupcion
    NVIC_Init(&NVIC_Struct);
}



/***********************************************
 * Funcion de la interrupcion
 ***********************************************/
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)// lo que se ejecuta es lo que hay dentro del if.
    {
        estado_led = 1;  // ESTA ES LA ACCION QUE VA A REALIZAR CUANTO SE EJECUTE LA INTERRUPCION.
        EXTI_ClearITPendingBit(EXTI_Line4); // lo que esta arriba de el es lo que se ejecuta cuando entre la interrupcion.
    }
}
