#include <Arduino.h>
#include <esp32-hal.h>

#define NR_OF_PANELS 5
#define NR_OF_LEDS   512
#define NR_OF_ALL_BITS 24*NR_OF_LEDS

#define SERIAL_LINK 0
#define SERIAL_VERBOSE 0

byte rmtPins[NR_OF_PANELS] = {17, 5, 18, 19, 21 };

rmt_data_t led_data[NR_OF_PANELS][NR_OF_ALL_BITS];
rmt_obj_t* rmt_send[NR_OF_PANELS];

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // disable BT
  btStop();

  // UPLINK
  Serial.begin(921600);
  Serial.println("");

  // DOWNLINK
  if (SERIAL_LINK)
    Serial1.begin(921600, SERIAL_8N1, 4, 2);

  // RMT init
  for (int PANEL = 0; PANEL < NR_OF_PANELS; PANEL++) {
    if ((rmt_send[PANEL] = rmtInit(rmtPins[PANEL], true, RMT_MEM_64)) == NULL) {
      Serial.println("init sender faiLED");
    }

    float realTick = rmtSetTick(rmt_send[PANEL], 100);
    Serial.printf("Channel %i - tick set to: %f ns\n", PANEL , realTick);
  }

  Serial.println("READY");
  digitalWrite(LED_BUILTIN, LOW);
}

int incomingByte = 0;
int byteCounter = 0;

bool panelDirty[NR_OF_PANELS];

byte MODE = 0;
byte PANEL = 0;
byte LED = 0;
int COLOR[] = {0,0,0};

unsigned long lastPing = 0;

void loop()
{

  // Ping
  if ((millis() - lastPing) > 1000) {
    lastPing = millis();
    Serial.println("PING");
  }

  // Downlink RECV: forward to UPLINK
  if (SERIAL_LINK)
    if(Serial1.available())
      Serial.write(Serial1.read());


  // Uplink RECV
  incomingByte = Serial.read();
  if (incomingByte < 0) return;
  byteCounter += 1;

  if (SERIAL_VERBOSE) Serial.println("RECV: "+String(incomingByte));

  // FWD to Downlink
  // if (SERIAL_LINK) Serial1.write(incomingByte);

  // draw & reset (except if it is led select which could be 255 !)
  if (incomingByte == 255 && !(MODE == 1 && byteCounter == 3) ) draw();
  if (incomingByte == 254) init();

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
      setLED(PANEL, LED, COLOR);

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
      setLED(PANEL, LED, COLOR);

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
      setLED(PANEL, LED, COLOR);

      // jump to next LED
      byteCounter = 1;
      nextLED();
    }
  }

}

void setLED(int panel, int led, int colo[3]) {

  int i = 3*8*led;
  for (int c = 0; c < 3; c++ ) {
    for (int bit = 0; bit < 8; bit++) {
      if ( colo[c] & (1 << (8 - bit)) ) {
        led_data[panel][i].level0 = 1;
        led_data[panel][i].duration0 = 8;
        led_data[panel][i].level1 = 0;
        led_data[panel][i].duration1 = 4;
      } else {
        led_data[panel][i].level0 = 1;
        led_data[panel][i].duration0 = 4;
        led_data[panel][i].level1 = 0;
        led_data[panel][i].duration1 = 8;
      }
      i++;
    }
  }
  panelDirty[panel] = true;

}

void init() {
  byteCounter = 0;
  MODE = 0;

  Serial.println("INIT");
}

void draw() {

  // Send the data to RMT
  for (int panel = 0; panel < NR_OF_PANELS; panel++)
    if (panelDirty[panel]) {
      rmtWrite(rmt_send[panel], led_data[panel], NR_OF_ALL_BITS);
      panelDirty[panel] = false;
      //Serial.println("DRAW Panel "+String(panel));
    }

  // reset
  byteCounter = 0;
  MODE = 0;

  Serial.println("DRAW");
  Serial.flush();
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
