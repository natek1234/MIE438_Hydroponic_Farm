
#define SensorPin A0 //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00 //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth 40 //times of collection
#define daytime 13 //hours of light
#define nightime 10 //hours of night
#define rise 30 //minutes of dusk and dawn
int pHArray[ArrayLenth]; //Store the average value of the sensor feedback
int pHArrayIndex=0;
unsigned long t = 0;
unsigned long goal_t = 0;
int state = 0;
int refill_state = 0;

void setup(void)
{
  pinMode(LED,OUTPUT);
  Serial.begin(9600);
  Serial.println("pH meter experiment!"); //Test the serial monitor
}

void loop(void)
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  t = t + samplingTime;
  if(millis()-samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++]=analogRead(SensorPin);
    if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
    voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
    pHValue = 3.5*voltage+Offset;
    samplingTime=millis();
  }
  if(pHValue > 6.5){
    state = 4;
  }
  if(millis() - printTime > printInterval) //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    Serial.print("Voltage:");
    Serial.print(voltage,2);
    Serial.print(" pH value: ");
    Serial.println(pHValue,2);
    digitalWrite(LED,digitalRead(LED)^1);
    printTime=millis();
  }

  // Main state machine //
  
  if(state == 0){ //Idle state lights on
      if(t > daytime*3600000-1000 && t < daytime*3600000+1000){ //check if all the daylight hours have passed with a 1000 millisecond margin
        state = 2;  
        t = 0;    
      }
      else{
        //keep lights on 
      }
  }
  else if(state == 1){ //Idle state lights off
      if(t > nightime*3600000-1000 && t < nightime*3600000+1000){ //check if all the nightime hours have passed with a 1000 millisecond margin
        state = 3;  
        t = 0;    
      }
      else{
        //keep lights off 
      }
  }
  else if(state == 2){ //30 min light transition to off
      if(t > rise*1800000-1000 && t < rise*1800000+1000){ //check if all the nightime hours have passed with a 1000 millisecond margin
        state = 1;  
        t = 0;    
      }
      else{
        //gradually turn lights off 
      }  
  }
  else if(state == 3) { //30 min light transition to on
      if(t > rise*1800000-1000 && t < rise*1800000+1000){ //check if all the nightime hours have passed with a 1000 millisecond margin
        state = 0;  
        t = 0;    
      }
      else{
        //gradually turn lights on
      }     
  }
  else if(state == 4) { //Feed state
    //feed plant
  }

  if(refill_state == 1){ //Keep refill LED on if refill needed
    //turn on LED
  }
}
 
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){ //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min; //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max; //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
