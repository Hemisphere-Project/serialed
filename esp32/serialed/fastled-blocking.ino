#include <FastLED.h>

#define COLOR_ORDER GRB
#define CHIPSET     WS2811

#define BRIGHTNESS 2

CRGB leds[NR_PANELS][NR_LEDS];

void flb_start() {
  xTaskCreatePinnedToCore(
                    flb_draw,   /* Function to implement the task */
                    "flb_draw", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    0);         /* Core where the task should run */
}


void flb_draw( void * pvParameters ) {

  FastLED.addLeds<CHIPSET,  5, COLOR_ORDER>(leds[0], NR_LEDS); //.setCorrection(TypicalLEDStrip);
  FastLED.addLeds<CHIPSET, 18, COLOR_ORDER>(leds[1], NR_LEDS); //.setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<CHIPSET, 19, COLOR_ORDER>(leds[2], NR_LEDS).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<CHIPSET, 21, COLOR_ORDER>(leds[3], NR_LEDS).setCorrection(TypicalLEDStrip);
  // FastLED.setBrightness(BRIGHTNESS);

  while(true){
      if (newFrame) {

        // Copy buffer to main frame
        for(int x = 0; x < NR_PANELS; x++)
          for(int y = 0; y < NR_LEDS; y++)
            leds[x][y] = CRGB(framebuffer[(x*NR_LEDS+y)*3], framebuffer[(x*NR_LEDS+y)*3+1], framebuffer[(x*NR_LEDS+y)*3+2]);

        // draw
        FastLED.show();
        newFrame = false;
      }
      else delay(1);
  }
}
