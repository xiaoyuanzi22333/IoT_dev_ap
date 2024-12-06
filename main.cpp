#include <WiFi.h>
#include <HTTPClient.h>
#if __has_include("esp_eap_client.h")
#include "esp_eap_client.h"
#else
#include "esp_wpa2.h"
#endif
#include <Wire.h>
#define EAP_IDENTITY "0235284403"   //请输入学号 或类似的网络账号
#define EAP_PASSWORD "HBXhbx711123"  //请输入密码
const char *ssid = "OnePass@HKSTP Free Wi-Fi"; // 网络SSID号，比如CMCC-PKU
int counter = 0;

void setup() {
  Serial.begin(115200);  
  delay(10);    
  Serial.println();
  Serial.print("Connecting to network: ");    
  Serial.println(ssid);  
  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA); //init wifi mode
#if __has_include ("esp_eap_client.h")
  esp_eap_client_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide identity
  esp_eap_client_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username
  esp_eap_client_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
  esp_wifi_sta_enterprise_enable();
#else
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide identity
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username --> identity and username is same
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
  esp_wifi_sta_wpa2_ent_enable();
#endif
  WiFi.begin(ssid); //connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter >= 60) {  //after 30 seconds timeout - reset board
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address set: ");
  Serial.println(WiFi.localIP());  //print LAN IP
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {  //if we are connected to Eduroam network
    counter = 0;                        //reset counter
    Serial.println("Wifi is still connected with IP: ");
    Serial.println(WiFi.localIP());            //inform user about his IP address
  } else if (WiFi.status() != WL_CONNECTED) {  //if we lost connection, retry
    WiFi.begin(ssid);
  }
  while (WiFi.status() != WL_CONNECTED) {  //during lost connection, print dots
    delay(500);
    Serial.print(".");
    counter++;
    if (counter >= 60) {  //30 seconds timeout - reset board
      ESP.restart();
    }
  }
  Serial.print("Connecting to website: ");
  HTTPClient http;
  http.begin("http://ip-api.com/json/");  // 准备启用连接
  int httpCode = http.GET();              // 发起GET请求
  if (httpCode > 0)                       // 如果状态码大于0说明请求过程无异常
  {
    if (httpCode == HTTP_CODE_OK)  // 请求被服务器正常响应，等同于httpCode == 200
    {
      String payload = http.getString();  // 读取服务器返回的响应正文数据
                                          // 如果正文数据很多该方法会占用很大的内存
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();  // 结束当前连接
  delay(10000);
   
}

