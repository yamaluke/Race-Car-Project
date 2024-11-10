// Testing to operate RaceCar

#include <ECE3.h>
#include <math.h>

const int left_nslp_pin=31; // nslp ==> awake & ready for PWM
const int left_dir_pin=29;
const int left_pwm_pin=40;
const int right_nslp_pin=11; // nslp ==> awake & ready for PWM
const int right_dir_pin=30;
const int right_pwm_pin=39;
const float Kp = 1/1000.0;
const float Kd = 1/500.0;
const int STOPVALUE = 000;
int STOPPOINTCOUNT = 0;

//== Prototypes ==//
void moveFoward(float weight, int speed);
void turn(float weight, int speed);


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
    moveFoward(0, baseSpd);
}


//===========================//
void loop(){
    int baseSpd = 50;
    moveFoward(0, baseSpd);

}


// motion functoin: controls movment of the racecar 
// weight: 0 is straight, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. The larger the weight, the tighter the turn.  
// the speed is how fast the car will move 
void moveFoward(float weight, int speed){
    // make both wheels go foward
    digitalWrite(right_dir_pin,LOW);
    digitalWrite(left_dir_pin,LOW);

    // set the speed of the wheels
    if(weight > 0){             // turn right
        analogWrite(left_pwm_pin, speed);
        analogWrite(right_pwm_pin, speed/weight);
    }else if(weight < 0){       // turn left
        weight *= -1;
        analogWrite(left_pwm_pin, speed/weight);
        analogWrite(right_pwm_pin, speed);
    }else{                      // straight
        analogWrite(left_pwm_pin, speed);
        analogWrite(right_pwm_pin, speed);
    }
}


// turn: used to make very tight turns
// weight: between -5 and 5, no zero, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
void turn(float weight, int speed){
    if(weight > 0){             // turn right 
        digitalWrite(right_dir_pin,HIGH);
        digitalWrite(left_dir_pin,LOW); 

        analogWrite(left_pwm_pin, speed);
        analogWrite(right_pwm_pin, speed * log(weight));
    }else if(weight < 0){       // turn left
        digitalWrite(right_dir_pin,LOW);
        digitalWrite(left_dir_pin,HIGH); 

        weight *= -1;

        analogWrite(left_pwm_pin, speed * log(weight));
        analogWrite(right_pwm_pin, speed);
    }
}


// motion: control center for car movement
// weight: 0 is straight, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
// note: Kp and Kd must be declared as global variables, also need to measure the black surface reading, and add that to global variable (STOPVALUE). STOPPOINTCOUNT also needs to be declared globally. 
void motion(int location, int derLocation, int speed){
    int turnRange = 1; // the point that the function decides to go from foward motion to turning motion

    //== check to see if car is at checkpoint ==//
    if(derLocation == 0 && location == STOPVALUE){
        if(STOPPOINTCOUNT == 0){
            //uturn function
            STOPPOINTCOUNT++;
        }else{
            moveFoward(0,0);
        }
    }else{
        //== Calculate weight ==//
        float weight = location*Kp + derLocation*Kd;
        if(weight >= -(turnRange+1) && weight <= (turnRange+1)){
            if (weight > 0){
                weight += 1;
            }else if(weight < 0){
                weight -= 1;
            }
            moveFoward(weight, speed);
        }else{
            if (weight > 0){
                weight -= turnRange;
            }
            else{
                weight += turnRange;
            }
            turn(weight, speed)
        }
    }    
}