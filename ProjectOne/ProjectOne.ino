#include <Adafruit_Circuit_Playground.h>

#define SECONDS_IN_A_MINUTE 60
#define MICROSECONDS_IN_A_SECOND  1000
#define MICROSECONDS_IN_A_MINUTE  60000000
#define NEXT_TEMPO_BUTTON   PIN_A3
#define PREV_TEMPO_BUTTON   PIN_A4
#define PLAY_TEMPO_BUTTON   PIN_A5

//to go from tempo to HZ, divide by 60
// temp is in BPM
unsigned int tempo = 120; //base tempo is 60BPM, matches a clock
unsigned int delayValue = MICROSECONDS_IN_A_MINUTE / tempo;
unsigned volatile short beat = 1;
volatile bool update = true;
Adafruit_CPlay_NeoPixel strip = Adafruit_CPlay_NeoPixel(1, PIN_A2, NEO_GRB + NEO_KHZ800);

uint16_t convertTempoToTimerValue(){
  return (48000000 / ((tempo/60)*1024)) - 1;
}

// Code taken from this forum post: https://forum.arduino.cc/t/timer-interrupt-on-arduino-zero-board/409166/3
// zero is the same basic chip as the one on the CPE.
// info for this can also be found in the reference manual
void timerSetup(){
  // Set up the generic clock (GCLK4) used to clock timers
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |          // Divide the 48MHz clock source by divisor 1: 48MHz/1=48MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Feed GCLK4 to TC4 and TC5
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
 
  REG_TC4_COUNT16_CC0 =  convertTempoToTimerValue();                   // Set the TC4 CC0 register as the TOP value in match frequency mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 3); 
  NVIC_EnableIRQ(TC4_IRQn);

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENSET_OVF;             // Enable TC4 interrupts
  // REG_TC4_INTENCLR = TC_INTENCLR_OVF;          // Disable TC4 interrupts
 
  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV1024 |   // Set prescaler to 1024, 48MHz/1024 = 46.875kHz
                   TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC4 into match frequency (MFRQ) mode 
                   TC_CTRLA_ENABLE;               // Enable TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization
}

void TC4_Handler(){     
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)             
  {
        // Put your timer overflow (OVF) code here:     
        // ...
    strip.clear();
    beat++;
    if(beat > 4){
      beat = 1;
    }
    update = true;
    REG_TC4_INTFLAG = TC_INTFLAG_OVF;         // Clear the OVF interrupt flag
  }
}

void setup() {
  CircuitPlayground.begin();
  strip.begin();
  pinMode(PIN_A1,OUTPUT);
  pinMode(PLAY_TEMPO_BUTTON,INPUT_PULLDOWN);
  CircuitPlayground.clearPixels();
  strip.clear();
  timerSetup();
}

void loop() {
  if(update){
    Serial.println(beat);
    CircuitPlayground.redLED(1);
    digitalWrite(PIN_A1, HIGH);
    delay(200);
    CircuitPlayground.redLED(0);
    digitalWrite(PIN_A1, LOW);
    // if(beat % 2 == 0){
    //   CircuitPlayground.redLED(1);
    //   digitalWrite(PIN_A1, HIGH);
    //   for(int i=0; i<1; i++) {
    //     strip.setPixelColor(i, 0, 50, 0);
    //   }
    // }
    // else{
    //   CircuitPlayground.redLED(0);
    //   digitalWrite(PIN_A1, LOW);
    //   for(int i=0; i<1; i++) {
    //     strip.setPixelColor(i, 50, 0, 0);
    //   }
    // }
    update = false;
  }
}
