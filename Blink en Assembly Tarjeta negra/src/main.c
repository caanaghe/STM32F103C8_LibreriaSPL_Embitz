
// Blink con Assembly. El funcionamiento es prender y apagar el led de la tarjeta cada 2 segundos, el tiempo se maneja a partir de un RTC.
// HECHO PARA LA TARJETA NEGRA. PRENDE Y APAGA EL LED PARPADEANDO.

#include "stm32f10x_conf.h"

int main(void)
{
    //configuración del reloj del puerto
    asm("MOVW r0, #0x1000");
    asm("MOVT r0, #0x4002"); //dirección base RCC= 0x4002.1000
    asm("MOVW r2, #0x18"); //offset de RCC_APB2ENR
    asm("LDR r1, [r0, r2]"); //lee su contenido, VA A LA DIRECCION R0 + R2 Y COGE ESE VALOR Y LO GUARDA EN R1.
    asm("ORR r1, r1, #0x08"); //bandera de habilitación del puerto B, PONE UN 1 EN EL BIT 3.
    asm("STR r1, [r0, r2]"); // ALMACENAMOS EN LA POSICION DE MEMORIA INICIAL.

// CARGAMOS A R0 LA DIRECCION DE RCC
// SUMAMOS 0x18 POR QUE ES EL OFFSET DEL RCC_APB2RSTR
// CAMBIAMOS EL 3 BIT POR QUE ES EL DE LOS PUERTOS IO.
// LDR CARGA A REGISTRO, DE MEMORIA A REGISTRO
// STR CARGA A MEMORIA, DE REGISTRO A MEMORIA.

    //configuración de los bits como salida
    asm("MOVW r0, #0x0C00"); // base puerto B
    asm("MOVT r0, #0x4001"); //dirección base 0x4001.0C00
    asm("MOVW r2, #0x04"); //offset del registro GPIOB_CRH
    asm("MOVW r1, #0x0000");
    asm("MOVT r1, #0x0001");
    asm("STR r1, [r0, r2]");

// CARGAMOS EN R0 LA DIRECCION DE MEMORIA DEL GPIO PORT B
// CARGAMOS EN R2 LA DIRECCION PARA CONFIGURACION ALTA (GPIOx_CRH)
// CARGAMOS EN R1 LA CONFIGURACION DESEADA
// GUARDAMOS EN LA POSICION DE MEMORIA R0 + R2 LO QUE HAY EN R1.

    // encender led
    asm("MOVW r2, #0x10"); //offset del registro GPIOB_BSRR
    asm("MOVT r1, #0x1000");
    asm("STR r1, [r0, r2]");

// SEGUIMOS EN LA MISMA DIRECCION DE MEMORIA DEL GPIO PORT B
// PERO ESTA VEZ VAMOS A PRENDER EL LED Y ESO SE HACE EN GPIOx_BSRR
// ACRGAMOS EN R1 LA CONFIGURACION DE PRENDER EL LED
// GUARDAMOS EN LA MEMORIA.

    while(1)
    {
    for (uint32_t i = 0; i < 2000000; ++i) asm("nop");
    // apagar led
    asm("MOVW r0, #0x0C00"); // base puerto B
    asm("MOVT r0, #0x4001");
    asm("MOVW r2, #0x10");
    asm("MOVT r1, #0x0000");
    asm("MOVW r1, #0x1000");
    asm("STR r1, [r0, r2]");
    for (uint32_t i = 0; i < 2000000; ++i) asm("nop");
    // encender led
    asm("MOVW r0, #0x0C00"); // base puerto B
    asm("MOVT r0, #0x4001");
    asm("MOVW r2, #0x10");
    asm("MOVW r1, #0x0000");
    asm("MOVT r1, #0x1000");
    asm("STR r1, [r0, r2]");
    }
}
