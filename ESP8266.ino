/*
  Reads MSGEQ7 IC with 7 different frequencies from range 0-255
  63Hz, 160Hz, 400Hz, 1kHz, 2.5kHz, 6.25KHz, 16kHz
*/
#define FASTLED_ESP8266_RAW_PIN_ORDER
// FastLED

#include "FastLED.h"

#define Version "1.1.0ESP"

#define LED_PINS    D1 // DATA_PIN
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B 
#define NUM_LEDS    80 // LED count
#define BRIGHTNESS_High  50  // reduce power consumption 1-255
#define BRIGHTNESS_Low  0
#define LED_DITHER  255  // try 0 to disable flickering
#define CORRECTION  TypicalLEDStrip
#define FRAMES_PER_SECOND 120
#define COOLING  55
#define SPARKING 120

CRGB leds[NUM_LEDS]; // Define the array of leds

// MSGEQ7
#include "MSGEQ7.h"

#define pinAnalogLeft A0
#define pinReset D2
#define pinStrobe D3
#define MSGEQ7_INTERVAL ReadsPerSecond(60)
#define MSGEQ7_SMOOTH true

int interval=50; // the time we need to wait for fade (fade slower at 500, Fade fast at 10, 50 is default).
int MusicInterval=2; // the time we need to wait Music (2 seems good) 5 is the limit.
int GenInterval=1000; // Gen millis() Global fade/reset.
unsigned long previousMillis=0; // millis() returns an unsigned long.
unsigned long previousMusicMillis=0; // music millis() returns an unsigned long.
unsigned long previousGenMillis=0; // Gen millis() returns an unsigned long.
unsigned long previousGen2Millis=0; // Gen millis() returns an unsigned long.

uint8_t bHue = -50;
uint8_t lHue = 150;
uint8_t mHue = 100;
uint8_t hHue = -10;

int length = NUM_LEDS / 4; //7 for all bands 4 for four

int spectrumValue[7];
int filter=60;
int oldB;

int buttonPin = D6;    // momentary push button on pin 0
int oldButtonVal = 0;
int nPatterns = 4;
int lightPattern = 1;

CMSGEQ7<MSGEQ7_SMOOTH, pinReset, pinStrobe, pinAnalogLeft> MSGEQ7;

CRGBPalette16 gPal;
bool gReverseDirection = false;

void setup() {
  Serial.begin(115200); //serial for debug
  delay(3000);
  pinMode(pinAnalogLeft, INPUT);
  pinMode(pinStrobe, OUTPUT);
  pinMode(pinReset, OUTPUT);
  Serial.println("Startup");
  Serial.println(Version);
  // FastLED setup
  FastLED.addLeds<CHIPSET, LED_PINS, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(CORRECTION);
  FastLED.setBrightness(BRIGHTNESS_High);
  FastLED.setDither(LED_DITHER);
  FastLED.show(); // needed to reset leds to zero
  
  pinMode(buttonPin, INPUT);
   digitalWrite(buttonPin, HIGH);  // button pin is HIGH, so it drops to 0 if pressed

   gPal = CloudColors_p;

  // This will set the IC ready for reading
  MSGEQ7.begin();
}

void loop() {

unsigned long currentMillis = millis();

  digitalWrite(pinReset, HIGH);
  digitalWrite(pinReset, LOW);
  // read that state of the pushbutton value;
  int buttonVal = digitalRead(buttonPin);
  MSGEQ7.reset();
  ESP.wdtFeed();
  if ((unsigned long)(currentMillis - previousGenMillis) >= GenInterval) {
  //LEDS.clear();
    if (buttonVal == LOW && oldButtonVal == HIGH) {// button has just been pressed
      lightPattern = lightPattern + 1;
      //delay(30);
      //Serial.print(currentMillis - previousGenMillis); //debug
      previousGenMillis = currentMillis;
    }
  //Serial.println(millis());
  }
  
  if (lightPattern > nPatterns) lightPattern = 1;
  oldButtonVal = buttonVal;
  
  switch(lightPattern) {
    case 1:
     Music(); 
        break;
    case 2:
     Fire();  
        break;
    case 3:
      Lights();
        break;
    case 4:
      LightsOUT();
        break;
  }
  //Serial.println(currentMillis - previousGen2Millis);
  if ((unsigned long)(currentMillis - previousGen2Millis) >= GenInterval) {
    fadeall();
    //Serial.println("40"/*currentMillis - previousGen2Millis*/); //debug
  previousGen2Millis = currentMillis;
  }
}

void Lights() {
  for (int i = 0; i<NUM_LEDS; i++) {
  leds[i] = CHSV(0, 17, 150);
  //leds[i] = CHSV(0, 17, BRIGHTNESS);
  }
  //FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
    //Serial.println("100");
}

void LightsOUT() {
  for (int i = 0; i<NUM_LEDS; i++) {
  leds[i] = CHSV( 0, 0, 0);
  }
  FastLED.show();
    //Serial.println("0");
}

void Fire() {
  //random16_add_entropy( random());
  FireWpalette();
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void FireWpalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[60];

  for( int i = 0; i < length/*NUM_LEDS*/; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / length/*NUM_LEDS*/) + 2));
    }
  
  for( int k= length/*NUM_LEDS*/ - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
  if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

  for( int j = 0; j < length/*NUM_LEDS*/; j++) {
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (length/*NUM_LEDS*/-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
    
    static byte heat2[60];

  for( int i = 0; i < length; i++) {
      heat2[i] = qsub8( heat2[i],  random8(0, ((COOLING * 10) / length/*NUM_LEDS*/) + 2));
    }
  
  for( int k= length - 1; k >= 2; k--) {
      heat2[k] = (heat2[k - 1] + heat2[k - 2] + heat2[k - 2] ) / 3;
    }
    
  if( random8() < SPARKING ) {
      int y = random8(7);
      heat2[y] = qadd8( heat2[y], random8(160,255) );
    }

  for( int j = 0; j < length; j++) {
      byte colorindex = scale8( heat2[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (length-1) - j+length;
      } else {
        pixelnumber = j+length;
      }
      leds[pixelnumber] = color;
    }
    
    static byte heat3[60];

  for( int i = 0; i < length; i++) {
      heat3[i] = qsub8( heat3[i],  random8(0, ((COOLING * 10) / length/*NUM_LEDS*/) + 2));
    }
  
  for( int k= length - 1; k >= 2; k--) {
      heat3[k] = (heat3[k - 1] + heat3[k - 2] + heat3[k - 2] ) / 3;
    }
    
  if( random8() < SPARKING ) {
      int y = random8(7);
      heat3[y] = qadd8( heat3[y], random8(160,255) );
    }

  for( int j = 0; j < length; j++) {
      byte colorindex = scale8( heat3[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (length-1) - j+(length*2);
      } else {
        pixelnumber = j+(length*2);
      }
      leds[pixelnumber] = color;
    }

    static byte heat4[60];

  for( int i = 0; i < length; i++) {
      heat4[i] = qsub8( heat4[i],  random8(0, ((COOLING * 10) / length/*NUM_LEDS*/) + 2));
    }
  
  for( int k= length - 1; k >= 2; k--) {
      heat4[k] = (heat4[k - 1] + heat4[k - 2] + heat4[k - 2] ) / 3;
    }
    
  if( random8() < SPARKING ) {
      int y = random8(7);
      heat4[y] = qadd8( heat4[y], random8(160,255) );
    }

  for( int j = 0; j < length; j++) {
      byte colorindex = scale8( heat4[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (length-1) - j+(length*3);
      } else {
        pixelnumber = j+(length*3);
      }
      leds[pixelnumber] = color;
    }
    //Serial.println("fire"); //debug
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(192); } } //Fade to black by 25%

void dimall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].fadeLightBy(64); } } //Fade color by 25%

void softReset(){ /*asm volatile ("  jmp 0");*/ }

void Music() {
  
//unsigned long currentMusicMillis = millis(); // grab current time
unsigned long currentMillis = millis(); // grab current time

for (int i=0;i<7;i++){
    digitalWrite(pinStrobe, LOW);
    //if ((unsigned long)(currentMillis - previousMillis) >= interval) {
    //delay(10);
    spectrumValue[i]=analogRead(pinAnalogLeft);
    //Serial.println(spectrumValue[i]);
    spectrumValue[i]=constrain(spectrumValue[i], 0, 1023);
    spectrumValue[i]=map(spectrumValue[i], 0,1023,0,254); //test point
    //Serial.println(spectrumValue[i]);
    digitalWrite(pinStrobe, HIGH);
}
  
if ((unsigned long)(currentMillis - previousMillis) >= interval) {
      fadeall();
      //dimall();
      //Serial.println(currentMillis - previousMillis); // debug
    previousMillis = currentMillis;
   }

if ((unsigned long)(currentMillis - previousMusicMillis) >= MusicInterval) {
Serial.println(currentMillis - previousMusicMillis);
  int bass = map((spectrumValue[0]+spectrumValue[1])/2/*/1.75*/, filter, 254, 0, length);
  //Serial.println(bass);
    bass=constrain(bass, 0, length);
  int low = map((/*spectrumValue[1]+*/spectrumValue[2]/*+spectrumValue[3]/3*/), filter, 254, 0, length);
    low=constrain(low, 0, length);
  int mid = map((spectrumValue[4]/*+spectrumValue[5]/2*/), filter, 254, 0, length);
    mid=constrain(mid, 0, length);
  int high = map(spectrumValue[6], filter, 254, 0, length);
    high=constrain(high, 0, length);
  //Serial.println(high);
    
    for(int led = 0; led < bass; led++) { 
        leds[led] = ColorFromPalette(LavaColors_p, bass+random(0,bHue), bass+random(BRIGHTNESS_Low,BRIGHTNESS_High), LINEARBLEND);
        //Serial.println(bass); //test point
    }
    for(int led = 0; led < low; led++) { 
        leds[-led+(length*2)-1] = ColorFromPalette(OceanColors_p, low+random(0,lHue), low+random(BRIGHTNESS_Low,BRIGHTNESS_High), LINEARBLEND);
    }
    for(int led = 0; led < mid; led++) { 
        leds[led+(length*2)] = ColorFromPalette(ForestColors_p, mid+random(0,mHue), mid+random(BRIGHTNESS_Low,BRIGHTNESS_High), LINEARBLEND);
        //Serial.println(mid); //test point
    }
    for(int led = 0; led < high; led++) { 
        leds[-led+(length*4)+1] = ColorFromPalette(CloudColors_p, high+random(0,hHue), high+random(BRIGHTNESS_Low,BRIGHTNESS_High), LINEARBLEND);
        //Serial.println(high);
    }
  }
    
   FastLED.show();
    //Serial.println(currentMillis - previousMusicMillis); //debug
   previousMusicMillis = currentMillis;
   //fadeall();
   //LEDS.clear();
  //}
  //fadeall();
}
