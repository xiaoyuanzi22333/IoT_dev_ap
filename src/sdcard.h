#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>

// 定義自定義 SPI 引腳
#define SD_MISO 34 // GPIO34
#define SD_MOSI 12 // GPIO12
#define SD_SCK 5   // GPIO5
#define SD_CS 15   // GPIO15

#define I2S_WS 19
#define I2S_SCK 23
#define I2S_SD 4
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (20000)
#define I2S_SAMPLE_BITS 16
#define I2S_READ_LEN (4 * 1024)
#define RECORD_TIME (15) // Seconds
#define I2S_CHANNEL_NUM (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

File file;
SPIClass sdSPI(VSPI);

const int headerSize = 44; // wave header size
const char filename[] = "/recording.wav";

void SDCardInit();
void listSD(const char* folderPath = "/");
void sdCardClean();
void updateWavHeader(String filename = filename);
void wavHeader(byte *header, int wavSize);


void SDCardInit()
{
    // 初始化 SPI
    Serial.println("Initializing SPI...");
    sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    Serial.println("SPI initialized.");

    // 初始化 SD 卡
    Serial.println("Initializing SD card...");
    if (!SD.begin(SD_CS, sdSPI))
    {
        Serial.println("SD card initialization failed!");
        while (1)
            yield();
    }

    SD.remove(filename); // Remove the existing file
    listSD();
}

void listSD(const char* folderPath)
{
    Serial.println(F("\r\nListing SD card files:"));
    static const char line[] PROGMEM = "=================================================";

    Serial.println(FPSTR(line));
    Serial.println(F("  File name                              Size"));
    Serial.println(FPSTR(line));

    File root = SD.open(folderPath);
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

    File file_list = root.openNextFile();
    while (file_list)
    {
        // if (file_list.isDirectory()) {
        //     Serial.print("DIR : ");
        //     String fileName = file_list.name();
        //     Serial.print(fileName);
        // } else {
        String fileName = file_list.name();
        Serial.print("  " + fileName);
        // 文件路径最多支持 31 个字符（可根据实际情况调整）
        int spaces = 33 - fileName.length(); // 格式化对齐
        if (spaces < 1)
            spaces = 1;
        while (spaces--)
        {
            Serial.print(" ");
        }
        String fileSize = (String)file_list.size();
        spaces = 10 - fileSize.length(); // 格式化对齐
        if (spaces < 1)
            spaces = 1;
        while (spaces--)
            Serial.print(" ");
        Serial.println(fileSize + " bytes");
        // }

        file_list.close();
        file_list = root.openNextFile(); // 获取下一个文件
    }

    root.close();
    Serial.println(FPSTR(line));
    Serial.println();
    delay(1000);
}

// 针对性删除
// 根目录下记得前面加上一个/
void sdCardFileDelete(String filePath)
{
    if (SD.exists(filePath))
    {
        Serial.println("文件存在，正在删除...");
        if (SD.remove(filePath))
        { // 删除文件
            Serial.println("文件已成功删除！");
        }
        else
        {
            Serial.println("文件删除失败！");
        }
    }
    else
    {
        Serial.println("文件不存在！");
    }
}

// 大清洗
void sdCardClean()
{
    Serial.println("start clean the SD card");
    File root = SD.open("/"); // 打开 SD 卡根目录
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

    Serial.println("成功打开根目录");
    File file = root.openNextFile(); // 获取下一个文件
    while (file)
    {
        String filename = file.name();
        if (filename == "System Volume Information")
        {
            file.close();
            file = root.openNextFile(); // 获取下一个文件
            continue;
        }
        Serial.print("deleting: ");
        Serial.println(filename);
        sdCardFileDelete("/" + filename);
        file.close();
        file = root.openNextFile(); // 获取下一个文件
    }

    listSD();
}

void updateWavHeader(String filename)
{
    // 重新打开文件以更新头部
    file = SD.open(filename, FILE_WRITE); // 使用 FILE_WRITE 模式读取并写入，不会覆盖文件
    if (!file)
    {
        Serial.println("Failed to open file file!");
    }
    else
    {
        // 将文件指针移动到末尾
        Serial.print("file file size: ");
        Serial.println(file.size());
        if (file.size() != 0)
        {
            file.seek(file.size());
        }
    }
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

void wavHeader(byte *header, int wavSize)
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
    header[24] = (char)(I2S_SAMPLE_RATE & 0xff);
    header[25] = (char)((I2S_SAMPLE_RATE >> 8) & 0xff);
    header[26] = (char)((I2S_SAMPLE_RATE >> 16) & 0xff);
    header[27] = (char)((I2S_SAMPLE_RATE >> 24) & 0xff);
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



#endif