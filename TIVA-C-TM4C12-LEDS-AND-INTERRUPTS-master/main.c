#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "driverlib2.h"
#include "inc/hw_ints.h"

/************************************************************
 * Primer ejemplo de manejo de pines de e/s, usando el HW de la placa
 * Los pines se definen para usar los leds y botones:
 * 		LEDS: F0, F4, N0, N1
 * 		BOTONES: J0, J1
 * Cuando se pulsa (y se suelta)un botón, cambia de estado,
 * entre los definidos en la matriz LED. El primer botón incrementa el estado
 * y el segundo lo decrementa. Al llegar al final, se satura.
 ************************************************************/


#define MSEC 40000 //Valor para 1ms con SysCtlDelay()


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))
enum estado_maq {M1_E0, M1_E1, M2_E0, M2_E1, M2_E2, M2_E3 , M3_E0, M3_E1 };


uint16_t modo_ant;
int modo;
uint16_t estado;

void rutina_interrupcion(void)
{
    if(B1_ON)
    {
        while(B1_ON);
        SysCtlDelay(20*MSEC);
        modo++;
        if(modo==3) modo=0;          //Por si satura
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
    }
    if(B2_ON)
      {
          while(B2_ON);
          SysCtlDelay(20*MSEC);
          modo--;
          if(modo<0) modo=2;      //Decrementa el estado. Si menor que cero, satura.
          GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);
      }


}

int LED[8][4]={
        1,1,1,1,
		0,0,0,0,
		0,0,0,1,
		0,0,1,1,
		0,1,1,1,
		1,1,1,1,
		1,0,1,0,
		0,1,0,1,
};



uint32_t reloj=0;


void enciende(uint8_t Est)
{
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1*LED[Est][0]);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[Est][1]);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4*LED[Est][2]);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[Est][3]);
}



int main(void)
{


    //Fijar velocidad a 120MHz
    reloj=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);



    //Habilitar los periféricos implicados: GPIOF, J, N
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //Definir tipo de pines
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4); //F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1); //N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1


    modo=0;
    modo_ant=0;
    enciende(estado);

    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);  // Habilitar pines de interrupción J0, J1
    GPIOIntRegister(GPIO_PORTJ_BASE, rutina_interrupcion);  //Registrar (definir) la rutina de interrupción
    IntEnable(INT_GPIOJ);                                   //Habilitar interrupción del pto J
    IntMasterEnable();                                      // Habilitar globalmente las ints


    while(1){
        enciende(estado);
                if(modo!=modo_ant){  //Para no estar continuamente accediendo a los puertos...

                    modo_ant=modo;
                    enciende(estado);

           }

                switch(estado){

                           case M1_E0:
                           SysCtlDelay(100*MSEC);
                           estado=M1_E1;
                           break;

                           case M1_E1:
                           SysCtlDelay(900*MSEC);
                           estado=M1_E0;
                               if (modo==1)
                                   {estado=M2_E0;break;}

                               if(modo==2)
                                   {estado=M3_E0;break;}
                           break;

                           case M2_E0:
                           SysCtlDelay(1000*MSEC);
                           estado=M2_E1;
                           if (modo==2)
                               {estado=M3_E0;break;}
                           if (modo==0)
                               {estado=M1_E0;break;}
                           break;

                           case M2_E1:
                            SysCtlDelay(1000*MSEC);
                            estado=M2_E2;
                            if (modo==2)
                                {estado=M3_E0;break;}
                            if (modo==0)
                                {estado=M1_E0;break;}
                            break;

                           case M2_E2:
                            SysCtlDelay(1000*MSEC);
                            estado=M2_E3;
                            if (modo==2)
                                {estado=M3_E0;break;}
                            if (modo==0)
                                {estado=M1_E0;break;}
                            break;

                           case M2_E3:
                            SysCtlDelay(1000*MSEC);
                            estado=M2_E0;
                            if (modo==2)
                                {estado=M3_E0;break;}
                            if (modo==0)
                                {estado=M1_E0;break;}
                            break;

                           case M3_E0:
                               SysCtlDelay(500*MSEC);
                               estado=M3_E1;
                               if (modo==0)
                                   {estado=M1_E0;break;}
                               if (modo==2)
                                   {estado=M2_E0;break;}
                           break;

                           case M3_E1:
                               SysCtlDelay(500*MSEC);
                               estado=M3_E0;
                               if (modo==0)
                                   {estado=M1_E0;break;}
                               if (modo==2)
                                   {estado=M2_E0;break;}
                           break;

                           }

    }
}







