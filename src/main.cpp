#include <Arduino.h>
#include <ESP32_FTPClient.h>

#include "server.h"
#include "sdcard.h"
#include "mic.h"
#include "ftpp.h"

const char *id = "LR72TV";
const char *password = "Neket7e2";

void ftp_test1();
void get_ftp_res();
void buttonRecord(void *arg);
void get_pasv_port();
void get_data_res();
void ftp_task();


char outBuf[128];

WiFiClient port_21;
WiFiClient port_data;

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
    delay(3000);
    initWifi(id, password);
    // xTaskCreatePinnedToCore(buttonRecord, NULL, 1024 * 64, NULL, 1, NULL, 1);
    // xTaskCreate(buttonRecord, NULL, 1024 * 4, NULL, 1, NULL);
    IPAddress ipa;
    if (WiFi.hostByName(ftp_server, ipa))
    {
        Serial.println("FTP Server is reachable!");
    }
    else
    {
        Serial.println("FTP Server is unreachable!");
    }
    delay(1000);
    Serial.println("init completed");

    // ftpp_test(); //测试能否通过dclinet写入
    // ftp_task(); //测试dclient能否连接
    ftp_upload();
}

void loop()
{
    // server.handleClient();
}

void ftp_task()
{
    if (port_21.connect(ftp_server, 21))
    {
        Serial.println("connect");
    }
    get_ftp_res();
    // delay(3000);
    port_21.println(F("USER HOHO"));
    get_ftp_res();
    port_21.println(F("PASS 711123"));
    get_ftp_res();
    port_21.println(F("PASV"));
    get_ftp_res();
    get_pasv_port();

    port_21.println(F("NLST"));
    get_ftp_res();
    get_data_res();

    delay(5000);
    // port_21.stop();
}

void get_ftp_res()
{
    char thisByte;
    unsigned char outCount = 0;
    delay(2500);
    if (!port_21.available())
    {
        Serial.println("no response");
        return;
    }
    while (port_21.available())
    {
        thisByte = port_21.read();
        if (outCount < sizeof(outBuf))
        {
            outBuf[outCount] = thisByte;
            outCount++;
            outBuf[outCount] = 0;
        }
    }
    Serial.print("response: ");
    Serial.printf("%s\n", outBuf);
}


void get_data_res()
{
    char thisByte;
    unsigned char outCount = 0;
    delay(1500);
    if (!port_data.available())
    {
        Serial.println("no data");
        return;
    }
    while (port_data.available())
    {
        thisByte = port_data.read();
        if (outCount < sizeof(outBuf))
        {
            outBuf[outCount] = thisByte;
            outCount++;
            outBuf[outCount] = 0;
        }
    }
    Serial.print("data received: ");
    Serial.printf("%s\n", outBuf);
}

void get_pasv_port()
{
    char *tStr = strtok(outBuf, "(,");
    int array_pasv[6];
    for ( int i = 0; i < 6; i++) {
      tStr = strtok(NULL, "(,");
      if (tStr == NULL) {
        Serial.println("gg");
        return;
      }
      array_pasv[i] = atoi(tStr);
    }
    unsigned int hiPort, loPort;
    Serial.println("array result: ");
    Serial.println(array_pasv[4]);
    Serial.println(array_pasv[5]);

    Serial.print("data port: ");
    Serial.println((array_pasv[4]*256 + array_pasv[5]));

    if (port_data.connect(ftp_server, (array_pasv[4]*256 + array_pasv[5])))
    {
        Serial.println("connect");
    } else {
        Serial.println("data port failed");
    }
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