#ifndef FILESYS_H
#define FILESYS_H

#include <LittleFS.h>
#include <HTTPClient.h>


#define I2S_WS 19
#define I2S_SCK 23
#define I2S_SD 4
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE  2000
#define I2S_SAMPLE_BITS 16
#define I2S_READ_LEN (8 * 1024)
#define RECORD_TIME (5) // Seconds
#define I2S_CHANNEL_NUM (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

File file;

void LittleFSInit(int headerSize = 44, char filename[] = "/recording.wav");
void listLittleFS(char filename[] = "/recording.wav");
void updateWavHeader(int headerSize=44, char filename[] = "/recording.wav");
void wavHeader(byte *header, int wavSize, int headerSize=44);
void uploadFile(const char *path, String serverUrl); 
void sendFile(); // 已弃用

void LittleFSInit(int headerSize = 44, char filename[] = "/recording.wav")
{
    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS initialisation failed!");
        while (1)
            yield();
    }

    LittleFS.remove(filename); // 移除掉已存在的file
    file = LittleFS.open(filename, FILE_WRITE);
    if (!file)
    {
        Serial.println("File is not available!");
    }

    byte header[headerSize];
    wavHeader(header, FLASH_RECORD_SIZE); // 生成wave header

    file.write(header, headerSize); // 写入header
    listLittleFS();
}


void listLittleFS(char filename[] = "/recording.wav")
{
    Serial.println(F("\r\nListing LittleFS files:"));
    static const char line[] PROGMEM = "=================================================";

    Serial.println(FPSTR(line));
    Serial.println(F("  File name                              Size"));
    Serial.println(FPSTR(line));

    fs::File root = LittleFS.open("/");
    if (!root)
    {
        Serial.println(F("Failed to open directory"));
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(F("Not a directory"));
        return;
    }

    fs::File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("DIR : ");
            String fileName = file.name();
            Serial.print(fileName);
        }
        else
        {
            String fileName = file.name();
            Serial.print("  " + fileName);
            // File path can be 31 characters maximum in LittleFS
            int spaces = 33 - fileName.length(); // Tabulate nicely
            if (spaces < 1)
                spaces = 1;
            while (spaces--)
                Serial.print(" ");
            String fileSize = (String)file.size();
            spaces = 10 - fileSize.length(); // Tabulate nicely
            if (spaces < 1)
                spaces = 1;
            while (spaces--)
                Serial.print(" ");
            Serial.println(fileSize + " bytes");
        }

        file = root.openNextFile();
    }

    Serial.println(FPSTR(line));
    Serial.println();
    delay(1000);
}

void updateWavHeader(int headerSize=44, char filename[] = "/recording.wav")
{
    // 重新打开文件以更新头部
    File file = LittleFS.open(filename, "r+"); // 使用 "r+" 模式读取并写入，不会覆盖文件
    if (!file)
    {
        Serial.println("Failed to reopen file for updating header");
        return;
    }

    // 获取文件大小
    int fileSize = file.size();

    // 重新生成 WAV 头部
    byte header[headerSize];
    wavHeader(header, fileSize - headerSize); // 音频数据的大小 = 总文件大小 - 头部大小

    // 移动文件指针到开头，并写入头部
    file.seek(0);                   // 回到文件开始位置
    file.write(header, headerSize); // 写入 WAV 头部

    file.close(); // 关闭文件

    Serial.println("WAV header updated successfully");
}

void wavHeader(byte *header, int wavSize, int headerSize=44)
{ // 数字小端格式，字符大端格式
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSize = wavSize + headerSize - 8;
    header[4] = (byte)(fileSize & 0xFF); // file size, 4byte integer
    header[5] = (byte)((fileSize >> 8) & 0xFF);
    header[6] = (byte)((fileSize >> 16) & 0xFF);
    header[7] = (byte)((fileSize >> 24) & 0xFF);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 0x10; // length of format data = 16, 4byte integer
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01; // format type:1(PCM), 2byte integer
    header[21] = 0x00;
    header[22] = 0x01; // channel number:1, 2byte integer
    header[23] = 0x00;
    header[24] = 0xD0; // sample rate:16000=0x00003E80, 4byte integer
    header[25] = 0x07;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x00; // SampleRate*BitPerSample*ChannelNum/8=16000*16*1/8=0x00007D00, 4byte integer
    header[29] = 0x7D;
    header[30] = 0x00;
    header[31] = 0x00;
    header[32] = 0x02; // BitPerSample*ChannelNum/8 = 2, 2byte integer
    header[33] = 0x00;
    header[34] = 0x10; // BitPerSample:16 = 0x0010, 2byte integer
    header[35] = 0x00;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (byte)(wavSize & 0xFF);
    header[41] = (byte)((wavSize >> 8) & 0xFF);
    header[42] = (byte)((wavSize >> 16) & 0xFF);
    header[43] = (byte)((wavSize >> 24) & 0xFF);
}

void uploadFile(const char *path, String serverUrl)
{
    File file = LittleFS.open(path, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file");
        return;
    }

    // 获取文件大小
    size_t fileSize = file.size();
    Serial.printf("File size: %d bytes\n", fileSize);

    // 初始化 HTTPClient
    HTTPClient http;
    http.begin(serverUrl);

    // 构建 multipart/form-data 请求头
    String boundary = "----ESP32Boundary"; // 定义 boundary
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);

    // 构建 multipart/form-data 请求体
    String bodyStart = "--" + boundary + "\r\n" +
                       "Content-Disposition: form-data; name=\"WavFile\"; filename=\"example.wav\"\r\n" +
                       "Content-Type: text/plain\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    // 计算请求体总大小
    size_t totalSize = bodyStart.length() + fileSize + bodyEnd.length();

    // 设置请求体大小
    http.addHeader("Content-Length", String(totalSize));

    // 开始 POST 请求
    //   int httpResponseCode = http.GET();
    http.GET();
    int httpResponseCode = http.POST("");
    Serial.println(httpResponseCode);
    Serial.println(http.connected());

    WiFiClient *stream = http.getStreamPtr();
    if (stream == nullptr)
    {
        Serial.println("stream is nullptr");
        return;
    }

    // 发送 multipart/form-data 请求头和 bodyStart
    stream->print(bodyStart);

    // 逐块读取文件内容并发送
    uint8_t buffer[512];
    while (file.available())
    {
        size_t bytesRead = file.read(buffer, sizeof(buffer));
        stream->write(buffer, bytesRead);
    }

    // 发送 bodyEnd
    stream->print(bodyEnd);
    //   httpResponseCode = http.POST("");
    // 获取服务器响应
    if (httpResponseCode > 0)
    {
        String response = http.getString();
        Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
        Serial.println("Response from server:");
        Serial.println(response);
    }
    else
    {
        Serial.printf("Error in sending POST request: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // 关闭 HTTP 请求和文件
    http.end();
    file.close();
}

void sendFile()
{
    char filePath[] = "/recording.wav";
    File file = LittleFS.open(filePath, "r"); // 打开文件
    if (!file)
    {
        Serial.println("Failed to open file!");
        return;
    }

    // 获取文件大小
    size_t fileSize = file.size();
    Serial.printf("Start sending file: %s (%d bytes)\n", filePath, fileSize);
    delay(3000);
    uint8_t buffer[512]; // 数据缓冲区
    size_t bytesSent = 0;
    Serial.println("     ");
    Serial.println("     ");
    Serial.println("     ");
    Serial.println("     ");
    // 循环读取文件并发送
    while (file.available())
    {
        size_t bytesRead = file.read(buffer, 512); // 读取文件数据到缓冲区
        Serial.write(buffer, bytesRead);           // 通过串口发送数据
        bytesSent += bytesRead;

        // 等待电脑端确认，避免数据丢失
        delay(100);
        if (bytesSent >= fileSize)
        {
            break;
        }
    }

    file.close();
    Serial.println("File transfer complete.");
}

#endif


