
// HECHO PARA LA STM32 TARJETA AZUL. ES EL MISMO PROGRAMA DE PRENDER Y APAGAR EL LED PARPADEANDO (Blink)
// PERO SE CUADRO LA FRECUENCIA DEL RELOJ.

#include "stm32f10x_conf.h"

    void reloj_8M(void);  // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_12M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_36M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_64M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK
    void reloj_72M(void); // CREAMOS UNS SUBRUTINA PARA CONFIGURAR EL CLOCK

int main(void)
{
    uint32_t time_conteo = 2000000;

    //configuración del reloj del puerto RCC
    asm("MOVW r0, #0x1000");
    asm("MOVT r0, #0x4002"); //dirección base RCC= 0x4002.1000
    asm("MOVW r2, #0x18"); //offset de RCC_APB2ENR
    asm("LDR  r1, [r0, r2]"); //lee su contenido, VA A LA DIRECCION R0 + R2 Y COGE ESE VALOR Y LO GUARDA EN R1.
    asm("ORR  r1, r1, #0x10"); //bandera de habilitación del puerto C, PONE UN 1 EN EL BIT 3.
    asm("STR  r1, [r0, r2]"); // ALMACENAMOS EN LA POSICION DE MEMORIA INICIAL.

// CARGAMOS A R0 LA DIRECCION DE RCC
// SUMAMOS 0x18 POR QUE ES EL OFFSET DEL RCC_APB2RSTR
// CAMBIAMOS EL 3 BIT POR QUE ES EL DE LOS PUERTOS IO.
// LDR CARGA A REGISTRO, DE MEMORIA A REGISTRO
// STR CARGA A MEMORIA, DE REGISTRO A MEMORIA.

    //configuración de los bits como salida
    asm("MOVW r0, #0x1000"); // base puerto C
    asm("MOVT r0, #0x4001"); //dirección base 0x4001.0C00
    asm("MOVW r2, #0x04"); //offset del registro GPIOB_CRH
    asm("MOVW r1, #0x0000");
    asm("MOVT r1, #0x0010");
    asm("STR  r1, [r0, r2]");

// CARGAMOS EN R0 LA DIRECCION DE MEMORIA DEL GPIO PORT B
// CARGAMOS EN R2 LA DIRECCION PARA CONFIGURACION ALTA (GPIOx_CRH)
// CARGAMOS EN R1 LA CONFIGURACION DESEADA
// GUARDAMOS EN LA POSICION DE MEMORIA R0 + R2 LO QUE HAY EN R1.

    // encender led
    asm("MOVW r2, #0x10"); //offset del registro GPIOB_BSRR
    asm("MOVT r1, #0x2000");
    asm("STR r1, [r0, r2]");

// SEGUIMOS EN LA MISMA DIRECCION DE MEMORIA DEL GPIO PORT B
// PERO ESTA VEZ VAMOS A PRENDER EL LED Y ESO SE HACE EN GPIOx_BSRR
// CARGAMOS EN R1 LA CONFIGURACION DE PRENDER EL LED
// GUARDAMOS EN LA MEMORIA.

    reloj_8M();

    while(1)
    {
        for (uint32_t i = 0; i < time_conteo; ++i) asm("nop");
            // apagar led
            asm("MOVW r0, #0x1000"); // base puerto B
            asm("MOVT r0, #0x4001");
            asm("MOVW r2, #0x10");
            asm("MOVT r1, #0x0000");
            asm("MOVW r1, #0x2000");
            asm("STR r1, [r0, r2]");

        for (uint32_t i = 0; i < time_conteo; ++i) asm("nop");
            // encender led
            asm("MOVW r0, #0x1000"); // base puerto B
            asm("MOVT r0, #0x4001");
            asm("MOVW r2, #0x10");
            asm("MOVW r1, #0x0000");
            asm("MOVT r1, #0x2000");
            asm("STR r1, [r0, r2]");
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



