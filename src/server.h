#ifndef SERVER_H
#define SERVER_H

#define BUTTON_PIN 18
#define LED_PIN 2

#include <WiFi.h>
#include <WebServer.h>
#include <esp_wpa2.h>

const char *ap_ssid = "PIVOT_TEST";
const char *ap_password = "12345678";

const unsigned long timeout = 10000; // 10 秒
unsigned long start_time;

String wifi_sid = "";
String wifi_pwd = "";
String wifi_iden = "";

WebServer server(80);

void AP_wifi(void *arg);
void connect_2_wifi(void *arg);

void handleMsg()
{
    String response = "";
    if (server.hasArg("sid"))
    {
        String sid = server.arg("sid");
        wifi_sid = sid;
        Serial.println("received wifi_SID: " + sid);
        response += "receieved wifi_SID: " + sid +'\n';
    }
    if (server.hasArg("pwd"))
    {
        String pwd = server.arg("pwd");
        wifi_pwd = pwd;
        Serial.println("received wifi_pwd: " + pwd);
        response += "receieved wifi_pwd: " + pwd +'\n';
    }
    if (server.hasArg("iden"))
    {
        String iden = server.arg("iden");
        wifi_iden = iden;
        Serial.println("received wifi_iden: " + iden);
        response += "receieved wifi_iden: " + iden +'\n';
    }
    if (!server.hasArg("sid") && !server.hasArg("pwd") && !server.hasArg("iden"))
    {
        server.send(400, "text/plain", "no msg");
    }
    server.send(200, "text/plain", response);
}

void initAP_server()
{
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("热点已启动");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("ap_SSID: ");
    Serial.println(ap_ssid);
    Serial.print("ap_password: ");
    Serial.println(ap_password);

    if (server.uri() == "/msg") {
        Serial.println("HTTP 服务器已在运行");
        return;
    }

    server.on("/msg", handleMsg);
    server.begin();
    Serial.println("HTTP 服务器已启动");
}

void AP_wifi(void *arg)
{
    while (1)
    {
        if (digitalRead(BUTTON_PIN) == HIGH)
        {
            initAP_server();
            vTaskDelete(NULL);
        }
        vTaskDelay(200);
    }
}

void connect_2_wifi(void *arg)
{
    while (1)
    {
        if (wifi_sid != "" && wifi_pwd != "" && wifi_iden == "")
        {
            Serial.println("");
            Serial.println("start connecting normal WiFi");
            WiFi.disconnect(true);
            delay(100);
            WiFi.mode(WIFI_OFF);
            delay(100);
            WiFi.mode(WIFI_STA); // 或 WIFI_AP，根据需要选择模式
            Serial.print("正在连接到 Wi-Fi...");
            WiFi.begin(wifi_sid, wifi_pwd);
            start_time = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start_time < timeout)
            {
                delay(1000);
                Serial.print(".");
            }

            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("");
                Serial.println("device cannot connect to wifi");
                Serial.println("please check the wifi id and pwd");
                wifi_sid, wifi_pwd = "";
                delay(100);
                WiFi.mode(WIFI_OFF);
                delay(100);
                WiFi.mode(WIFI_AP);
                wifi_sid, wifi_pwd, wifi_iden = "";
                initAP_server();
                vTaskDelay(200);
                continue;
            }

            Serial.println();
            Serial.println("Wi-Fi 已连接！");
            Serial.print("IP 地址: ");
            Serial.println(WiFi.localIP()); // 打印 ESP32 的 IP 地址
            server.stop();
            vTaskDelete(NULL);
        }
        if (wifi_sid != "" && wifi_pwd != "" && wifi_iden != "")
        {
            Serial.println("");
            Serial.println("connecting to Enterprise WiFi");
            WiFi.disconnect(true);
            delay(100);
            WiFi.mode(WIFI_OFF);
            delay(100);
            WiFi.mode(WIFI_STA); // 或 WIFI_AP，根据需要选择模式
            Serial.print("正在连接到 Wi-Fi...");
            const char* username = wifi_iden.c_str();
            const char* password = wifi_pwd.c_str();
            esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username));  // 设置用户名
            esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username));  // 设置用户名
            esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password));  // 设置密码
            esp_wifi_sta_wpa2_ent_enable();          // 启用 WPA2 企业认证

            WiFi.begin(wifi_sid);
            Serial.println("正在连接企业WiFi...");

            // 等待连接成功
            start_time = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start_time < timeout) {
              delay(1000);
              Serial.println("连接中...");
            }

            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("");
                Serial.println("device cannot connect to wifi");
                Serial.println("please check the wifi id and pwd");
                wifi_sid, wifi_pwd = "";
                delay(100);
                WiFi.mode(WIFI_OFF);
                delay(100);
                WiFi.mode(WIFI_AP);
                wifi_sid, wifi_pwd, wifi_iden = "";
                initAP_server();
                vTaskDelay(200);
                continue;
            }

            Serial.println("连接成功！");
            Serial.print("IP地址: ");
            Serial.println(WiFi.localIP());
            server.stop();
        }
        vTaskDelay(200);
    }   
}


void check_wifi_status(void *arg)
{
    while (1)
    {
        if (WiFi.getMode() == 2)
        {
            digitalWrite(LED_PIN, HIGH);
            delay(1000);
            digitalWrite(LED_PIN, LOW);
        }
        else if (WiFi.status() == WL_CONNECTED)
        {
            digitalWrite(LED_PIN, HIGH);
        }
        else
        {
            digitalWrite(LED_PIN, LOW);
        }
        vTaskDelay(500);
    }
}

#endif