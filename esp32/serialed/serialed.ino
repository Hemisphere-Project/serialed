#include <WiFi.h>
#define BAUDRATE 921600

#define NR_PANELS 2
#define NR_LEDS   256

#define FRAMESIZE (NR_PANELS*NR_LEDS*3)
#define BUFFERSIZE (FRAMESIZE+32)

byte framebuffer[BUFFERSIZE];
bool newFrame = false;

void setup() {

  WiFi.mode(WIFI_OFF);
  btStop();

  com_init();
  flb_start();
}


void loop() {
  newFrame = com_loop();
}
