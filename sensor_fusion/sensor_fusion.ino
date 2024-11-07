#include <ECE3.h>

uint16_t sensorValues[8];
int minimum[8] = {782, 805, 712, 758, 574, 666, 707, 828};
int maximum[8] = {1718, 1695, 1788, 1187, 1360, 1834, 1792, 1672};
int weights[8] = {-15, -14, -12, -8, 8, 12, 14, 15};

void setup()
{
  ECE3_Init();
  Serial.begin(9600); // set the data rate in bits per second for serial data transmission
  delay(2000);
}


void loop()
{

  errorCalculator();

  delay(50);
}


// Calculates the error using the sensor input data
// note must copy the minimum, maximum, and weights arrays for this function to work
int errorCalculator(){
  ECE3_read_IR(sensorValues);

  float currentValue[8];
  int fused_values = 0;

  for (int i = 0; i < 8; i++){
    // check if value is in the bounds for minimum 
    if(sensorValues[i] < minimum[i]){
      minimum[i] = sensorValues[i];
    }

    //subtract minimum
    currentValue[i] = (float)sensorValues[i] - minimum[i];

    // check to see if sensor value is in maximum bound 
    if(currentValue[i] > maximum[i]){
      maximum[i] = currentValue[i];
    }

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

  // Serial.print(fused_values);
  return fused_values;
}
