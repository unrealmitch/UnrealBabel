#include "module_colors.h"

static byte array1[] = {255,0,0,
						255,85,0,
						255,170,0,
						255,255,0,
						170,255,0,
						85,255,0,
						0,255,0,
						85,255,0,
						170,255,0,
						255,255,0,
						255,170,0,
						255,85,0};

static byte array2[] = {0,255,0,
						128,255,0,
						255,255,0,
						255,128,0,
						255,0,0,
						255,0,128,
						255,0,255,
						128,0,255,
						0,0,255,
						0,128,255,
						0,255,255,
						0,255,128};

static byte array3[] = {0,255,0,
						64,255,0,
						128,255,0,
						192,255,0,
						255,255,0,
						255,192,0,
						255,128,0,
						255,64,0,
						255,0,0,
						255,0,64,
						255,0,128,
						255,0,192,
						255,0,255,
						192,0,255,
						128,0,255,
						64,0,255,
						0,0,255,
						0,64,255,
						0,128,255,
						0,192,255,
						0,255,255,
						0,255,192,
						0,255,128,
						0,255,64};

module_colors::module_colors(int timer_arco){

	_timer_arco = timer_arco;
	arco_start = 0;
	arco_max = 23;
	counter_led = 0;

	sc_red = 255; sc_green = 255; sc_blue = 255;
}

//Array de movimiento con cambio constante automatico time_arco
void module_colors::DynamicColor(byte * VU_colors, byte nled, byte array[]){
	unsigned long now = millis();

	if (!nled){

		if( now > time_arco + _timer_arco){
			time_arco = millis();
			if(arco_start >= arco_max)
				arco_start = 0;
			else
				arco_start++;
		}
		counter_led=arco_start;
	} 

	VU_colors[0] = array[(counter_led) * 3];
	VU_colors[1] = array[(counter_led) * 3 + 1];
	VU_colors[2] = array[(counter_led) * 3 + 2];

	if (counter_led > 0)
		counter_led--;
	else
		counter_led=arco_max;
}

//Array de movimiento con cambio manual
void module_colors::ManualColor(byte * VU_colors, byte nled, byte array[]){
	unsigned long now = millis();

	if (!nled){
		counter_led=arco_start;
	} 

	VU_colors[0] = array[(counter_led) * 3];
	VU_colors[1] = array[(counter_led) * 3 + 1];
	VU_colors[2] = array[(counter_led) * 3 + 2];

	if (counter_led > 0)
		counter_led--;
	else
		counter_led=arco_max;
}


//Modos de color para leds VU_Meter [MSG: 0-> nothing, 1->beat bass detect]
extern void module_colors::ColorVU(byte * VU_colors, byte mode_led, byte  nled, byte msg){

	switch (mode_led){

		case 0:	//Progresive G to B
			switch (nled){
				case 0: case 1:
				VU_colors[0] = 0;
				VU_colors[1] = 255;
				VU_colors[2] = 0;
				break;

				case 2: case 3:  case 4: case 5:
				VU_colors[0] = (nled - 2) * 64;
				VU_colors[1] = 255;
				VU_colors[2] = 0;
				break;

				case 6:  case 7: case 8: case 9: 
				VU_colors[0] = 255;
				VU_colors[1] = 255 - ((nled - 6) * 64);
				VU_colors[2] = 0;
				break;

				case 10:  case 11: case 12: case 13:
				VU_colors[0] = 255;
				VU_colors[1] = 0;
				VU_colors[2] = (nled - 10) * 64;
				break;

				case 14:  case 15: case 16: case 17:
				VU_colors[0] = 255 - ((nled - 14) * 64);
				VU_colors[1] = 0;
				VU_colors[2] = 255;
				break;

				default:
				VU_colors[0] = 0;
				VU_colors[1] = 0;
				VU_colors[2] = 255;
				break;
			}
		break;



		case 1:	//Semi Static G-Y-R-V-B

			switch (nled){
				case 0: case 1: case 2: case 3:
				VU_colors[0] = 0;
				VU_colors[1] = 255;
				VU_colors[2] = 0;
				break;

				case 4: case 5:  case 6: case 7: case 8:
				VU_colors[0] = (nled - 4) * 50;
				VU_colors[1] = 255;
				VU_colors[2] = 0;
				break;

				case 9:  case 10: case 11: case 12: case 13:
				VU_colors[0] = 255;
				VU_colors[1] = 255 - ((nled - 4) * 50);
				VU_colors[2] = 0;
				break;

				case 14:  case 15: case 16:
				VU_colors[0] = 255;
				VU_colors[1] = 0;
				VU_colors[2] = (nled - 8) * 85;
				break;

				case 17:  case 18: case 19:
				VU_colors[0] = 255 - ((nled - 5) * 85);
				VU_colors[1] = 0;
				VU_colors[2] = 255;
				break;

				default:
				VU_colors[0] = 0;
				VU_colors[1] = 0;
				VU_colors[2] = 255;
				break;
			}
		break;


		case 2: //Constant V-B-G-Y-R-W
			switch (nled){
			case 0:
				VU_colors[0] = 200;
				VU_colors[1] = 30;
				VU_colors[2] = 200;
				break;

				case 1:
				VU_colors[0] = 0;
				VU_colors[1] = 0;
				VU_colors[2] = 255;
				break;

				case 2:
				VU_colors[0] = 0;
				VU_colors[1] = 255;
				VU_colors[2] = 255;
				break;

				case 3:
				VU_colors[0] = 0;
				VU_colors[1] = 255;
				VU_colors[2] = 0;
				break;

				case 4:
				VU_colors[0] = 255;
				VU_colors[1] = 255;
				VU_colors[2] = 0;
				break;

				case 5:
				VU_colors[0] = 255;
				VU_colors[1] = 0;
				VU_colors[2] = 0;
				break;

				case 6:
				VU_colors[0] = 255;
				VU_colors[1] = 0;
				VU_colors[2] = 255;
				break;


				default:
				VU_colors[0] = 255;
				VU_colors[1] = 255;
				VU_colors[2] = 255;
				break;
			}
		break;

		case 3: //Rainbow R-G
		DynamicColor(VU_colors,nled,array1);
		break;

		case 4: //Rainbow R-G-B (GAY!!!!)
		DynamicColor(VU_colors,nled,array3);
		break;

		case 5:
			if(msg){
				sc_red = random(0, 255);
				sc_green = random(0, 255);
				sc_blue = random(0, 255);
			} 
			/*
			VU_colors[0] = sc_red - ((sc_red/17) * nled);
			VU_colors[1] = sc_green - ((sc_green/17) * nled);
			VU_colors[2] = sc_blue - ((sc_blue/17) * nled);
			*/

			VU_colors[0] = ((sc_red/16) * nled);
			VU_colors[1] = ((sc_green/16) * nled);
			VU_colors[2] = ((sc_blue/16) * nled);
		break;
	}
}
