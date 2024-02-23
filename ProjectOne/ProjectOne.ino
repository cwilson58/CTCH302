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
  playing = !playing;
}

void increaseTempoHandler(){
  updateTempo = 1;
  delay(100);
}

void decreaseTempoHandler(){
  updateTempo = -1;
  delay(100);
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
  Serial.println(beat);
  digitalWrite(PIN_A1, HIGH);
  if(beat%2 != 0){
    CircuitPlayground.redLED(1);
    strip.setPixelColor(0, 0, 50, 0);
    strip.show();
  }
  delay(200);
  CircuitPlayground.redLED(0);
  digitalWrite(PIN_A1, LOW);
  strip.setPixelColor(0, 50, 0, 0);
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
}
