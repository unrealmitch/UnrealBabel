/*
	Library:	module_colors.h
	Info:		Module to get colors to ledstrep [configurable]
	Date:		10-2014
	Author: 	UnrealMitch
	Mail:		unrealmitch@gmail.com
	Web:		https://github.com/unrealmitch
	License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)
*/

//Modos de color para leds VU_Meter

	#ifndef module_colors_h
	#define module_colors_h

		#include <Arduino.h>

		class module_colors{
			public:
				module_colors(int timer_arco);
				void ColorVU(byte * VU_colors, byte mode_led, byte  nled, byte msg);

			private:
				unsigned long 
					time_arco,
					_timer_arco;	//Tiempo para mover el array (DynamicColor)

				int 
					arco_start, 				  //Inicio ColorVU en el array
					arco_max,				      //Número máximo de recorrido en el array
					counter_led;

				byte
					//Colores para modo 5 (Static color change by beat)
  				sc_red, sc_green, sc_blue;
        
				void
					DynamicColor(byte * VU_colors, byte nled, byte array[]),
					ManualColor(byte * VU_colors, byte nled, byte array[]);
    };

#endif