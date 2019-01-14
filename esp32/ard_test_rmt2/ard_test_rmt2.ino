#include <Arduino.h>

#define SERIAL_VERBOSE 1
#define BAUDRATE 3000000

#define NR_OF_PANELS 4
#define NR_OF_LEDS   256
#define NR_OF_BITS 24*NR_OF_LEDS

int incomingByte = 0;
int byteCounter = 0;
unsigned long lastPing = 0;

byte MODE = 0;
byte PANEL = 0;
byte LED = 0;
int COLOR[] = {0,0,0};

void setup()
{
  // disable BT
  btStop();

  // UPLINK
  Serial.begin(BAUDRATE);
  Serial.print("\n");

  // RMT
  rmt_init();

  // READY
  Serial.print("READY\n");
}

void loop()
{

  // Ping
  if ((millis() - lastPing) > 1000) {
    Serial.print("WAIT\n");
    lastPing = millis();
  }

  // Uplink RECV
  incomingByte = Serial.read();
  if (incomingByte < 0) return;
  lastPing = millis();
  byteCounter += 1;

  // Serial.println("RECV: "+String(incomingByte));

  // draw & reset (except if it is led select which could be 255 !)
  if (incomingByte == 255 && !(MODE == 1 && byteCounter == 3) ) {
    rmt_draw();
    reset();
    Serial.print("DRAW\n");
  }

  // just reset
  if (incomingByte == 254 && !(MODE == 1 && byteCounter == 3)) {
    reset();
    Serial.print("RESET\n");
  }

  // MODE sel
  else if (byteCounter == 1) setMODE(incomingByte);

  /*
    MODE 1-LED
  */
  else if (MODE == 1) {

    // PANEL sel
    if (byteCounter == 2) PANEL = incomingByte;

    // LED sel
    else if (byteCounter == 3) LED = incomingByte;

    // COLOR set
    else if (byteCounter == 4) COLOR[0] = incomingByte;
    else if (byteCounter == 5) COLOR[1] = incomingByte;
    else if (byteCounter == 6) {
      COLOR[2] = incomingByte;

      // set LED COLOR
      rmt_setLED(PANEL, LED, COLOR);

      // jump to next LED
      byteCounter = 3;
      nextLED();
    }

  }

  /*
    MODE 1-PANEL
  */
  else if (MODE == 2) {

    // PANEL sel
    if (byteCounter == 2) PANEL = incomingByte;

    // COLOR set
    else if (byteCounter == 3) COLOR[0] = incomingByte;
    else if (byteCounter == 4) COLOR[1] = incomingByte;
    else if (byteCounter == 5) {
      COLOR[2] = incomingByte;

      // set LED COLOR
      rmt_setLED(PANEL, LED, COLOR);

      // jump to next LED
      byteCounter = 2;
      nextLED();
    }

  }

  /*
    MODE ALL
  */
  else if (MODE == 3) {

    // COLOR set
    if (byteCounter == 2)       COLOR[0] = incomingByte;
    else if (byteCounter == 3)  COLOR[1] = incomingByte;
    else if (byteCounter == 4)  {
      COLOR[2] = incomingByte;

      // set LED COLOR
      rmt_setLED(PANEL, LED, COLOR);

      // jump to next LED
      byteCounter = 1;
      nextLED();
    }
  }

}


void reset() {
  byteCounter = 0;
  MODE = 0;
}

void setMODE(byte m) {
  MODE = m;
  PANEL = 0;
  LED = 0;
  COLOR[0] = 0;
  COLOR[1] = 0;
  COLOR[2] = 0;
  Serial.println("MODE "+String(m));
}

void nextLED() {
  LED += 1;
  if (LED == 0 || LED >= NR_OF_LEDS) {
    PANEL += 1;
    LED = 0;
  }
  if (PANEL >= NR_OF_PANELS) PANEL = 0;
}
