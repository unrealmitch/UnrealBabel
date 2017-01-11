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

	#ifndef strip_modes_h
	#define strip_modes_h

	
		#include <Arduino.h>

		struct Color{
			byte r;
			byte g;
			byte b;
		};

		class strip_modes{
			public:
				strip_modes(int timer);
				Color setStrip(int mode, unsigned long time);
				void setColor(byte r, byte g, byte b);

				Color _color;

			private:
				void strip_modes::rgb();
				
				unsigned long 
					_timer = 100,
					_time = 0;	//Tiempo para mover el array (DynamicColor)
					      	
				bool
					_sum_state = true;

				byte
					_rgb_state = 0;

        
    };

#endif