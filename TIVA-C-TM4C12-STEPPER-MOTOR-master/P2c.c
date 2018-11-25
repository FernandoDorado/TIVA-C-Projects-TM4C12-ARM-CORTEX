//#define PART_TM4C1294NCPDT
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"


/*************
 * Ejemplo de manejo de un motor paso a paso conectado en el boosterpack 1 (pines PF1, PF2, PF3, PG0)
 * Si se pulsa un botón gira en un sentido, y si se pulsa el otro botón gira en sentido contrario.
 * Si no se pulsa ninguno, permanece en reposo.
 *************/

#define MSEC 40000
#define MaxEst 10


uint32_t Puerto[]={
        GPIO_PORTF_BASE,
        GPIO_PORTF_BASE,
        GPIO_PORTF_BASE,
        GPIO_PORTG_BASE,

};
uint32_t Pin[]={
        GPIO_PIN_1,
        GPIO_PIN_2,
        GPIO_PIN_3,
        GPIO_PIN_0,
        };

int Step[4][4]={1,0,0,0,
                0,0,0,1,
                0,0,1,0,
                0,1,0,0
};

#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

int RELOJ;
int  movimiento;
volatile int secuencia=0;
volatile int posicion=0;     //Posicion relativa desde el arranque. entre 0 y 513
volatile int angulo=0;      //angulo relativo, en grados (para GUI_COMPOSER)
int paso=0;

void IntTimer(void);
void inter(void);



int main(void)
{
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)
    TimerLoadSet(TIMER0_BASE, TIMER_A, (RELOJ/200)-1);
    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer);


    IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout
    IntMasterEnable();  //Habilitacion global de interrupciones
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0, 1, 2A y 2B

    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);  // Habilitar pines de interrupción J0, J1
    GPIOIntRegister(GPIO_PORTJ_BASE, inter);                //Registrar (definir) la rutina de interrupción
    IntEnable(INT_GPIOJ);                                   //Habilitar interrupción del pto J
    IntMasterEnable();                                      // Habilitar globalmente las ints



    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);


    // Configuración para el modo de bajo consumo 
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOJ | SYSCTL_PERIPH_TIMER0); // Habilitar el Puerto J y el T0 durante el modo de bajo consumo
    SysCtlPeripheralClockGating(true);                // Habilitar el apagado de los periféricos


    while(1){

        if(B1_OFF && B2_OFF){    //Si no pulsamos ningun boton
            movimiento=0;
            SysCtlSleep();       // Entrada en modo de bajo consumo
        }
    }
}


void IntTimer(void)
{
    int i;
    if(movimiento!=0){
    if(movimiento==1) secuencia+=1;
    else if(movimiento==2) secuencia-=1;
    if(secuencia==-1) secuencia=3;
    if(secuencia==4) secuencia=0;
    for(i=0;i<4;i++)  GPIOPinWrite(Puerto[i],Pin[i],Pin[i]*Step[secuencia][i]);
    }
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

}

void inter(void)
{
    SysCtlDelay(15*MSEC);
    if (B1_ON) movimiento = 1;
    else if(B2_ON) movimiento = 2;
    GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);

}
