#include "stm32f10x_conf.h"

void LED_Init(void);

int main(void)
{
    LED_Init();
    // habilitacion del reloj
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    // habilita acceso al rtc
    PWR_BackupAccessCmd(ENABLE);
    // habilita el reloj externo
    RCC_LSEConfig(RCC_LSE_ON);
    // espera hasta que el reloj se estabilice
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    // selecciona el reloj externo como fuente
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    // habilita el rtc
    RCC_RTCCLKCmd(ENABLE);
    // espera por sincronización
    RTC_WaitForSynchro();
    while(RTC_GetFlagStatus(RTC_FLAG_RTOFF)== RESET);
    RTC_SetPrescaler(32768); // divisor del reloj externo
    RTC_SetCounter(0);
    uint32_t reloj= RTC_GetCounter();



  while(1)
  {
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET)
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    else
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    // pone contador en ceros
    while((RTC_GetCounter()- reloj)< 2);
    reloj= RTC_GetCounter();// polling
  }
}

/***********************************************
 * Inicialización del puerto B, pin 12
 ***********************************************/
void LED_Init(void)
{
    GPIO_InitTypeDef GPIOB_Struct;
    GPIOB_Struct.GPIO_Pin  = GPIO_Pin_12;
    GPIOB_Struct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIOB_Struct.GPIO_Mode = GPIO_Mode_Out_PP;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_Init(GPIOB, &GPIOB_Struct);
}
