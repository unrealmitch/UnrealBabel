
/*
 	Proyect:	UnrealBabel Sound
 	Version:	1 [based in "uLed Music v2.3"]
 	Info:		Arduino proyect for analyce audio and do differente effect 
  				with a diferent led strips.

 	Date:		21-10-2015
 	Author: 	UnrealMitch
 	Mail:		unrealmitch@gmail.com
 	Web:		https://github.com/unrealmitch
 	License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)

 	Changelog:
 				0.1:Added more than 1 argument for serial comunication.
 					Added one strip [5050 - same color - RGB]
 					Added support for 5050 [bass]
 				0.2:Added support wifi communication
 				1.0:Adapted to Arduino Mega
 					Changed wifi to real serial port
 					Added watchdog
 					Added autoselect mode [Wifi timeout]


*/

#include <FastLED.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "strip_modes.h"
#include "WS2801.h"
#include "module_colors.h"
#include <avr/wdt.h>

//AUX
#define TRUE 1
#define FALSE 0

//Entrada
#define PIN_AUDIO_L 3 					//Pin In of Audio Input [R]
#define PIN_AUDIO_R 0 					//Pin In of Audio Input [L]

//Salida
#define PIN_LED 8 						//Pin Out of Leds Data WS2811

#define PIN_LED_R 5						//Top Leds
#define PIN_LED_G 6
#define PIN_LED_B 7

#define PIN_LED2_R 3					//Bot Leds
#define PIN_LED2_G 2					
#define PIN_LED2_B 4					

//Comunication
#define SERIAL_ARGS 4
#define WIFI_ARGS 4

//Control
#define NUM_LEDS 50

//Debug
#define DEBUG 0
#define DEBUG_VU 0
#define DEBUG_WIFI 1

//Leds Distribution
#define num_leds_rgb 8					//Num Leds BEAT [RGB Top]		
#define num_leds_rgb_bot 3				//Num Leds BEAT 2 [RGB Bot / side]
#define num_leds_side 18 				//Num Leds VU Metter per side: ((NUM_LEDS - num_leds_rgb - num_leds_rgb_bot * 2) / 2)	

//Fire
#define NUM_LEDS_FIRE 24
#define FRAMES_PER_SECOND 30
#define COOLING  55
#define SPARKING 120


/********************************************************************************************************/
/********************************************************************************************************/

/* 												Variables												*/

/********************************************************************************************************/
/********************************************************************************************************/



//Instances clase
module_colors colors_instance(125);

//Led
CRGBArray<NUM_LEDS> strip;		//Colors of all leds of strip 5050 with WS2812
byte VU_colors[3] = {0,0,0};	//Temp array for set color of one led

//Colors of RGB
byte led1_color[3] = {0,0,0};	//Color of Strip1 of LEDS 5050
byte led2_color[3] = {0,0,0};	//Color of Strip2 of LEDS 5050
strip_modes *smode;				//Object of strip color modes


//Serial Communication
byte arg_i = 0;
unsigned long lastAckTime;

//Wifi Communication
String wifi_buffer = "";

//Change Values [Metter] and diferrence of levels (sound)
int incremento = 7;	//Level differences to up LED

//Maximos
int max_sound=0;	//Max reached in time unit
int vmax = 0;		//Max local audio [Short period timer_vmax_less]
int vmax_abs = 0;	//Max absolute audio [Long period timer_vmax_rst]
int max_led=0;  	//Led reached highest

//Buffer of colors
byte const buffer_array_color_size = 9;
byte buffer_array_color[buffer_array_color_size][3];
byte last_rgb_color[3];

//Beat Variables
struct beat_info_str{
	boolean state_last;
	boolean state_now;
	boolean beat_detected;
} beat_info;


/****************************************/
/*				CONFIG					*/
/****************************************/

	//   MODOS  //
	int mode = 0;
	int smode_selected = 0;

	byte mode_rgb = 2;
	/*RGB mode: 
	0-> Disabled
	1-> Activate [BEAT] by bass & acute [EQ]
	2-> Activate [BEAT] by bass & acute [EQ] & Replicate in VU
	*/

	byte mode_colors = 4; 
	const byte mode_colors_max = 5;
	/*Colours of VU meter
	0->Progresive G to B
	1->Semi Static G-Y-R-V-B
	2->Constant V-B-G-Y-R-W
	3->Rainbow R-G
	4->Rainbow R-G-B (GAY!!!!)
	5->Single color changed by beat (random color)
	*/


	byte mode_beats = 2;
	const byte mode_beats_max = 2;
	/*BEAT Modes:
	0-> Move array with temp
	1-> Move array with beat
	2-> Copy Beat RGB color in all leds (VU METTER)
	*/

	byte mode_strip = 1;
	const byte mode_strip_max = 2;
	/*Strip LED modes:
	0-> Off
	1-> On
	*/

	//   PARAMS   //

	//EQ
	int beat_bass_min = 512, beat_acute_min = 512;				//Minimun analog value to detect Beat
	bool beat_bass_gradual = 1, beat_acute_gradual = 1;			//Flash or Gradual change state led at Beat Detect
	bool bass_color [3] = {1,0,0}, acute_color [3] = {0,0,1};	//Color for Beats Detect
	bool beat_bass_eqtovu = 0, beat_bass_eqtovu_gradual = 0, bass_color_vu [3] = {1,0,0};	//Same values for Bass Detect in VU Meter [no RGB]

	//VU Metter
	bool MaxLed_on = TRUE; 			//Still on for a moment the led of last max.
	bool MaxLed_Degree_Vu = TRUE; 	//All Vu descend as do MaxLed
	bool VU_Meter_Static = FALSE; 	//Set 1 for turn on all leds of VU Meter


	//RGB Variables
	bool MinToChange_On = TRUE;		//Activate if it's necesary a min. to change color of rgb
	bool already_activate_degrade = FALSE;	// If degrade with delay is already activate
	bool degree_delay_on = TRUE;	//Active delay that the rgb wait to degrade
	bool degree_on = TRUE;			//Modo degradado

	const int MinToChange = 60; 	//Min value of all colors of RGB to can change color [Mode 3]
	

/****************************************/
/*				TIMERS					*/
/****************************************/

	//VU_Meter_Max_LED
	unsigned long time_max_led;
	long timer_max_led = 10;
	int timer_max_led_start = 100;
	int timer_max_led_cont  = 20;

	//Dynamic calc
	unsigned long time_dinamic_calc;
	long timer_dinamic_calc = 2500;

	//Degraded effect [BEAT RGB]
	unsigned long time_degree;
	const long timer_degree = 50;	
	const int value_degree = 8;

	//Activate Degree [BEAT RGB]
	unsigned long time_degree_delay;
	const long timer_degree_delay = 500;

	//BEAT move array [BEAT RGB]
	unsigned long time_beat_move_array;
	const long timer_beat_move_array = 25;

	//Attenuate BEAT Leds
	unsigned long time_attenuate;
	unsigned long timer_attenuate = 60;
	unsigned long timer_attenuate_first = 600;
	byte attenuate_subtract = 20;



/********************************************************************************************************/
/********************************************************************************************************/

/* 												Functions												*/

/********************************************************************************************************/
/********************************************************************************************************/



					/****************************************/
					/*					AUX					*/
					/****************************************/


void(* resetFunc) (void) = 0;

//Send a msg for serial port: "msj: valor"
void print_int(int valor,String msj){
	Serial.print(msj);
	Serial.println(valor);
}

//Get arduino's free ram [in bytes]
int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

//Auxiliary function for DisplayConfig to turn on leds by mode 
int getLedMode(int startLed, int numLeds, int actual){
	//if (21 <= (startLed+actual) <=28) return -1;
	if (actual > numLeds/2) return ( (49-startLed-2) - (actual-numLeds/2) + 1);
	else return (2 + startLed + actual - 1);
}



					/****************************************/
					/*		Strip5050 [LedX] & RGBColor		*/
					/****************************************/


void Leds_set(){
	analogWrite(PIN_LED_R, led1_color[0]);
	analogWrite(PIN_LED_G, led1_color[1]);
	analogWrite(PIN_LED_B, led1_color[2]);

	analogWrite(PIN_LED2_R, led2_color[0]);
	analogWrite(PIN_LED2_G, led2_color[1]);
	analogWrite(PIN_LED2_B, led2_color[2]);
}

void Led1_SetColor(byte red, byte green, byte blue){
	led1_color[0] = red;
	led1_color[1] = green;
	led1_color[2] = blue;
}

void Led2_SetColor(byte red, byte green, byte blue){
	led2_color[0] = red;
	led2_color[1] = green;
	led2_color[2] = blue;
}

void Leds_SetColor(byte red, byte green, byte blue){
	led1_color[0] = red;
	led1_color[1] = green;
	led1_color[2] = blue;
	
	led2_color[0] = red;
	led2_color[1] = green;
	led2_color[2] = blue;
}

void Leds_SetColorRnd(){
	//Set RGB leds with random color

	int max_value = 255;
	last_rgb_color[0] = random(max_value);
	last_rgb_color[1] = random(max_value);
	last_rgb_color[2] = random(max_value);

	led1_color[0] = last_rgb_color[0];
	led1_color[1] = last_rgb_color[1];
	led1_color[2] = last_rgb_color[2];

	Led1_SetColor(led1_color[0],led1_color[1],led1_color[2]);
}

void Leds_set_byColor(Color color){
	analogWrite(PIN_LED_R, color.r);
	analogWrite(PIN_LED_G, color.g);
	analogWrite(PIN_LED_B, color.b);

	analogWrite(PIN_LED2_R, color.r);
	analogWrite(PIN_LED2_G, color.g);
	analogWrite(PIN_LED2_B, color.b);
}

void Led1_fade(unsigned long time){
	byte max = led1_color[0];
	if(led1_color[1]>max) max = led1_color[1]>max;
	if(led1_color[2]>max) max = led1_color[2]>max;

	byte subs = attenuate_subtract;
	//byte subs = byte (((int) attenuate_subtract * (int) led1_color[i]) / (int) max); //map(attenuate_subtract,0,max,0,led1_color[i]);
	if(time_attenuate <= time){
		for(int i = 0; i < 3;i++){
			
			if(led1_color[i] >= subs)
				led1_color[i]-= subs;
			else
				led1_color[i] = 0;
		}
		Leds_set();
		time_attenuate = time + timer_attenuate;
	}
}

void Leds_clear(){
	Led1_SetColor(0,0,0);
	Led2_SetColor(0,0,0);
	Leds_set();
	ResetStream();
	SendStream();
}



					/****************************************/
					/*				StreamWS2081			*/
					/****************************************/



//Stream LEDs
void Duplicate(){ //Mirror the left side of the LED strip on the right
	int i; int j;
	for (i=NUM_LEDS-1; i > (NUM_LEDS-1-num_leds_side-num_leds_rgb_bot); i--){
		j=NUM_LEDS-1-i;
		SetStream(strip[j].g,strip[j].r,strip[j].b,i);
	}
}

void ResetStream(){
	//Set off all leds
	
	int i;

	for (i=0;i<NUM_LEDS;i++){
		SetStream(0,0,0,i);
	}

}
void SetStream(int r, int g, int b, int pos){ 
	//Set color of one led in strim

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	strip[pos].r = g;
	strip[pos].g = r;
	strip[pos].b = b;
}

void SendStream(){
	//Send the buffer of colours [leds] to Ledstrim

	Duplicate();
	FastLED.show();
}

void SendStream_nodup(){
	//Send the buffer of colours [leds] to Ledstrim without duplicate

	FastLED.show();
}



					/****************************************/
					/*			COLOR - StreamWS2081		*/
					/****************************************/



void SetRgb(int ired, int igreen, int iblue){ //SET RGB leds with beat sound

	int i;
	int div = 1;
	int leds_active = num_leds_side;

	if (mode==3)
		leds_active = 0;

	for (i = 0; i < num_leds_rgb_bot; i++){
		SetStream(ired/div,igreen/div,iblue/div,i);
	}

	for (i = num_leds_side + num_leds_rgb_bot ; i < num_leds_side + num_leds_rgb_bot + num_leds_rgb; i++){
		SetStream(ired/div,igreen/div,iblue/div,i);
	}
}

void SetRgb_Top(int ired, int igreen, int iblue){ //Set RGB leds in top

	int i;

	for (i = num_leds_side + num_leds_rgb_bot ; i < num_leds_side + num_leds_rgb_bot + num_leds_rgb; i++){
		SetStream(ired,igreen,iblue,i);
	}
}


void SetRgb_Bot(int ired, int igreen, int iblue){ //Set RGB leds in button

	int i;
	int div = 1;
	int leds_active = num_leds_side;

	if (mode==3)
		leds_active = 0;

	for (i = 0; i < num_leds_rgb_bot; i++){
		//SetStream(ired/div,igreen/div,iblue/div,i);
		SetStream(ired/div,igreen/div,iblue/div,i);
	}

}


void SetRgb_Bass(int ired, int igreen, int iblue){ //Set RGB leds of Bass

	int i;

	for (i = 1; i < num_leds_rgb_bot; i++){
		//SetStream(ired/div,igreen/div,iblue/div,i);
		SetStream(ired,igreen,iblue,i);
	}

	for (i = num_leds_side + num_leds_rgb_bot + 2; i < num_leds_side + num_leds_rgb_bot + num_leds_rgb - 2; i++){
		SetStream(ired,igreen,iblue,i);
	}

}

void SetRgb_Acute(int ired, int igreen, int iblue){ //Set RGB leds of Acutes

	int i;

	for (i = 0; i < 1; i++){
		//SetStream(ired/div,igreen/div,iblue/div,i);
		SetStream(ired,igreen,iblue,i);
	}

	SetStream(ired,igreen,iblue,21);
	SetStream(ired,igreen,iblue,22);
	SetStream(ired,igreen,iblue,27);
	SetStream(ired,igreen,iblue,28);
}

void RGB_MSG(int* led, int num_leds, int red, int green,int blue, int num_flash, int tdelay){	
	//Send a msg for flash of leds

	for (int k = 0; k < num_flash; k++){
		for(int i = 0; i < num_leds; i++){
			SetStream(red,green,blue,led[i]);
		}

	SendStream_nodup();

	delay(tdelay);

		for(int i = 0; i < num_leds; i++){
			SetStream(0,0,0,led[i]);
		}

	SendStream_nodup();
	delay(tdelay);
	}

}



					/****************************************/
					/*			AUDIO - StreamWS2081		*/
					/****************************************/



boolean Rgb_Low_Values(int min){	
	//If is a necesary min to change color of leds (MinToChange_On need set 1)
	if(MinToChange_On == 0) return true;
	if(led1_color[0] > min) return false;
	if(led1_color[1] > min) return false;
	if(led1_color[2] > min) return false;

	return true;
}


byte SetLeds(byte lvl_audio, byte lvl_min){ 
	//VU Meter: Turn on leds depending on sound level

	byte  i;
	byte leds_on = 0;
	byte lvl_i = lvl_min;

	for(i=0;i<num_leds_side;i++){ 

		colors_instance.ColorVU(VU_colors,mode_colors,i,beat_info.beat_detected);

		if( lvl_audio>=lvl_i || VU_Meter_Static){
			SetStream(VU_colors[0],VU_colors[1],VU_colors[2],i+num_leds_rgb_bot);
			leds_on++;
		} else{
			SetStream(0, 0, 0,i+num_leds_rgb_bot);
		}
		lvl_i+=incremento;
	}

	return leds_on;
}


void SetLeds_by_max(byte max){	
	//Turn on leds of VU_Metter by max led [Turn on all leds under max]

	for(byte i=0;i<=max;i++){
		colors_instance.ColorVU(VU_colors,mode_colors,i,0);
		SetStream(VU_colors[0],VU_colors[1],VU_colors[2],i+num_leds_rgb_bot);
	}
	
}

void SetRgb_Max(int max_value){		
	//Set A Rgb color by random number (0 to max_value)

	led1_color[0] = random(max_value);
	led1_color[1] = random(max_value);
	led1_color[2] = random(max_value);

	SetRgb(led1_color[0],led1_color[1],led1_color[2]);
}

void SetRgb_by_lvl(int lvl_audio){	
	//In function of max absolute calculate de intesity with the lvl of audio

	if ( Rgb_Low_Values(MinToChange) ){
		float max_value = (float) lvl_audio * ( ((float) vmax_abs)/255); 		if (max_value < 50.0) max_value = 50.0;
		SetRgb_Max ((int) max_value);
	}
}



					/****************************************/
					/*			EFFECTS - StreamWS2811		*/
					/****************************************/



void Effect_Led_MaxToLvl(int leds_on, long time_now){ 
	//Turn on one led [Color:White] that show the max of volume .
	
	if (max_led<=leds_on){
		max_led = leds_on;
		time_max_led = time_now;
		timer_max_led = timer_max_led_start;
	}

	if( (time_now <= (time_max_led+timer_max_led))){ 
		//With timer of max_led, lower the max LED to actual volume
		if (max_led <= 0) return;{
			if(MaxLed_Degree_Vu) SetLeds_by_max(max_led);
			//SetStream(255,255,255,max_led+num_leds_rgb_bot);
		}
	} else {
		if (max_led <= 0) return;
		time_max_led=time_now;
		timer_max_led=timer_max_led_cont;
		//SetStream(255,255,255,max_led+num_leds_rgb_bot);
		if(MaxLed_Degree_Vu) SetLeds_by_max(max_led);
		if (max_led > 0) max_led--;
	}
}

void Activate_EfDegreat(long time_now){ 
	//Delay to can attenuate RGB leds

	if (already_activate_degrade == 0) {
		if (time_now > (time_degree_delay + timer_degree_delay) ){
			degree_on =  TRUE;
			already_activate_degrade = TRUE;
		}
	}
}

void Effect_Rgb_Move(long now){		
	//Move color of RGB by leds of VU_Metter

	if(now > (time_beat_move_array+timer_beat_move_array)) {
		for(int i = (num_leds_side / 2) -1 ; i > 0 ;i--){
			buffer_array_color[i][0] =  buffer_array_color[i-1][0];
			buffer_array_color[i][1] =  buffer_array_color[i-1][1];
			buffer_array_color[i][2] =  buffer_array_color[i-1][2];
		}

		buffer_array_color[0][0] = last_rgb_color[0];
		buffer_array_color[1][0] = last_rgb_color[1];
		buffer_array_color[2][0] = last_rgb_color[2];

		last_rgb_color[0] = 0;
		last_rgb_color[1] = 0;
		last_rgb_color[2] = 0;

		time_beat_move_array = now;
	}

	for(int i = 0; i < (num_leds_side / 2) ;i++){

		//Below (por debajo)
		SetStream(buffer_array_color[i][0], buffer_array_color[i][1], buffer_array_color[i][2],i+num_leds_rgb_bot);
		SetStream(buffer_array_color[i][0], buffer_array_color[i][1], buffer_array_color[i][2],NUM_LEDS-1-i);

		//Over (por arriba)
		SetStream(buffer_array_color[i][0], buffer_array_color[i][1], buffer_array_color[i][2],num_leds_rgb_bot+num_leds_side-1-i);
		SetStream(buffer_array_color[i][0], buffer_array_color[i][1], buffer_array_color[i][2],NUM_LEDS-num_leds_side+i);
	}
}

void Clear_RGB_Move(){ //Clear the array of RGB MOVE
	for(int i=0; i < 9; i++){
		for(int k; k < 3;k++)
			buffer_array_color[i][k] = 0;
	}
}

void RGB_Move_Msg(int colour){
	Clear_RGB_Move(); 

	switch(colour){
		case 0: buffer_array_color[0][0] = 255; buffer_array_color[1][0] = 155; buffer_array_color[2][0] = 55; break;
		case 1: buffer_array_color[0][1] = 255; buffer_array_color[1][1] = 155; buffer_array_color[2][1] = 55; break;
		case 2: buffer_array_color[0][2] = 255; buffer_array_color[0][2] = 155; buffer_array_color[0][2]; break;
		
		default: 
			for(int i=0; i < 3; i++){
				for(int k; k < 3;k++)
					buffer_array_color[i][k] = 255 - (i*100);
			}
		break;
	}

	for(int i=0; i < 12; i++){
		Effect_Rgb_Move(i*100000);
		delay(50);
		SendStream();
	}
	
	time_beat_move_array = millis();

	ResetStream();
	SendStream();
}

void Effect_fire(){

}



					/****************************************/
					/*					SERIAL				*/
					/****************************************/


void serial_server(unsigned long now){
	char arg;

	byte lvl = 0;
	byte leds_on = 0;
	incremento = 7;

	if ( (arg_i<SERIAL_ARGS) && ( (arg = Serial.read()) >= 0) ) {

		if(arg_i == 0){
			ResetStream();
			lvl = arg;
			leds_on = SetLeds(lvl,0) - 1;
			SetRgb_Top(led1_color[0],led1_color[1],led1_color[2]);
			Effect_Led_MaxToLvl(leds_on,now);
			Led1_fade(now);
			SendStream();			
		} else if (arg_i> 0 && arg_i < 4) {
			led1_color[arg_i-1] = arg*2;
			led2_color[arg_i-1] = arg*2;
		} else {
			led2_color[arg_i-4] = arg*2;
		}

		if(arg_i == SERIAL_ARGS - 1){
			Leds_set();
			time_attenuate = now + timer_attenuate_first;
			arg_i = -1;
		}

		lastAckTime = now;
		arg_i++;

	}else if( ((now - lastAckTime) > 30) || (arg_i >= SERIAL_ARGS) ){
		Serial.print('B');
		lastAckTime = now;
		arg_i = 0;
	}
}

					/****************************************/
					/*					WIFI				*/
					/****************************************/



bool wifi_check(){

	if(Serial1.available() > 0) return true;

	Serial1.println("AT");
	if(Serial1.readString().indexOf("OK") != -1){

		//AT+CWMODE=3
		//AT+CWJAP="| Yates' Lovers |","camarerasycervezas"
		//AT+CIFSR
		Serial1.println("AT+CIPMUX=1");
		delay(50);
		Serial1.println("AT+CIPSERVER=1,80");
		delay(50);

		return true;
	}

	return false;
}

String wifi_server(){
	String input = "";

	char c_input;
	while (Serial1.available()) {
		c_input= Serial1.read();
		input += c_input;
	}

	if(input != ""){
		int start,end;
		wifi_buffer.concat(input);

		if(wifi_buffer.length() > 12){
			if(DEBUG_WIFI){ Serial.println("----BUFFER----"); Serial.println(wifi_buffer); Serial.println("----ORDER----");}
			
			if( (end = wifi_buffer.lastIndexOf(";]")) != -1 && (start = wifi_buffer.indexOf("[", end - WIFI_ARGS*4-4)) != -1){
				String order = wifi_buffer.substring(start+1,end+1); 
				wifi_buffer = wifi_buffer.substring(wifi_buffer.length()-WIFI_ARGS*4-4, wifi_buffer.length()-1);
				wifi_process(order);
				return order;
			}else if(wifi_buffer.length() > 256) {
				wifi_buffer = wifi_buffer.substring(wifi_buffer.length()-WIFI_ARGS*4-4, wifi_buffer.length()-1);
			}
		}
	}

	return "";
}

void wifi_process(String order){
	int wargs[WIFI_ARGS];

	int i = 0, start = 0, end = 0;

	while( (end = order.indexOf(";", start)) != -1){
		if(DEBUG_WIFI) Serial.println(order.substring(start,end));
		wargs[i++] = order.substring(start,end).toInt();
		start = end + 1;
	}


	unsigned long now = millis();

	if(wargs[0] >=0 && wargs[0] < 256){		//First Arg 0 -> Set Normal Color [R.G.B]
		ResetStream();

		incremento = 14;
		int leds = SetLeds(wargs[0],5) - 1;
		//Effect_Led_MaxToLvl(leds,now);

		Leds_SetColor(wargs[1], wargs[2], wargs[3]);
		SetRgb_Top(led1_color[0],led1_color[1],led1_color[2]);
		SetRgb_Bot(led1_color[0],led1_color[1],led1_color[2]);

		Leds_set();
		SendStream();
	}else if(wargs[0] == -1){			//First Arg -1 -> Command -> Change Mode
		if(wargs[1] == 0){
			mode = 0;
			smode_selected =  wargs[2];
			if(led1_color[0] == 0 && led1_color[1] == 0 && led1_color[2] == 0){
				smode->setColor(random(0,255),random(0,255),random(0,255));
			}else{
				smode->setColor(led1_color[0],led1_color[1],led1_color[2]);
			}
			smode->setTimer(50);
			if (wargs[3] > 0) smode->setTimer(wargs[3]);
			//ssmode->setTimer(1);
		}

		Leds_set_byColor(smode->setStrip(smode_selected, now));
	}
}



/********************************************************************************************************/
/********************************************************************************************************/

/* 												PROGRAM													*/

/********************************************************************************************************/
/********************************************************************************************************/



void setup () {

	Serial.begin(115200);
	Serial1.begin(115200);
	analogReference(DEFAULT);



	//Pins of Strep 5050 
	pinMode(PIN_LED_R, OUTPUT);
	pinMode(PIN_LED_G, OUTPUT);
	pinMode(PIN_LED_B, OUTPUT);

	pinMode(PIN_LED2_R, OUTPUT);
	pinMode(PIN_LED2_G, OUTPUT);
	pinMode(PIN_LED2_B, OUTPUT);

	pinMode(13, OUTPUT);


	//Pin of Strip WS2811
	pinMode(PIN_LED,OUTPUT);

	//LedStream
	FastLED.addLeds<WS2811, PIN_LED, RGB>(strip, NUM_LEDS);
	//FastLED.addLeds<NEOPIXEL, PIN_LED>(leds, NUM_LEDS);

	//Init stipmodes
	smode = new strip_modes(10);

	//Starting wifiServer
	int time_c = 100;

	/*
	Leds_SetColor(100,0,0);Leds_set();delay(time_c);
	Leds_SetColor(0,100,0);Leds_set();delay(time_c);
	Leds_SetColor(0,0,100);Leds_set();delay(time_c);
	*/

	if(wifi_check()){
		Leds_SetColor(0,50,0);Leds_set();
		if(DEBUG_WIFI) Serial.println("#W: Server Up!");
	}else{
		Leds_SetColor(50,0,0);Leds_set();
	}

	//Start indicator
	RGB_Move_Msg(1);
	Led1_SetColor(0,0,0);Led2_SetColor(0,0,0);Leds_set();

	beat_info.state_now = 0;

	//WatchDog
	wdt_disable();
	wdt_enable(WDTO_500MS);
}


void loop() {
	unsigned long now = millis();

	if (Serial1.available()){				//Check if wifi-data are waiting in SerialPort1 [ESP8266]
		mode = 2;
	}else{
		if ( (now - lastAckTime) > 10000){	//Send a keep-alive package by SerialPort1[PC]
			Serial.print('B');
			lastAckTime = now;
		}
		if (Serial.available()) mode = 1; 	//Check if pc-data are waiting in SerialPort [PC]
	}

	if(mode == -1){					


	}else if(mode == 0){			//Automate mode -> Different Effects
		Leds_set_byColor(smode->setStrip(smode_selected, now));
	}else if(mode == 1){			//Info from PC -> Serie
		serial_server(now);
	}else if(mode==2){				//Info from Wifi
		wifi_server();
	}

	wdt_reset();
}
