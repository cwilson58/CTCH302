#pragma once
#include <stdint.h>

#define SECONDS_IN_A_MINUTE 60
#define MICROSECONDS_IN_A_SECOND  1000
#define MICROSECONDS_IN_A_MINUTE  60000000
#define NEOPIXEL_PIN        PIN_A2
#define NEXT_TEMPO_BUTTON   PIN_A3
#define PREV_TEMPO_BUTTON   PIN_A6
#define PLAY_TEMPO_BUTTON   PIN_A5
#define NUMBER_OF_TEMPOS    6

extern unsigned int tempo;
extern Adafruit_CPlay_NeoPixel strip;

void timerSetup();
void metronomeSetup();
uint16_t convertTempoToTimerValue();