#include <ECE3.h>
#include<math.h>

uint16_t sensorValues[8];
int minimum[8] = {782, 805, 712, 758, 574, 666, 707, 828};
int maximum[8] = {1718, 1695, 1788, 1187, 1360, 1834, 1792, 1672};
int weights[8] = {15, 14, 12, 8, -8, -12, -14, -15};
const int left_nslp_pin=31; // nslp ==> awake & ready for PWM
const int left_dir_pin=29;
const int left_pwm_pin=40;
const int right_nslp_pin=11; // nslp ==> awake & ready for PWM
const int right_dir_pin=30;
const int right_pwm_pin=39;
float Kp = 1.0/1000;
float Kd = 1.0/500;
const int STOPVALUE = 000;
int stopPointCount = 0;
int previous_fused_value = 0;

//== Prototypes ==//
int errorCalculator();
void moveFoward(float weight, int speed);
void turn(float weight, int speed);
void uturn(int speed);
void motion(int location, int derLocation, int speed);


//== Setup ==//
void setup()
{
  ECE3_Init();
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


    int baseSpd =40;
    moveFoward(0, baseSpd);
    // Serial.begin(9600);
}


//== Loop ==//
void loop()
{
  int fused_values=errorCalculator();
  int baseSpd = 40;
  float derivative_error;
  derivative_error = (fused_values - previous_fused_value);
  
  motion(fused_values,derivative_error, baseSpd);
  
  previous_fused_value = fused_values;

  // Serial.println(weight);
  // delay(50);
}


// errorCalculator: Calculates the error using the sensor input data
// note must copy the minimum, maximum, and weights arrays for this function to work
int errorCalculator(){
  ECE3_read_IR(sensorValues);

  float currentValue[8];
  int fused_values = 0;

  for (int i = 0; i < 8; i++){
    // check if value is in the bounds for minimum 
//    if(sensorValues[i] < minimum[i]){
//      minimum[i] = sensorValues[i];
//    }

    //subtract minimum
    currentValue[i] = (float)sensorValues[i] - minimum[i];

    // check to see if sensor value is in maximum bound 
//    if(currentValue[i] > maximum[i]){
//      maximum[i] = currentValue[i];
//    }

    //divide by maximum
    currentValue[i] /= maximum[i];

    //multiply by 1000
    currentValue[i] *= 1000;

    //multiply by weights
    currentValue[i] *= weights[i];
  }

  //take average
  for (int i = 0; i < 8; i++){
    fused_values += currentValue[i];   
  }
  fused_values /= 8;

  //Serial.println(fused_values);
  return fused_values;
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
// weight: between -5.5 and 5.5, no zero, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
void turn(float weight, int speed){
    if(weight > 0){             // turn right 
        digitalWrite(right_dir_pin,HIGH);
        digitalWrite(left_dir_pin,LOW); 

        analogWrite(left_pwm_pin, speed);
        analogWrite(right_pwm_pin, speed * log(weight/2));
    }else if(weight < 0){       // turn left
        digitalWrite(right_dir_pin,LOW);
        digitalWrite(left_dir_pin,HIGH); 

        weight *= -1;

        analogWrite(left_pwm_pin, speed * log(weight/2));
        analogWrite(right_pwm_pin, speed);
    }
}


//uturn: make a 360 degree rotation at checkpoint
void uturn(int speed){
    int location;
    int preLocation;
    int derivativeValue = -1;

    turn(5.5,speed);
    delay(50);

    location = errorCalculator();
    preLocation = location;
    
    while(location != STOPVALUE || derivativeValue == 0){
        // calculate new sensor and derivative value 
        location = errorCalculator();
        derivativeValue = location - preLocation;
        preLocation = location;
    }
    moveFoward(0,0);
}


// motion: control center for car movement
// weight: 0 is straight, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
// note: Kp and Kd must be declared as global variables, also need to measure the black surface reading, and add that to global variable (STOPVALUE). STOPPOINTCOUNT also needs to be declared globally. 
void motion(int location, int derLocation, int speed){
    int turnRange = 1; // the point that the function decides to go from foward motion to turning motion

    //== check to see if car is at checkpoint ==//
    if(derLocation == 0 && location == STOPVALUE){
        if(stopPointCount == 0){
            uturn(speed);
            moveFoward(0, speed);
            delay(50);
            stopPointCount++;
        }else{
            moveFoward(0,0);
        }
    }else{
        //== Calculate weight ==//
        float weight = location*Kp + derLocation*Kd;

        //== Determine what kind of turn to make ==//
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