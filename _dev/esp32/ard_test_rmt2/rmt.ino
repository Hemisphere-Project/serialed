#include <esp32-hal.h>

byte rmtPins[NR_OF_PANELS] = {5, 18, 19, 21};
rmt_data_t led_data[NR_OF_PANELS][NR_OF_BITS];
rmt_obj_t* rmt_send[NR_OF_PANELS];
bool panelDirty[NR_OF_PANELS];


// INIT
void rmt_init() {
  for (int PANEL = 0; PANEL < NR_OF_PANELS; PANEL++) {
    if ((rmt_send[PANEL] = rmtInit(rmtPins[PANEL], true, RMT_MEM_64)) == NULL) {
      Serial.print("ERROR: init sender faiLED\n");
    }

    float realTick = rmtSetTick(rmt_send[PANEL], 100);
    Serial.printf("INFO: Channel %i - tick set to: %f ns\n", PANEL , realTick);
  }
}


// SET LED
void rmt_setLED(int panel, int led, int colo[3]) {

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


// DRAW
void rmt_draw() {

  // Send the data to RMT
  for (int panel = 0; panel < NR_OF_PANELS; panel++)
    if (panelDirty[panel]) {
      rmtWrite(rmt_send[panel], led_data[panel], NR_OF_BITS);
      panelDirty[panel] = false;
      //Serial.print("DRAW Panel "+String(panel)+"\n");
    }
}
