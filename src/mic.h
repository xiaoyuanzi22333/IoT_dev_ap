#ifndef MIC_H
#define MIC_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "filesys.h"


unsigned long start_time;
unsigned long end_time;

void i2sInit();
void i2sClose();
void i2s_adc(void *arg);
void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len);

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
};

void i2sClose()
{
    i2s_stop(I2S_PORT);
    i2s_driver_uninstall(I2S_PORT);
}

void i2s_adc(void *arg)
{
    Serial.println('a');
    int i2s_read_len = I2S_READ_LEN;
    int flash_wr_size = 0;
    size_t bytes_read;
    char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));
    if (i2s_read_buff == NULL)
    {
        Serial.println("Failed to allocate memory for i2s_read_buff");
        vTaskDelete(NULL); // 删除当前任务
    }

    uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char));
    if (flash_write_buff == NULL)
    {
        Serial.println("Failed to allocate memory for flash_write_buff");
        free(i2s_read_buff);
        vTaskDelete(NULL);
    }

    Serial.println('b');

    // 准备录音
    // i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

    Serial.println(" *** Recording Start *** ");
    start_time = millis();
    // while(millis() - start_time < RECORD_TIME*1000)
    while (flash_wr_size < FLASH_RECORD_SIZE)
    {
        i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
        file.write((const byte *)flash_write_buff, i2s_read_len);
        flash_wr_size += i2s_read_len;
        ets_printf("Recording Progress: %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
    }
    Serial.println(" *** Recording Complete *** ");
    end_time = millis();
    Serial.println((end_time - start_time));
    Serial.println("Recording complete");

    // 关闭文件并释放内存
    file.close();
    free(i2s_read_buff);
    free(flash_write_buff);

    // 更新 WAV 头部
    updateWavHeader();

    listLittleFS();

    Serial.println("start transferring");
    delay(1000);
    // sendFile(filename);
    // xTaskCreatePinnedToCore(sendFile, "sendfile", 4096, NULL, 1, NULL, 0);
    //   sendFile();
    uploadFile("/recording.wav","http://192.168.103.46:8000/");
    vTaskDelete(NULL); // 删除任务
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

#endif