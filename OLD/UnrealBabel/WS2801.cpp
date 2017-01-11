/*
  Library:	WS2801.h
  Info:		Library for control Ledstrip (chip:2801)
  Date:		10-2014
  Author: 	UnrealMitch
  Mail:		unrealmitch@gmail.com
  Web:		https://github.com/unrealmitch
  License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)
*/

#include "WS2801.h"

WS2801::WS2801(byte num_leds, byte pinData, byte pinClock){
	_pinData= pinData;
	_pinClock= pinClock;
	_numLeds = num_leds;

	_ArrayStream = new byte * [num_leds];

	for (int i = 0; i < num_leds; i++)
		_ArrayStream[i] = new byte[3];

}

void WS2801::led_start(){
	//LedStream

	pinMode(_pinData, OUTPUT);
	pinMode(_pinClock, OUTPUT);


	LED_DDR  |=  LED_PIN; // Enable output for LED
	LED_PORT &= ~LED_PIN; // LED off

  	
	SPI.begin();	
	//SPI.setMOSI(_pinData);
	//SPI.setCLK(_pinClock);

	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV16); // 1 MHz max, else flicker
}

void WS2801::led_set(byte led, byte red, byte green, byte blue){
	_ArrayStream[led][0] = red;
	_ArrayStream[led][1] = green;
	_ArrayStream[led][2] = blue;
}

void WS2801::led_set(int led, int red, int green, int blue){
	if (red < 0) red = 0; 		if (red > 255) red = 255;
	if (green < 0) green = 0;	if (green > 255) green = 255;
	if (blue < 0) blue = 0;		if (blue > 255) blue = 255;

	led_set(led,red,green,blue);
}

void WS2801::led_mirror(){ //Duplica el lado izquierdo de la malla led en el derecho
	int i,k,
		led_side = _numLeds / 2;

	for(i=0; i < (_numLeds / 2) ; i++){
		for(k=0; k<3; k++){
			_ArrayStream[i][k] = _ArrayStream[led_side + i][k];
		}
	}
}

void WS2801::led_reset(){ //Set off all leds
	int i, k;
	for (i=0;i<_numLeds;i++){
		for(k=0; k<3; k++){
			_ArrayStream[i][k] = 0;
		}
	}

}

void WS2801::led_send(){  //Envia el buffer de colores a los leds [grb]

	led_mirror();

	for(int i=0; i<_numLeds ; i++){
		for(SPDR = _ArrayStream[i][1]; !(SPSR & _BV(SPIF)); );
		for(SPDR = _ArrayStream[i][2]; !(SPSR & _BV(SPIF)); );
		for(SPDR = _ArrayStream[i][0]; !(SPSR & _BV(SPIF)); );
	}
}