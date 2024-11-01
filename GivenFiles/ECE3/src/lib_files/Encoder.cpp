#include <stdlib.h>

#include "Encoder.h"

volatile int32_t left_count = 0;
volatile int32_t right_count = 0;

int32_t getEncoderCount_left(){
	return left_count;
}

int32_t getEncoderCount_right(){
	return right_count;
}

void resetEncoderCount_left(){
	left_count = 0;
}

void resetEncoderCount_right(){
	right_count = 0;
}

void ISR_LEFT() {
	if (digitalRead(72)){
		left_count++;
	} else {
		left_count--;
	}
}
void ISR_RIGHT() {
	if (digitalRead(56)){
		right_count++;
	} else {
		right_count--;
	}
  
}
