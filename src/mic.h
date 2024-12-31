#ifndef MIC_H
#define MIC_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "sdcard.h"
#include <HTTPClient.h>


unsigned long startTime;
unsigned long endTime;

void i2sInit();
void i2sClose();
void i2s_adc();
void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len);
void uploadFile(String path);
void uploadFolder(String folderPath);
bool folderExists(String folderPath);

HTTPClient http;

void i2sInit()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,
        .dma_buf_count = 64, // 64 buff per dma
        .dma_buf_len = 1024, // 1024 samples per buffer
        .use_apll = 1};      // use APLL-CLK,frequency 16MHZ-128MHZ,it's for audio

    esp_err_t ret = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (ret != ESP_OK)
    {
        Serial.printf("I2S driver install failed: %s\n", esp_err_to_name(ret));
        return;
    }

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD};

    ret = i2s_set_pin(I2S_PORT, &pin_config);
    if (ret != ESP_OK)
    {
        Serial.printf("I2S pin configuration failed: %s\n", esp_err_to_name(ret));
        i2s_driver_uninstall(I2S_PORT);
        return;
    }

    Serial.println("I2s Init Success");
};

void i2sClose()
{
    i2s_stop(I2S_PORT);
    i2s_driver_uninstall(I2S_PORT);
    Serial.println("closed i2s");
}

bool lock = true;


void i2s_adc()
{
    Serial.println("start opening file");
    File raw = SD.open("/kysten2.txt", FILE_WRITE);
    File file = SD.open(filename, FILE_WRITE);
    byte header[headerSize];
    wavHeader(header, FLASH_RECORD_SIZE); // Generate WAV header
    file.write(header, headerSize); // Write header

    Serial.println('a');
    int i2s_read_len = I2S_READ_LEN;
    int flash_wr_size = 0;
    size_t bytes_read;
    char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));
    if (i2s_read_buff == NULL)
    {
        Serial.println("Failed to allocate memory for i2s_read_buff");
        vTaskDelete(NULL);
    }

    uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char));
    if (flash_write_buff == NULL)
    {
        Serial.println("Failed to allocate memory for flash_write_buff");
        free(i2s_read_buff);
        vTaskDelete(NULL);
    }

    Serial.println('b');
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

    Serial.println(" *** Recording Start *** ");
    startTime = millis();
    while (flash_wr_size < FLASH_RECORD_SIZE)
    {
        i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
        file.write((const byte *)flash_write_buff, i2s_read_len);
        raw.write((const byte *)flash_write_buff, i2s_read_len);
        flash_wr_size += i2s_read_len;
        ets_printf("Recording Progress: %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
    }
    Serial.println(" *** Recording Complete *** ");
    endTime = millis();
    Serial.println((endTime - startTime));
    Serial.println("Recording complete");

    file.close();
    raw.close();
    free(i2s_read_buff);
    free(flash_write_buff);

    updateWavHeader();
    listSD();

    Serial.println("Start transferring");
    delay(1000);
    i2sClose();
    uploadFile("/recording.wav");
}


void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;
    float filteredSample = 0;
    // 一个采样点是2byte，每2个byte
    for (int i = 0; i < len; i += 2)
    {
        dac_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
        d_buff[j++] = 0;
        d_buff[j++] = dac_value * 256 / 4096;
    }
}

// for test only
const char* ID = "LR72TV";
const char* PASSWORD = "Neket7e2";
const String serverUrl = "http://192.168.103.41:8000";

void uploadFile(String path)
{
    initWifi(ID, PASSWORD);
    delay(1000);
    File file = SD.open(path, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file");
        return;
    }

    size_t fileSize = file.size();
    Serial.printf("File size: %d bytes\n", fileSize);

    http.begin(serverUrl);

    String boundary = "----ESP32Boundary";
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);

    String bodyStart = "--" + boundary + "\r\n" +
                       "Content-Disposition: form-data; name=\"WavFile\"; filename=\""+ file.name() +"\"\r\n" +
                       "Content-Type: text/plain\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    size_t totalSize = bodyStart.length() + fileSize + bodyEnd.length();

    http.addHeader("Content-Length", String(totalSize));

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

    stream->print(bodyStart);

    int buffer_length = 512*4;
    uint8_t buffer[buffer_length];
    unsigned long start = millis();
    int i = 0;
    int record_size = file.size();
    while (file.available())
    {
        // Serial.print("buff transfer: "); Serial.println(i);
        size_t bytesRead = file.read(buffer, sizeof(buffer));
        stream->write(buffer, bytesRead);
        i += buffer_length;
        ets_printf("Uploading Progress: %u%%\n", i * 100 / record_size);
    }
    Serial.print("time used: "); Serial.println(millis()-start);

    stream->print(bodyEnd);

    if (httpResponseCode > 0)
    {
        String response = http.getString();
        Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
        Serial.println("Response from server:");
        Serial.println(response);
        Serial.println("response endddd");
    }
    else
    {
        Serial.printf("Error in sending POST request: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    file.close();
}


void uploadFolder(String folderPath)
{
    if (!folderExists(folderPath))
    {
        Serial.println("folderPath error");
        return;
    }
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
    initWifi(ID, PASSWORD);
    delay(1000);
    while (file_list)
    {
        
        String filePath = folderPath + '/' + file_list.name();
        Serial.println(filePath);
        uploadFile(filePath);
        file_list.close();
        file_list = root.openNextFile();
    }
    root.close();
    WiFi.disconnect();
    Serial.println("Wi-Fi 已断开");
}

bool folderExists(String folderPath)
{
    File dir = SD.open(folderPath); // 尝试打开文件夹
    if (!dir)
    {
        // 如果无法打开，则目录不存在
        return false;
    }
    bool isFolder = dir.isDirectory();
    dir.close(); // 关闭文件夹
    return isFolder;
}

#endif