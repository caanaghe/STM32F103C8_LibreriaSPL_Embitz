
// HECHO PARA LA STM32 TARJETA AZUL.
// PROGRAMA PARA ENCENDER Y APAGAR UN LED POR RTC (Blink). ADEMAS YA DEJAMOS ASSEMBLY Y HACEMOS TODO CON C.

#include "stm32f10x_conf.h"

    void reloj_8M(void);  // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_12M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_36M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_64M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_72M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK

int main(void)
{

/* ASSEMBLY
    //configuración del reloj del puerto RCC
    asm("MOVW r0, #0x1000");
    asm("MOVT r0, #0x4002"); //dirección base RCC= 0x4002.1000
    asm("MOVW r2, #0x18"); //offset de RCC_APB2ENR
    asm("LDR  r1, [r0, r2]"); //lee su contenido, VA A LA DIRECCION R0 + R2 Y COGE ESE VALOR Y LO GUARDA EN R1.
    asm("ORR  r1, r1, #0x10"); //bandera de habilitación del puerto C, PONE UN 1 EN EL BIT 3.
    asm("STR  r1, [r0, r2]"); // ALMACENAMOS EN LA POSICION DE MEMORIA INICIAL.
*/
    //configuración del reloj del puerto C RCC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // PARA EL LED

 /* ASSEMBLY
    configuración de los bits como salida
    asm("MOVW r0, #0x1000"); // base puerto C
    asm("MOVT r0, #0x4001"); //dirección base 0x4001.0C00
    asm("MOVW r2, #0x04"); //offset del registro GPIOB_CRH
    asm("MOVW r1, #0x0000");
    asm("MOVT r1, #0x0010");
    asm("STR  r1, [r0, r2]");
*/

    //configuración de los bits como salida, PARA EL LED.
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin= GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Speed= GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Mode= GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);


    // encender led
    //asm("MOVW r2, #0x10"); //offset del registro GPIOB_BSRR
    //asm("MOVT r1, #0x2000");
    //asm("STR r1, [r0, r2]");

    /// FUNCIONES PARA ESCRIBIR EN EL GPIO Y PRENDER EL LED.
    GPIO_SetBits(GPIOC, GPIO_Pin_13); // APAGA EL LED
    //GPIO_ResetBits(GPIOC, GPIO_Pin_13); // ENCIENDE EL LED
    //GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); // APAGA EL LED, PERO SE PUEDE ESCRIBIR CERO O UNO
    //GPIO_Write(GPIOC, 0x0000); // ENCIENDE EL LED, TIENE Q SER UN VALOR DE 16 BITS
                                // 00X0 0000 0000 0000

    reloj_8M(); // funcion para configurar el sysclock

    /// CONFIGURACION DEL RTC
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

    /// inicializacion del RTC
    RTC_SetAlarm(2);// programa alarma para 2 segundos
    RTC_ClearFlag(RTC_FLAG_ALR);
    while(RTC_GetFlagStatus(RTC_FLAG_RTOFF)== RESET);
    uint32_t reloj= RTC_GetCounter();

  while(1)
  {
      if((RTC_GetCounter()- reloj)>= 2)
      {
          reloj= RTC_GetCounter();
          if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)== 1)
              GPIO_ResetBits(GPIOC,  GPIO_Pin_13);
          else
              GPIO_SetBits(GPIOC, GPIO_Pin_13);
      }
  }
}

    void reloj_8M(void) // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    {
       // CONFIGURAR EL RELOJ
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
        RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div2,RCC_PLLMul_2); // 4*2
        RCC_PLLCmd(ENABLE); // habilita el PLL
        while((RCC->CR & RCC_CR_PLLRDY) == 0); // espera que se estabilice.
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // se escoge PLL como Sysclk

    }

    void reloj_12M(void) // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    {
       // CONFIGURAR EL RELOJ
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
        RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div2,RCC_PLLMul_3);
        RCC_PLLCmd(ENABLE); // habilita el PLL
        while((RCC->CR & RCC_CR_PLLRDY) == 0); // espera que se estabilice.
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // se escoge PLL como Sysclk

    }

        void reloj_36M(void) // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    {
       // CONFIGURAR EL RELOJ
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
        RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div2,RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE); // habilita el PLL
        while((RCC->CR & RCC_CR_PLLRDY) == 0); // espera que se estabilice.
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // se escoge PLL como Sysclk

    }

        void reloj_64M(void) // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    {
       // CONFIGURAR EL RELOJ
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
        RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_8);
        RCC_PLLCmd(ENABLE); // habilita el PLL
        while((RCC->CR & RCC_CR_PLLRDY) == 0); // espera que se estabilice.
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // se escoge PLL como Sysclk

    }

        void reloj_72M(void) // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    {
       // CONFIGURAR EL RELOJ
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // Asegura PLL no esté en uso
        RCC_PLLCmd(DISABLE);// para cambiar multiplicador, debe deshabilitarse el PLL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE); // habilita el PLL
        while((RCC->CR & RCC_CR_PLLRDY) == 0); // espera que se estabilice.
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // se escoge PLL como Sysclk

    }

