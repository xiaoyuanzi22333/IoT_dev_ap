#include <Arduino.h>

#include "server.h"
#include "sdcard.h"
#include "mic.h"


const char* id = "LR72TV";
const char* password = "Neket7e2";

void buttonRecord(void *arg);

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    pinMode(LED_PIN, OUTPUT); 
    // xTaskCreatePinnedToCore(AP_wifi, NULL, 1024 * 4, NULL, 1, NULL, 0);
    // xTaskCreatePinnedToCore(connect_2_wifi, NULL, 1024 * 4, NULL, 1, NULL, 1);
    // xTaskCreatePinnedToCore(check_wifi_status, NULL, 1024 * 2, NULL, 1, NULL, 0);
    // LittleFSInit();
    // i2sInit();
    SDCardInit();
    // initWifi(id, password);
    xTaskCreatePinnedToCore(buttonRecord, NULL, 1024 * 4, NULL, 1, NULL, 1);
}


void loop()
{
    // server.handleClient();
}


void buttonRecord(void *arg){
    while (true)
    {
        if (digitalRead(BUTTON_PIN) == HIGH && lock)
        {
            lock = false;
            Serial.println("begin to record");
            // i2sClose();
            i2sInit();
            delay(1000);
            i2s_adc();
            Serial.println("buttonRecord completed");
            lock = true;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}