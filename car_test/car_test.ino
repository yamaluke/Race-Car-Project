#include <ECE3.h>
#include<math.h>

//======================//
//== Global Variables ==//
//======================//
//== Variables for sensor fusion ==//
uint16_t sensorValues[8];
const int MINIMUM[8] = {782, 805, 712, 758, 574, 666, 707, 828};
const int MAXIMUM[8] = {1718, 1695, 1788, 1187, 1360, 1834, 1792, 1672};
const int WEIGHTS[8] = {15, 14, 12, 8, -8, -12, -14, -15};

//== Pin numbers ==//
const int left_nslp_pin=31; // nslp ==> awake & ready for PWM
const int left_dir_pin=29;
const int left_pwm_pin=40;
const int right_nslp_pin=11; // nslp ==> awake & ready for PWM
const int right_dir_pin=30;
const int right_pwm_pin=39;
const int LED_RF = 41;

//== weights and other variables used by functions ==//
int previous_fused_value = 0;       // holds the previous fused value, and will be used to calculate the change 
float Kp = 1.0/1000;                // proportional constant to calculate weight of turn
float Kd = 1.0/200;                 // derivative constant to calculate weight of turn
int stopPointCount = 0;             // number of times that the car has been on the black square 
int sensorState = 0;                // Used to determine if the car is on the black square; 0: on track, 1: off track, -1: on black


//================//
//== Prototypes ==//
//================//
int errorCalculator();
void moveFoward(float weight, int speed);
void turn(float weight, int speed);
void uturn();
void motion(int location, int derLocation, int speed);


//===========//
//== Setup ==//
//===========//
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


//==========//
//== Loop ==//
//==========//
void loop()
{
  int fused_values=errorCalculator();
  int baseSpd = 60;
  int derivative_error;
  derivative_error = (fused_values - previous_fused_value);
  
  motion(fused_values,derivative_error, baseSpd);
  
  previous_fused_value = fused_values;

  // Serial.println(weight);
  // delay(50);
}



//=====================//
//== Other Functions ==//
//=====================//

//== errorCalculator                                          ==//
// Input:   none                                                //             
// Output:  (int) returns fused value of sensors                //
// note:    Output value represents the error/dissplacment      //
//          of car relative to the line. Arrays MINIMUM,        //
//          MAXIMUM and WEIGHTS must exist in global scope      //
int errorCalculator(){
    sensorState = -1;               // reset sensor state
    ECE3_read_IR(sensorValues);     // read current values from sensor

    // local variables
    float currentValue[8];          // will hold values used to calculate the fused value
    int fusedValue = 0;

    // normalize and apply weights to the sensor values
    for (int i = 0; i < 8; i++){
        // used to check if car is on black square
        // if it isn't, then at least one of the sensors will read a value less then 2000
        if(currentValue[i] < 2000){
            sensorState = 0;
        }

        // remove minimum
        currentValue[i] = (float)sensorValues[i] - MINIMUM[i];

        //divide by maximum
        currentValue[i] /= MAXIMUM[i];

        //multiply by 1000
        currentValue[i] *= 1000;

        //multiply by weights
        currentValue[i] *= WEIGHTS[i];
    }

    //take average of all normalized values 
    for (int i = 0; i < 8; i++){
        fusedValue += currentValue[i];   
    }
    fusedValue /= 8;

    //Serial.println(fused_values);     // used for debugging
    return fusedValue;
}


//== moveFoward: moves foward, but can do so with slight turn ==//
// Input:   weight (float): |weight| must be greater or equal   //
//          to 1, or 0. 0 is straight, (-) is left,             //
//          (+) is right, and the magnitude changes how much it // 
//          turns. The larger the weight, the tighter the turn. //
//          speed (int): determines speed of car                //             
// Output:  (void) moves the car based on the weights           //
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


//== turn: used to make very tight turns                      ==//
// Input:   weight (float): -5.5 and 5.5, no zero. (-) is left, //
//          (+) is right, and the magnitude changes how much it // 
//          turns. The larger the weight, the tighter the turn. //
//          speed (int): determines speed of car                //             
// Output:  (void) moves the car based on the weights           //
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


//== uturn: 180 degree turn of car                            ==//
// Input:   none                                                //             
// Output:  (void) rotates car                                  //
void uturn(){
    int speed = 50; //==!! Design note: maybe increase speed, and cut delay !!==//
    turn(5.5,speed);
    digitalWrite(LED_RF,HIGH);  // to determine if uturn has been activated
    delay(1420);                // delay time to allow for full u-turn, can be adjusted based on battery level
}


// motion: control center for car movement
// weight: 0 is straight, (-) is left, (+) is right, and the magnitude is the weight for how much it turns. 
// the speed is how fast the car will move 
// note: Kp and Kd must be declared as global variables, also need to measure the black surface reading, and add that to global variable (STOPVALUE). STOPPOINTCOUNT also needs to be declared globally. 
void motion(int location, int derLocation, int speed){
    int turnRange = 0; // the point that the function decides to go from foward motion to turning motion

    //== check to see if car is at checkpoint ==//
    if(derLocation == 0 && sensorState == -1){
        if(stopPointCount == 0){
            uturn();
            moveFoward(0, speed);
            delay(500);
            stopPointCount++;
        }else{
            moveFoward(0,0);
        }
    }else{
        //== Calculate weight ==//
        float weight = location*Kp + derLocation*Kd;

        //== Determine if weight is positive, negative, or 0 ==//
        if(weight > 0){
            //== Determine type of turn to make when weight is positive ==//
            // if |weight| < turnRange+1, will use moveFoward function, and if not will use turn function
            if(weight >= -(turnRange+1) && weight <= (turnRange+1)){
                weight += 1;
                moveFoward(weight, speed);
            }else{
                weight -= turnRange;
                turn(weight, speed);
            }
        }else if (weight < 0){
            //== Determine type of turn to make when weight is negative ==//
            // if |weight| < turnRange+1, will use moveFoward function, and if not will use turn function
            if(-weight >= -(turnRange+1) && -weight <= (turnRange+1)){
                weight -= 1;
                moveFoward(weight, speed);
            }else{
                weight += turnRange;
                turn(weight, speed);
            }
        }else{
            //== when weight is 0, simply move foward ==//
            //==!! design note: could multiply speed because system is stable
            moveFoward(weight,speed);
        }
    }    
}