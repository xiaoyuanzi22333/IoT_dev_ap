#ifndef BLUECLASSIC_H
#define BLUECLASSIC_H

#include <ArduinoJson.h>
#include "BluetoothSerial.h"
#include "sdcard.h"

#define CHUNK_SIZE 512             // 蓝牙传输分块大小（512字节）


BluetoothSerial SerialBT;



void initBlueClassic();
void checkBlueState(void* arg);
void handleBluetoothClient(void *arg);
void processCommand(String command);
void transferFileBluetooth(String filePath);
void getFileDict(String filePath);



void initBlueClassic()
{
    SerialBT.begin("ESP32_BT"); // 设置蓝牙设备名称
    xTaskCreatePinnedToCore(checkBlueState, NULL, 1024 * 4, NULL, 1, NULL, 0);
    Serial.println("The device started, now you can pair it with bluetooth!");
}

void checkBlueState(void* arg)
{
    while(true)
    {
        bool state = SerialBT.hasClient();
        if(state){
            Serial.println("bluetooth connected");
            xTaskCreatePinnedToCore(handleBluetoothClient, NULL, 1024 * 4, NULL, 1, NULL, 0);
            vTaskDelete(NULL);
        }
        vTaskDelay(300);
    }
}

void handleBluetoothClient(void *arg)
{
    while(true){
        if(SerialBT.available()){
            String msg = SerialBT.readString();
            Serial.println("Received Commmands: " + msg);
            int colonIndex = msg.indexOf(':'); // 查找冒号的位置
            String prefix = msg.substring(0, colonIndex); // 获取冒号前的部分
            String directory = msg.substring(colonIndex + 1); // 获取冒号后的部分
            if(prefix == "101"){
                // transferFileBluetooth("/example.txt");
                Serial.println("received prefix: " + prefix);
                Serial.println("received directory: " + directory);
                getFileDict(directory); // 获取目录文件列表
            }
        }
        vTaskDelay(50);
    }
}



void processCommand(String command)
{
    if(command.startsWith("101"))
    {
        Serial.println("101 received");
    }
    else if(command.startsWith("202"))
    {
        Serial.println("202 received");
    }
    else if(command.startsWith("303"))
    {
        Serial.println("303 received");
    }
    else if(command.startsWith("404"))
    {
        Serial.println("transferring file now");

    }
}


void transferFileBluetooth(String filePath)
{
    if (SerialBT.hasClient()) {
        File file = SD.open(filePath, "r"); // 从 ESP32 文件系统读取文件
        if (!file) 
        {
            SerialBT.println("Failed to open file");
            return;
        }

        // SerialBT.println("400: start file transfer");
        // delay(1000);
        // SerialBT.println("400: start file transfer");

        // String msg_400 = "400: start file transfer\n";
        // SerialBT.write((const uint8_t*)msg_400.c_str(),msg_400.length());
        // delay(50);
        // SerialBT.write((const uint8_t*)msg_400.c_str(),msg_400.length());
        // delay(50);
        Serial.println("400: start file transfer");

        // 分块发送文件
        uint8_t buffer[256]; // 每次发送 512 字节
        size_t bytesRead;
        int epoch = 1;
        // SerialBT.println("400: start file transfer");
        delay(100);

        while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
            Serial.print(epoch);Serial.print("");Serial.println(bytesRead);
            SerialBT.write(buffer, bytesRead); // 发送数据块
            delay(20); // 添加延迟以避免数据丢失
            epoch ++;
        }
        file.close();

        // 发送结束标志
        Serial.println("发送结束标志");
        String msg_401 = "401: end file transfer\n";
        SerialBT.write((const uint8_t*)msg_401.c_str(),msg_401.length());
        delay(50);
        SerialBT.write((uint8_t*)"", 0); // 发送空包标志结束
        Serial.println("401: file transfer end");
    }
}

void getFileDict(String filePath)
{
    File root = SD.open(filePath);
    JsonDocument jsonDoc;
    if(!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }
    File file = root.openNextFile();
    while (file) 
    {
        if (file.isDirectory()) {
            // Serial.print("  [目录] ");
            // Serial.println(file.name());

            // // 如果需要递归列出子目录内容
            // if (levels) {
            //     listDir(fs, file.name(), levels - 1);
            // }
            jsonDoc[file.name()] = "dir";
        } else {
            // Serial.print("  [文件] ");
            // Serial.print(file.name());
            // Serial.print("  大小: ");
            // Serial.println(file.size());
            jsonDoc[file.name()] = "file";
        }
        file = root.openNextFile();
    }
    root.close();

    // 序列化 JSON 对象为字符串
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // 打印 JSON 字符串
    Serial.println("生成的 JSON 对象:");
    Serial.println(jsonString);
    SerialBT.write((const uint8_t*)jsonString.c_str(),jsonString.length());
    delay(50);
    // SerialBT.println("401: end file transfer");
}

#endif