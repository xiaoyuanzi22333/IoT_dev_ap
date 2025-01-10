#ifndef BLUECLASSIC_H
#define BLUECLASSIC_H

#include "BluetoothSerial.h"
#include "sdcard.h"

#define CHUNK_SIZE 512             // 蓝牙传输分块大小（512字节）


BluetoothSerial SerialBT;



void initBlueClassic();
void checkBlueState(void* arg);
void handleBluetoothClient(void *arg);
void processCommand(String command);
void transferFileBluetooth(String filePath);


void initBlueClassic()
{
    SerialBT.begin("ESP32_BT"); // 设置蓝牙设备名称
    xTaskCreatePinnedToCore(checkBlueState, NULL, 1024 * 4, NULL, 1, NULL, 0);
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
            if(msg.startsWith("101")){
                transferFileBluetooth("/example.txt");
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
        uint8_t buffer[512]; // 每次发送 512 字节
        size_t bytesRead;
        int epoch = 1;
        // SerialBT.println("400: start file transfer");
        delay(100);

        while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
            Serial.print(epoch);Serial.print("");Serial.println(bytesRead);
            SerialBT.write(buffer, bytesRead); // 发送数据块
            delay(10); // 添加延迟以避免数据丢失
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
        
        // SerialBT.write((const uint8_t*)msg_401.c_str(),msg_401.length());
        // delay(50);
    }
}

#endif