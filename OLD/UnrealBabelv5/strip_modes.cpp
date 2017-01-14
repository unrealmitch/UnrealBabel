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

void strip_modes::setTimer(int t){
	_timer = t;
}
void strip_modes::setColor(byte r, byte g, byte b){
	_color.r = r; _color.g = g; _color.b = b;
	_color_bk = _color;
}


Color strip_modes::setStrip(int mode, unsigned long time){

	if(time >= _timer + _time){
		
		int aux_timer = _timer;

		switch(mode){

			case 1: strip_modes::rgb(); 
			break;

			case 2:
				if(_sum_state){
					if(_rgb_value >= 255) _sum_state = false;  else _rgb_value+= 5;
				}
				else{
					if(_rgb_value <= 0) _sum_state = true; else _rgb_value-= 5;
				}

				if ( _color_bk.r < _rgb_value) _color.r = 0; else _color.r = _color_bk.r - _rgb_value;
				if ( _color_bk.g < _rgb_value) _color.g = 0; else _color.g = _color_bk.g - _rgb_value;
				if ( _color_bk.b < _rgb_value) _color.b = 0; else _color.b = _color_bk.b - _rgb_value;
			break;

			case 3:
				aux_timer*=5;
			case 4:
				if(time >= aux_timer + _time){
					if(_sum_state){
						_color_bk = _color;
						_color = _color_off;
						_sum_state = false;
					}
					else{
						_color = _color_bk;
						_sum_state = true;
					}
				}else return _color;
			break;

			default: 
			break;
		}

	_time = time;
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