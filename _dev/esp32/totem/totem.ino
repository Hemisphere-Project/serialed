#include <Arduino.h>
#include <FastLED.h>

#define BAUDRATE 12000000

#define BYTE_PANEL_START 250
#define BYTE_PANEL_STOP 251
#define BYTE_START_FRAME 253
#define BYTE_STOP_FRAME 254
#define BYTE_MSG_END 255

#define NR_OF_PANELS 1
#define NR_OF_LEDS   256

#define LED_PIN  18
#define COLOR_ORDER GRB
#define CHIPSET     WS2811

#define MAX_BRIGHTNESS 2

// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

int PANEL = 0;
int COLOR[] = {0,0,0};

#define SIZE 4096
byte chunk[SIZE];
int inputCounter = 0;
unsigned long lastPing = 0;

int readSize = 0;
int readStart = 0;
int readStop = 0;

void setup()
{
  // UPLINK
  Serial.begin(BAUDRATE);
  Serial.print("\n");

  // FASTLED
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(255);

  // READY
  Serial.print("READY\n");
}

void loop()
{
  static uint8_t hue = 0;

  // Ping
  if ((millis() - lastPing) > 1000) {
    Serial.print("WAIT\n");
    lastPing = millis();
  }

  // Uplink RECV
  readSize = Serial.readBytesUntil(BYTE_MSG_END, chunk, SIZE);
  if (readSize == 0) return;
  lastPing = millis();

   Serial.print("GOT "+String(readSize)+" last: "+String(chunk[readSize-1])+"\n");
  Serial.print("GOT ");
  for (int k=0; k<(readSize); k+=1) {
    Serial.print(chunk[k]);
    Serial.print(" ");
  }
  Serial.print("\n");


  // DRAW
  if (chunk[0] == BYTE_START_FRAME && chunk[readSize-1] == BYTE_STOP_FRAME) {

    for (int k=0; (k*3+3)<(readSize-1); k+=1) {
      leds[k] = CRGB(chunk[k*3+1],chunk[k*3+2],chunk[k*3+3]);
    }

    FastLED.show();
    Serial.print("DRAW 1\n");
    inputCounter=0;
    PANEL = -1;
    hue++;
  }


}
