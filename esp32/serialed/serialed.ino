#include <WiFi.h>

/* CONFIG */

#define BAUDRATE 921600
//#define BRIGHTNESS 10
#define NR_OUTPUTS 1
#define NR_LEDS   150

/**/

#define FRAMESIZE (NR_OUTPUTS*NR_LEDS*3)
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
