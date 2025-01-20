#include <Arduino.h>
// #include <ESP32_FTPClient.h>

#include "server.h"
#include "sdcard.h"
#include "mic.h"
// #include "ftpp.h"
#include "blueclassic.h"

const char *id = "LR72TV";
const char *password = "Neket7e2";

void ioT_Wifi_Service();
void buttonRecord(void *arg);


char outBuf[128];

WiFiClient port_21;
WiFiClient port_data;

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    pinMode(LED_PIN, OUTPUT);
    // 文件上传手机app
    SDCardInit();
    initWifi(id, password);
    initBlueClassic();
    // uploadFiletoNode("/raw.txt");
    // ioT_Wifi_Service();
}

void loop()
{
    // 启动服务器必须加上这句话
    server.handleClient();
}

void ioT_Wifi_Service()
{
    xTaskCreatePinnedToCore(AP_wifi, NULL, 1024 * 4, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(connect_2_wifi, NULL, 1024 * 4, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(check_wifi_status, NULL, 1024 * 2, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(ClientHandle, NULL, 1024 * 4, NULL, 1, NULL, 1);
}


void recordTask()
{
    // recording task
    // LittleFSInit();
    i2sInit();
    SDCardInit();
    delay(3000);
    initWifi(id, password);
    xTaskCreatePinnedToCore(buttonRecord, NULL, 1024 * 64, NULL, 1, NULL, 1);
    xTaskCreate(buttonRecord, NULL, 1024 * 4, NULL, 1, NULL);
}


void buttonRecord(void *arg)
{
    while (true)
    {
        if (digitalRead(BUTTON_PIN) == HIGH && lock)
        {
            lock = false;
            Serial.println("begin to record");

            delay(1000);
            uploadFolder("/wav_folder");
            delay(1000);

            // i2sClose();
            // i2sInit();
            // i2s_adc();

            Serial.println("buttonRecord completed");
            lock = true;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}