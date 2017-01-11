
/*
 	Proyect:	UnrealBabel Sound
 	Version:	2.2 [based in "uLed Music v2.3]
 	Info:		Arduino proyect for analyce audio and do differente effect 
  				with a led strip. Version controlled by PC [processing].

 	Date:		21-10-2015
 	Author: 	UnrealMitch
 	Mail:		unrealmitch@gmail.com
 	Web:		https://github.com/unrealmitch
 	License:	GNU GPL (http://www.gnu.org/copyleft/gpl.html)

 	Changelog:
 				2.1:Added more than 1 argument for serial comunication.
 					Added one strip [5050 - same color - RGB]
 					Added support for 5050 [bass]
 				2.2:Added support wifi communication
*/

#include "WS2801.h"
#include "module_colors.h"
#include <FastLED.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "Wifi.h"

//AUX
#define TRUE 1
#define FALSE 0

//Entrada
#define PIN_AUDIO_L 3 					//Pin In of Audio Input [R]
#define PIN_AUDIO_R 0 					//Pin In of Audio Input [L]
#define PIN_IRR 5				        //Pin In of IR reciver

//Salida
#define PIN_LED 9 						//Pin Out of Leds Data [D9]
#define PIN_LED_R 6
#define PIN_LED_G 5
#define PIN_LED_B 3

//Serie Comunication
#define SERIAL_ARGS 2

//Control
#define NUM_LEDS 50

//Debug
#define DEBUG 0
#define DEBUG_VU 0

//Leds Distribution
#define num_leds_rgb 8					//Num Leds BEAT [RGB Top]		
#define num_leds_rgb_bot 3				//Num Leds BEAT 2 [RGB Bot / side]
#define num_leds_side 18 				//Num Leds VU Metter per side: ((NUM_LEDS - num_leds_rgb - num_leds_rgb_bot * 2) / 2)	



/********************************************************************************************************/
/********************************************************************************************************/

/* 												Variables												*/

/********************************************************************************************************/
/********************************************************************************************************/



//Instances clase
module_colors colors_instance(125);

//Led
CRGB leds[NUM_LEDS];
byte VU_colors[3] = {0,0,0};	//Temp array for set color of one led

//Colors of RGB
byte color_beat[3] = {0,0,0};

//Serial Communication
byte arg_i = 0;

//Wifi Communication
Wifi wserver(4,2);

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
	int mode = 1;


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
					/*			Stream5050 & RGBColor		*/
					/****************************************/


void setColor5050(){
	analogWrite(PIN_LED_R, color_beat[0]);
	analogWrite(PIN_LED_G, color_beat[1]);
	analogWrite(PIN_LED_B, color_beat[2]);
}

void writeColor5050(byte red, byte green, byte blue){
	color_beat[0] = red;
	color_beat[1] = green;
	color_beat[2] = blue;
}

void writeColor5050_rnd(){	
	//Set RGB leds with random color

	int max_value = 255;
	last_rgb_color[0] = random(max_value);
	last_rgb_color[1] = random(max_value);
	last_rgb_color[2] = random(max_value);

	color_beat[0] = last_rgb_color[0];
	color_beat[1] = last_rgb_color[1];
	color_beat[2] = last_rgb_color[2];

	writeColor5050(color_beat[0],color_beat[1],color_beat[2]);
}

void attenuate_beat(unsigned long time){
	byte max = color_beat[0];
	if(color_beat[1]>max) max = color_beat[1]>max;
	if(color_beat[2]>max) max = color_beat[2]>max;

	byte subs = attenuate_subtract;
	//byte subs = byte (((int) attenuate_subtract * (int) color_beat[i]) / (int) max); //map(attenuate_subtract,0,max,0,color_beat[i]);
	if(time_attenuate <= time){
		for(int i = 0; i < 3;i++){
			
			if(color_beat[i] >= subs)
				color_beat[i]-= subs;
			else
				color_beat[i] = 0;
		}
		setColor5050();
		time_attenuate = time + timer_attenuate;
	}
}


					/****************************************/
					/*				StreamWS2081			*/
					/****************************************/



//Stream LEDs
void Duplicate(){ //Mirror the left side of the LED strip on the right
	int i; int j;
	for (i=NUM_LEDS-1; i > (NUM_LEDS-1-num_leds_side-num_leds_rgb_bot); i--){
		j=NUM_LEDS-1-i;
		SetStream(leds[j].g,leds[j].r,leds[j].b,i);
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

	leds[pos].r = g;
	leds[pos].g = r;
	leds[pos].b = b;
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
	if(color_beat[0] > min) return false;
	if(color_beat[1] > min) return false;
	if(color_beat[2] > min) return false;

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

	color_beat[0] = random(max_value);
	color_beat[1] = random(max_value);
	color_beat[2] = random(max_value);

	SetRgb(color_beat[0],color_beat[1],color_beat[2]);
}

void SetRgb_by_lvl(int lvl_audio){	
	//In function of max absolute calculate de intesity with the lvl of audio

	if ( Rgb_Low_Values(MinToChange) ){
		float max_value = (float) lvl_audio * ( ((float) vmax_abs)/255); 		if (max_value < 50.0) max_value = 50.0;
		SetRgb_Max ((int) max_value);
	}
}



					/****************************************/
					/*			EFFECTS - StreamWS2081		*/
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

/********************************************************************************************************/
/********************************************************************************************************/

/* 												PROGRAM													*/

/********************************************************************************************************/
/********************************************************************************************************/



void setup () {

	Serial.begin(115200);
	analogReference(DEFAULT);

	//Pins of Strep 5050 
	pinMode(PIN_LED_R, OUTPUT);
	pinMode(PIN_LED_G, OUTPUT);
	pinMode(PIN_LED_B, OUTPUT);

	//Pins of Strep WS8011
	pinMode(PIN_LED,OUTPUT);

	//LedStream
	FastLED.addLeds<NEOPIXEL, PIN_LED>(leds, NUM_LEDS);

	//Starting wifiServer
	writeColor5050(255,255,255);setColor5050();
	wserver.start("| Yates' Lovers |", "camarerasycervezas", 80);
	

	//Start indicator
	writeColor5050(0,50,0);setColor5050();
	RGB_Move_Msg(1);
	writeColor5050(0,0,0);setColor5050();

	beat_info.state_now = 0;

}


void loop() {
	if(mode == 0){	//Info from PC -> Serie

		unsigned long
			lastAckTime,
			now;
		char arg;

		byte lvl = 0;
		byte leds_on = 0;
		incremento = 7;

		lastAckTime = millis();

		for(;;) {
			now = millis();
			
			if ( (arg_i<SERIAL_ARGS) && ( (arg = Serial.read()) >= 0) ) {

				if(arg_i == 0){
					ResetStream();
					lvl = arg;
					leds_on = SetLeds(lvl,0) - 1;
					SetRgb_Top(color_beat[0],color_beat[1],color_beat[2]);
					Effect_Led_MaxToLvl(leds_on,now);
					attenuate_beat(now);
					SendStream();			
				} else {
					if(arg > 0){
						writeColor5050_rnd();
						setColor5050();
						time_attenuate = now + timer_attenuate_first;
					}
				}

				lastAckTime = now;
				arg_i++;

			}else if( ((now - lastAckTime) > 50) || (arg_i >= SERIAL_ARGS) ){
				Serial.print('B');
				lastAckTime = now;
				arg_i = 0;
			}
		}



	}else if(mode==1){	//Info from PC -> Wifi

		byte leds_on = 0;
		incremento = 14;

		unsigned long now;

		int * wargs;
		int nargs;
		for(;;){
			now = millis();
			wargs = wserver.getQuest();
			nargs = sizeof(wargs)/sizeof(int);

			if(wargs != NULL){
				ResetStream();

				if(wargs[0]>-1){
					leds_on = SetLeds(wargs[0],0) - 1;
					//Effect_Led_MaxToLvl(leds_on,now);		
				}

				if(nargs>=3 || true){
					writeColor5050(wargs[1], wargs[2], wargs[3]);
					SetRgb_Top(color_beat[0],color_beat[1],color_beat[2]);
					SetRgb_Bot(color_beat[0],color_beat[1],color_beat[2]);
					setColor5050();
				}
			}
				SendStream();
				free(wargs);
		}
	}
}
