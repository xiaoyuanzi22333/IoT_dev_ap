#include "server.h"

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    pinMode(LED_PIN, OUTPUT); 
    xTaskCreatePinnedToCore(AP_wifi, NULL, 1024 * 4, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(connect_2_wifi, NULL, 1024 * 4, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(check_wifi_status, NULL, 1024 * 2, NULL, 1, NULL, 0);
}


void loop()
{
    server.handleClient();
}
