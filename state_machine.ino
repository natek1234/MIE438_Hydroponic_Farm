#include <Adafruit_NeoPixel.h>
#define LED_PIN 6
#define LED_COUNT 72
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);

#define FeedPin 7 //Pin to control feeding valve
#define RefillLED 8 //Refill LED 
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
unsigned long start_time = 0;
unsigned long end_time = 0;
unsigned long cooldown = 0;
unsigned long goal_t = 0;
int state = 0;
int refill_state = 0;
int prev_state;

void setup(void)
{
  //Start with strip on (daylight)
  strip.begin();
  uint32_t white = strip.Color(255, 255, 255);
  strip.fill(white, 0);
  strip.setBrightness(255);
  strip.show();

  //Set feed pina nd refill LED pin
  pinMode(FeedPin, OUTPUT);
  pinMode(RefillLED, OUTPUT);

  //pH sensor
  pinMode(LED,OUTPUT);
  Serial.begin(9600);
}

void loop(void)
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  float factor;
    
  t = t + samplingTime;
  //This collects pH samples
  if(millis()-samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++]=analogRead(SensorPin);
    if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
    voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
    pHValue = 3.5*voltage+Offset;
    samplingTime=millis();
  }

  //If pH value above limit and the cooldown of 30 minutes has occured, go into feed state
  if(pHValue > 6.5 and millis()>cooldown){
    if(state != 4){
      prev_state = state;
      start_time = t;
      end_time = t + 10000; //Set end time to 10 seconds after start
      cooldown = millis() + 1800000;           
    }
    state = 4;
  }

  // Main state machine //
  //Only run every 30 minutes
  if(t%1800000 < 1000 || t%1800000 > 1800000 - 1000){
    
    if(state == 0){ //Idle state lights on
        if(t > daytime*3600000-1000 && t < daytime*3600000+1000){ //check if all the daylight hours have passed with a 1000 millisecond margin
          state = 2;  
          t = 0;    
        }
        else{
          //Keep lights at max brightness
          strip.setBrightness(255);
          strip.show(); 
        }
    }
    else if(state == 1){ //Idle state lights off
        if(t > nightime*3600000-1000 && t < nightime*3600000+1000){ //check if all the nightime hours have passed with a 1000 millisecond margin
          state = 3;  
          t = 0;    
        }
        else{
          //Keep lights off
          strip.setBrightness(0);
          strip.show();
        }
    }
    else if(state == 2){ //30 min light transition to off
        if(t > rise*1800000-1000 && t < rise*1800000+1000){ //check if all the nightime hours have passed with a 1000 millisecond margin
          state = 1;  
          t = 0;    
        }
        else{
          //gradually turn lights off 
          factor = t/(rise*1800000); //find factor by which to multiply max brightness
          strip.setBrightness(255*(1-factor));
          strip.show();       
        }  
    }
    else if(state == 3) { //30 min light transition to on
        if(t > rise*1800000-1000 && t < rise*1800000+1000){ //check if all the nightime hours have passed with a 1000 millisecond margin
          state = 0;  
          t = 0;    
        }
        else{
          //gradually turn lights on
          factor = t/(rise*1800000); //find factor by which to multiply max brightness
          strip.setBrightness(255*factor);
          strip.show();
        }     
    }
    else if(state == 4 and millis()>cooldown) { //Feed state
      if(t > end_time){
        state = prev_state;
        digitalWrite(FeedPin, LOW);
      }
      else{
         //feed plant
        digitalWrite(FeedPin, HIGH);
      }
    }
  
    if(refill_state == 1){ //Keep refill LED on if refill needed
      //turn on LED
      digitalWrite(RefillLED, HIGH);
    }
    else{
      digitalWrite(RefillLED, LOW);
    }
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
