/*
  Library:	Wifi.h
  Info:		Library for control server web with ESP8266 chip
  Date:		10-2016
  Author: 	UnrealMitch
  Mail:		unrealmitch@gmail.com
  Web:		https://github.com/unrealmitch
  License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)

*/

#ifndef Wifi_h
	#define Wifi_h

	#include <Arduino.h>
	#include <SoftwareSerial.h>
	#include <String.h>


	class Wifi{
		public:
			Wifi(byte, byte);
			start(String, String, int);
			server();
			send(char);
			String read();
			String buildWeb();
			int * getQuest();
			bool available();
			bool check();

		
		private:
			wait_command(String,String,int,bool);
			wait_command(String);
			SoftwareSerial _serial_wifi;
			byte 
				_pin_rx,
				_pin_tx,
				_status;
			String
				_command = "",
				_output = "";
	};
#endif