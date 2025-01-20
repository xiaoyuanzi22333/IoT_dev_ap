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
    // ftp.InitFile("Type I");

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
    // ftp.InitFile("Type I");

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


// void ftp_transfer()
// {
//     // FTP task
//     IPAddress ipa;
//     if (WiFi.hostByName(ftp_server, ipa))
//     {
//         Serial.println("FTP Server is reachable!");
//     }
//     else
//     {
//         Serial.println("FTP Server is unreachable!");
//     }
//     delay(1000);
//     ftpp_test(); //测试能否通过dclinet写入
//     ftp_task(); //测试dclient能否连接
//     ftp_upload(); //测试ftp文件上传
// }


// void ftp_task()
// {
//     if (port_21.connect(ftp_server, 21))
//     {
//         Serial.println("connect");
//     }
//     get_ftp_res();
//     // delay(3000);
//     port_21.println(F("USER HOHO"));
//     get_ftp_res();
//     port_21.println(F("PASS 711123"));
//     get_ftp_res();
//     port_21.println(F("PASV"));
//     get_ftp_res();
//     get_pasv_port();

//     port_21.println(F("NLST"));
//     get_ftp_res();
//     get_data_res();

//     delay(5000);
//     // port_21.stop();
// }

// void get_ftp_res()
// {
//     char thisByte;
//     unsigned char outCount = 0;
//     delay(2500);
//     if (!port_21.available())
//     {
//         Serial.println("no response");
//         return;
//     }
//     while (port_21.available())
//     {
//         thisByte = port_21.read();
//         if (outCount < sizeof(outBuf))
//         {
//             outBuf[outCount] = thisByte;
//             outCount++;
//             outBuf[outCount] = 0;
//         }
//     }
//     Serial.print("response: ");
//     Serial.printf("%s\n", outBuf);
// }


// void get_data_res()
// {
//     char thisByte;
//     unsigned char outCount = 0;
//     delay(1500);
//     if (!port_data.available())
//     {
//         Serial.println("no data");
//         return;
//     }
//     while (port_data.available())
//     {
//         thisByte = port_data.read();
//         if (outCount < sizeof(outBuf))
//         {
//             outBuf[outCount] = thisByte;
//             outCount++;
//             outBuf[outCount] = 0;
//         }
//     }
//     Serial.print("data received: ");
//     Serial.printf("%s\n", outBuf);
// }

// void get_pasv_port()
// {
//     char *tStr = strtok(outBuf, "(,");
//     int array_pasv[6];
//     for ( int i = 0; i < 6; i++) {
//       tStr = strtok(NULL, "(,");
//       if (tStr == NULL) {
//         Serial.println("gg");
//         return;
//       }
//       array_pasv[i] = atoi(tStr);
//     }
//     unsigned int hiPort, loPort;
//     Serial.println("array result: ");
//     Serial.println(array_pasv[4]);
//     Serial.println(array_pasv[5]);

//     Serial.print("data port: ");
//     Serial.println((array_pasv[4]*256 + array_pasv[5]));

//     if (port_data.connect(ftp_server, (array_pasv[4]*256 + array_pasv[5])))
//     {
//         Serial.println("connect");
//     } else {
//         Serial.println("data port failed");
//     }
// }




#endif