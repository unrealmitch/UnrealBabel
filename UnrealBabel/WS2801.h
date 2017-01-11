/*
  Library:	WS2801.h
  Info:		Library for control Ledstrip (chip:2801)
  Date:		10-2014
  Author: 	UnrealMitch
  Mail:		unrealmitch@gmail.com
  Web:		https://github.com/unrealmitch
  License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)
*/

#ifndef WS2801_h
	#define WS2801_h

	#include <Arduino.h>
	#include <SPI.h>

	#define LED_DDR  DDRB
	#define LED_PORT PORTB
	#define LED_PIN  _BV(PORTB5)

	class WS2801{
		public:
			WS2801(byte num_leds, byte pinData, byte pinClock);
			
			void
				led_start(),
				led_set(byte led, byte red, byte green, byte blue),
				led_set(int led, int red, int green, int blue),
				led_mirror(),
				led_reset(),
				led_send();


		private:
			byte
				_pinData,
				_pinClock,
				_numLeds;

			byte ** _ArrayStream;
	};
#endif
