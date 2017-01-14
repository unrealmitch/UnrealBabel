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

Wifi::Wifi(){
}

//						//
//		Comunication	//
//						//

bool Wifi::available(){
	return Serial1.available();
}

Wifi::send(char input){
	if(input != '\n'){
		_command+=input;
	}else{
		Serial1.println(_command);
		_command = "";
	}
}

String Wifi::read(){
	while(Serial1.available()){
		char input = Serial1.read();
		Serial.write(input);
		if(input != '\n' && input != '\r'){
			_output+= input;
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
		Serial1.println(command);
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
		Serial1.println("AT");
		time = millis();
		delay(200);
			read();read();read();read();
			input = read();
			if(input.indexOf("OK") > -1) return true;
	}

	return false;
}

//						//
//		Server			//
//						//

Wifi::setup(String ssid, String passwd, int port){

	Serial.println("[W] 0 - Starting system");
	Serial1.begin(115200);

	if(!check()) return false;
	Serial.println("[W] 1 - Connecting");
	wait_command("AT+CWMODE=3");
	wait_command("AT+CWJAP=\""+ssid+"\",\"" + passwd + "\"","OK", 10000,false);

	Serial.println("[W] 2 - Connected");
	wait_command("AT+CIFSR","OK", 1000, true);
	Serial.println("[W] 3 - Starting Server");
	wait_command("AT+CIPMUX=1");
	wait_command("AT+CIPSERVER=1," + port);
	Serial.println("[W] 4 - Server UP!");
	return true;
}

bool Wifi::start(){
	Serial1.begin(115200);
	if(!check()) return false;
	wait_command("AT+CIPMUX=1");
	wait_command("AT+CIPSERVER=1,80");
	Serial.println("[W] Server UP!");
	return true;
}

int * Wifi::getOrder(){
	int int_arg[4] = {0,0,0,0};

	Serial.println("| IO |"); 
	while(Serial1.available()){
		char input = Serial1.read();
		Serial.write(input);
	}

	return int_arg;

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
			//Serial1.println("AT+CIPSEND=0,2");
			//Serial1.println("NO");
			//Serial1.println("AT+CIPCLOSE=0");

			String args = input.substring(input.indexOf("GET")+4, input.indexOf("HTTP"));
			 //Serial.println("SS" + input);
			//String clientID = "0"; //input.substring(5,input.indexOf(",",5));
			//String output = buildWeb();
			//String cmd = "AT+CIPSEND=0," + output.length();// + clientID + "," + output.length();
			//Serial1.println("AT+CIPSEND=0,6");
			//Serial1.println(output);
			//Serial1.println("AT+CIPCLOSE=0");
			//wait_command(cmd,"OK",1000, true);
			//wait_command(output,"OK",5000, true);
			//Serial1.println("AT+CIPCLOSE=0");
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