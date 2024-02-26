#include <Adafruit_Circuit_Playground.h>
#include "Metronome.h"

//to go from tempo to HZ, divide by 60
const int tempoList[] = {40,60,80,120,144,170};
int tempoIndex = 1; //base tempo is 60BPM, matches a clock
unsigned int tempo = tempoList[tempoIndex]; 
unsigned volatile short beat = 1;
volatile bool update = true;
volatile bool playing = true;
volatile short updateTempo = 0;
Adafruit_CPlay_NeoPixel strip = Adafruit_CPlay_NeoPixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
int idleRGB[] = {50,0,0};
// ISRs
void TC4_Handler(){     
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)             
  {
    beat++;
    if(beat > 4){
      beat = 1;
    }
    update = true;
    REG_TC4_INTFLAG = TC_INTFLAG_OVF;         // Clear the OVF interrupt flag
  }
}

void playHandler(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 200){
    playing = !playing;
  }
  last_interrupt_time = interrupt_time;
}

void increaseTempoHandler(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 200){
    updateTempo = 1;
  }
  last_interrupt_time = interrupt_time;
}

void decreaseTempoHandler(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 200){
    updateTempo = -1;
  }
  last_interrupt_time = interrupt_time;
}

// helper functions
void changeTempo(){
    tempoIndex += updateTempo;
    updateTempo = 0;
    if(tempoIndex < 0) {
      tempoIndex = 0;
    }
    else if(tempoIndex >= NUMBER_OF_TEMPOS){
      tempoIndex = NUMBER_OF_TEMPOS - 1;
    }
    tempo = tempoList[tempoIndex];
    if(tempo == 40){
      idleRGB[0] = 50;
      idleRGB[1] = 0;
      idleRGB[2] = 0;
    }
    else if(tempo == 60){
      idleRGB[0] = 0;
      idleRGB[1] = 0;
      idleRGB[2] = 50;
    }
    else if(tempo == 120){
      idleRGB[0] = 255;
      idleRGB[1] = 165;
      idleRGB[2] = 0;
    }
    else if(tempo == 144){
      idleRGB[0] = 155;
      idleRGB[1] = 155;
      idleRGB[2] = 53;
    }
    else{
      idleRGB[0] = 160;
      idleRGB[1] = 32;
      idleRGB[2] = 240;
    }
    Serial.println(tempo);
    // reconfigure timer
    noInterrupts();
    REG_TC4_COUNT16_CC0 =  convertTempoToTimerValue();                   // Set the TC4 CC0 register as the TOP value in match frequency mode
    while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization
    interrupts();
}

/*
* Show the best 
*/
void updateState(){
  digitalWrite(PIN_A1, HIGH);
  if(beat%2 != 0){
    CircuitPlayground.redLED(1);
    strip.setPixelColor(0, 0, 50, 0);
    strip.show();
  }
  delay(200);
  CircuitPlayground.redLED(0);
  digitalWrite(PIN_A1, LOW);
  strip.setPixelColor(0, idleRGB[0], idleRGB[1], idleRGB[2]);
  strip.show();
}

void setup() {
  metronomeSetup();
  timerSetup();
  attachInterrupt(digitalPinToInterrupt(PLAY_TEMPO_BUTTON), playHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(NEXT_TEMPO_BUTTON), increaseTempoHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(PREV_TEMPO_BUTTON), decreaseTempoHandler, RISING);
}

void loop() {
  if(updateTempo != 0){
    changeTempo();
  }
  if(playing && update){
    updateState();
    update = false;
  }
  else if(!playing){ //when not playing, solid green LED,
    strip.clear();
    CircuitPlayground.redLED(1);
    strip.setPixelColor(0, 0, 50, 0);
    strip.show();
  }
}
