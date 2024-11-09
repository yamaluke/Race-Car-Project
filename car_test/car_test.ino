#include <ECE3.h>

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
float Kp = 1/1000.0;
float Kd = 1/500.0;
int previous_fused_value = 0;

//== Prototypes ==//
void moveFoward(float weight, int speed);
int errorCalculator();


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
    Serial.begin(9600);
}


//== Loop ==//
void loop()
{

  int fused_values=errorCalculator();
  int baseSpd = 40;
  float derivative_error;
  derivative_error = (fused_values - previous_fused_value);
  
  float weight = fused_values*Kp; //+ derivative_error*Kd;
  
  if (weight > 0){
     weight += 1;
  }
  else{
    weight -= 1;
  }
  
  moveFoward(weight, baseSpd);
  previous_fused_value = fused_values;

  Serial.println(weight);
//  delay(50);
}


// moveFoward function: controls movment of the racecar 
// weight: 0 is straight, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
void moveFoward(float weight, int speed){
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
