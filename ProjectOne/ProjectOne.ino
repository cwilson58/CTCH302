#include <Adafruit_Circuit_Playground.h>

#define SECONDS_IN_A_MINUTE 60
#define MICROSECONDS_IN_A_SECOND  1000
#define MICROSECONDS_IN_A_MINUTE  60000000

// temp is in BPM
unsigned int tempo = 60;
unsigned int delayValue = MICROSECONDS_IN_A_MINUTE / tempo;
unsigned short beat = 1;

void setup() {
  // put your setup code here, to run once:
  CircuitPlayground.begin();
}

void loop() {
  Serial.println(beat);
  // how am I going to ensure that the system does not drag the tempo over long time? probably a constant adjustment based on asm
  delayMicroseconds(delayValue);
  beat++;
  if(beat % 2 == 0){
    CircuitPlayground.redLED(1);
  }
  else{
    CircuitPlayground.redLED(0);
  }
  if(beat > 4){
    beat = 1;
  }
}
