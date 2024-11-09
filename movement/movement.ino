// Testing to operate RaceCar

#include <ECE3.h>

const int left_nslp_pin=31; // nslp ==> awake & ready for PWM
const int left_dir_pin=29;
const int left_pwm_pin=40;
const int right_nslp_pin=11; // nslp ==> awake & ready for PWM
const int right_dir_pin=30;
const int right_pwm_pin=39;

//== Prototypes ==//
void motion(float weight, int speed);


//== Initial setup ==//
void setup(){
    pinMode(left_nslp_pin,OUTPUT);
    pinMode(left_dir_pin,OUTPUT);
    pinMode(left_pwm_pin,OUTPUT);
    
    pinMode(right_nslp_pin,OUTPUT);
    pinMode(right_dir_pin,OUTPUT);
    pinMode(right_pwm_pin,OUTPUT);

    digitalWrite(left_dir_pin,LOW);
    digitalWrite(left_nslp_pin,HIGH);
    
    digitalWrite(right_dir_pin,LOW);
    digitalWrite(right_nslp_pin,HIGH);


    int baseSpd = 70;
    motion(0, baseSpd);
}


//===========================//
void loop(){
    int baseSpd = 50;
    motion(0, baseSpd);

}


// motion functoin: controls movment of the racecar 
// weight: 0 is straight, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
void motion(float weight, int speed){
    if(weight > 0){
        analogWrite(left_pwm_pin, speed);
        analogWrite(right_pwm_pin, speed/weight);
    }else if(weight < 0){
        weight *= -1;
        analogWrite(left_pwm_pin, speed/weight);
        analogWrite(right_pwm_pin, speed);
    }else{
        analogWrite(left_pwm_pin, speed);
        analogWrite(right_pwm_pin, speed);
    }
}