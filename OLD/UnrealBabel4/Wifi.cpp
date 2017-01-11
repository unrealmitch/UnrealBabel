/*

  Library:	ESP8266.h
  Info:		Library for control server web with ESP8266 chip
  Date:		10-2016
  Author: 	UnrealMitch
  Mail:		unrealmitch@gmail.com
  Web:		https://github.com/unrealmitch
  License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)
  
*/

#include "Wifi.h"

//						//
//		Contructor		//
//						//

Wifi::Wifi(byte rx, byte tx) : _serial_wifi(SoftwareSerial(rx,tx)){
	_pin_rx = rx;
	_pin_tx = tx;
	_status = 0;
}

//						//
//			AUX			//
//						//

Wifi::wait_command(String command){
	wait_command(command,"OK",1000,false);
}

Wifi::wait_command(String command, String cut, int timeout, bool print){

	unsigned long time = millis();
	String input = ""; int i = 0;

	while(true){
		//Serial.println("[W] C:" + command);
		_serial_wifi.println(command);
		time = millis();
		while( time + timeout > millis() ){
			input = read();
			if(input != "" && print) Serial.println(input);
			if(input.indexOf(cut) > -1) return;
			if(input.indexOf("ERROR") > -1) return;
		}
	}
	delay(5);
}

bool Wifi::check(){

	unsigned long time = millis();
	String input = ""; int i = 0;

	for(int i = 3; i>0; i--){
		_serial_wifi.println("AT");
		time = millis();
		delay(200);
			read();read();read();read();
			input = read();
			if(input.indexOf("OK") > -1) return true;
	}

	return false;
}



//						//
//		Comunication	//
//						//

bool Wifi::available(){
	return _serial_wifi.available();
}
Wifi::send(char input){
	if(input != '\n'){
		_command+=input;
	}else{
		_serial_wifi.println(_command);
		_command = "";
	}
}

String Wifi::read(){
	if(_serial_wifi.available()){
		char input = _serial_wifi.read();
		if(input != '\n' && input != '\r'){
			_output+= input;
			return "";
		}else{
			String tmp_output = _output;
			_output = "";
			//Serial.println("F:" + tmp_output);
			return tmp_output;
		}
	}

	return "";
}

//						//
//		Server			//
//						//

Wifi::start(String ssid, String passwd, int port){

	Serial.println("[W] 0 - Starting system");
	_serial_wifi.begin(115200);
	_serial_wifi.println("AT+CIOBAUD=74880");
	_serial_wifi.begin(74880);
	_status=0;

	if(!check()) return false;
	//wait_command("AT");
	_status = 1;
	Serial.println("[W] 1 - Connecting");
	wait_command("AT+CWMODE=3");
	wait_command("AT+CWJAP=\""+ssid+"\",\"" + passwd + "\"","OK", 10000,false);
	Serial.println("[W] 2 - Connected");
	//wait_command("AT+CIFSR","OK", 1000, true);
	Serial.println("[W] 3 - Starting Server");
	wait_command("AT+CIPMUX=1");
	wait_command("AT+CIPSERVER=1,80");
	Serial.println("[W] 4 - Server UP!");
	return true;
}

Wifi::faststart(){
	_serial_wifi.begin(74880);
	if(!check()) return false;
	wait_command("AT+CIPMUX=1");
	wait_command("AT+CIPSERVER=1,80");
	//Serial.println("[W] Server UP!");
	return true;
}

int * Wifi::getQuest(){
	int int_arg[10];
	int i, cut;

	String input = read();

	if(input.indexOf("+IPD") == -1) return NULL;
		
	String args = input.substring(input.indexOf("[")+1, input.indexOf("]"));
			
	while(true){
		cut = args.indexOf(";");
		if(cut == -1) break;
		int_arg[i]=args.substring(0,cut).toInt();
		args = args.substring(cut+1);
		i++;
	}

	if (i > 0){
		int * return_arg = malloc(i*sizeof(int));

		for(int j = 0; j<i;j++){
			return_arg[j] = int_arg[j];
		}

		return return_arg;
	}
	
}



Wifi::server(){
	String input = read();

	if(input.length() > 10){
		if(input.substring(0,4) == "+IPD"){
			//_serial_wifi.println("AT+CIPSEND=0,2");
			//_serial_wifi.println("NO");
			//_serial_wifi.println("AT+CIPCLOSE=0");

			String args = input.substring(input.indexOf("GET")+4, input.indexOf("HTTP"));
			Serial.println("SS" + input);
			//String clientID = "0"; //input.substring(5,input.indexOf(",",5));
			//String output = buildWeb();
			//String cmd = "AT+CIPSEND=0," + output.length();// + clientID + "," + output.length();
			//_serial_wifi.println("AT+CIPSEND=0,6");
			//_serial_wifi.println(output);
			//_serial_wifi.println("AT+CIPCLOSE=0");
			//wait_command(cmd,"OK",1000, true);
			//wait_command(output,"OK",5000, true);
			//_serial_wifi.println("AT+CIPCLOSE=0");
		}
	}
}

String Wifi::buildWeb(){
	String html;

	html  = "<html>";
	html +=     "<head>";
	html +=       "<title>{1}</title>";
	html +=     "</head>";
	html +=     "<body>";
	html +=       "<h1>{1}</h1>";
	html +=       "<p>Segundos en funcionamiento: {2}</p>";
	html +=     "</body>";
	html += "</html>";
	html.replace( "{1}", "Servidor web ESP8266" );
	html.replace( "{2}", String(millis()/1000)  );

	html = "prueba";

	return html;
}