# TIVA-C-TM4C12-LEDS AND INTERRUPTS


 Los pines se definen para usar los leds y botones:
  		LEDS: F0, F4, N0, N1
  		BOTONES: J0, J1
 Cuando se pulsa (y se suelta)un botón, cambia de estado,
 entre los definidos en la matriz LED. El primer botón incrementa el estado
 y el segundo lo decrementa. Al llegar al final, se satura.
