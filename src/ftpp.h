#ifndef FTPP_H
#define FTPP_H

#include <ESP32_FTPClient.h>
#include <WiFiClient.h>
#include "sdcard.h"

char ftp_server[] = "192.168.103.43";
char ftp_user[] = "HOHO";
char ftp_pass[] = "711123";

ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass, 5000, 2);

void ftpp_test()
{
    Serial.println("start FTP testing");
    Serial.println("FTP client initing");
    ftp.OpenConnection();

    // 打印连接成功后的信息
    Serial.print("FTP status: ");
    Serial.println(ftp.isConnected());

    ftp.ChangeWorkDir("/rdf/myftp/123");
    ftp.InitFile("Type I");

    ftp.NewFile("hello.txt");

    Serial.println("start writing data");
    ftp.dclient.println("hello!!!!!world.");
    ftp.dclient.println("second line");

    delay(3000);
    ftp.CloseConnection();
}

void ftp_upload()
{
    Serial.println("FTP client initing");
    ftp.OpenConnection();

    // 打印连接成功后的信息
    Serial.print("FTP status: ");
    Serial.println(ftp.isConnected());

    ftp.ChangeWorkDir("/rdf/myftp/123");
    ftp.InitFile("Type I");

    ftp.NewFile("wavFile.wav");

    // 打开文件
    File file = SD.open("/rain.mp3");
    if (!file)
    {
        Serial.println("Failed to open file.");
        return;
    }

    Serial.println("Reading file content:");

    // 以 buffer 的形式读取文件内容
    const size_t bufferSize = 64; // 定义缓冲区大小
    char buffer[bufferSize];      // 缓冲区
    size_t bytesRead;             // 实际读取的字节数

    // 按块读取文件
    while ((bytesRead = file.readBytes(buffer, bufferSize)) > 0)
    {
        ftp.WriteData((unsigned char *)buffer, bytesRead);
        // Serial.write(buffer, bytesRead); // 将 buffer 内容写到串口
    }

    // 关闭文件
    file.close();
    Serial.println("\nFile reading completed.");
    // ftp.CloseFile();

    delay(3000);
    ftp.CloseConnection();
}

#endif