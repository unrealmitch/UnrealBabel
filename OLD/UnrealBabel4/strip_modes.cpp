#include "strip_modes.h"

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

strip_modes::strip_modes(int timer){
	_timer = timer;
	_color = {0,0,0};
}

Color strip_modes::setStrip(int mode, unsigned long time){

	if(time >= _timer + _time){
		_time = time;
		switch(mode){
			case 0:
				if(_sum_state){
					if(++_color.r == 255) _sum_state = false;
				}
				else{
					if(--_color.r == 0) _sum_state = true;
				}
			break;

			case 1: strip_modes::rgb(); break;

			default: 
			break;
		}
	}

	return _color;
}

void strip_modes::rgb(){
	switch(_rgb_state){
		case 0:
			if(++_color.r == 255) _rgb_state++;
		break;

		case 1:
			if(++_color.g == 255) _rgb_state++;
		break;

		case 2:
			if(--_color.r == 0) _rgb_state++;
		break;

		case 3:
			if(++_color.b == 255) _rgb_state++;
		break;

		case 4:
			if(--_color.g == 0) _rgb_state++;
		break;

		case 5:
			if(++_color.r == 255) _rgb_state++;
		break;

		case 6:
			if(--_color.b == 0) _rgb_state = 1;
		break;
	}
}