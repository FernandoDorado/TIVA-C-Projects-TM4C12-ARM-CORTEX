#include <stdint.h>
#include <stdbool.h>
#include "driverlib2.h"
#include <math.h>


 void IntTimer1(void);
 int i=0, PeriodoPWM,t=0;
 float sen[50],t_rad;
 const float dospi=6.28;


 int main(void)
{
    uint32_t periodo1;
    uint32_t Reloj;

    Reloj = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);       //Habilita T1
    TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);   //T1 a 120MHz
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);    //T1 igual...
    periodo1 = Reloj/5; //0.2s
    TimerLoadSet(TIMER1_BASE, TIMER_A, periodo1 -1);
    TimerIntRegister(TIMER1_BASE,TIMER_A,IntTimer1);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();  //Habilitacion global de interrupciones
    TimerEnable(TIMER1_BASE, TIMER_A);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_4);    // al PWM le llega un reloj de 30MHz
    GPIOPinConfigure(GPIO_PF0_M0PWM0);           //Conectar el pin a PWM
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0); //Tipo de pin PWM
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PeriodoPWM=29999; // 1kHz a 30M
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PeriodoPWM); //frec:1kHz
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PeriodoPWM*0.01);   //Inicialmente, un 1%
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);     //Habilita el generador 0
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT , true);    //Habilita la salida 0


     for(t=0;t<50;t++)
     {
       t_rad=(float)t*dospi/50.0;
       sen[t]=sin(t_rad);
     }


    while(1)
    {
        //Bucle infinito en el que no se hace nada
    }
}




void IntTimer1(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    float duty;
    i++; if(i==50) i=0;
    t_rad=(float)t*dospi/50.0;
    duty=sen[i];
    duty=(float)PeriodoPWM * (duty + 1)/2.0;
    duty=duty*0.98+0.01; //Fijamos entre 1 y 99%
    //Corregir
    PWMPulseWidthSet(PWM0_BASE,PWM_OUT_0,(int) duty);

}



