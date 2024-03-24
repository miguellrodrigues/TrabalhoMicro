#include "include/i2c-lcd.h"
#include <esp_log.h>
#include <driver/i2c.h>
#include <freertos/task.h>
#include <stdint.h>

#define SLAVE_ADDRESS_LCD 0x27
#define I2C_NUM I2C_NUM_0

static const char *TAG = "LCD";

esp_err_t err;

void lcd_send_cmd(uint8_t cmd) {
    uint8_t data_t[4];
    data_t[0] = (cmd & 0xF0) | 0x0C; // en=1, rs=0
    data_t[1] = (cmd & 0xF0) | 0x08; // en=0, rs=0
    data_t[2] = ((cmd << 4) & 0xF0) | 0x0C; // en=1, rs=0
    data_t[3] = ((cmd << 4) & 0xF0) | 0x08; // en=0, rs=0
    err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
    if (err != 0) ESP_LOGI(TAG, "Error in sending command");
}

void lcd_send_data(uint8_t data) {
    uint8_t data_t[4];
    data_t[0] = (data & 0xF0) | 0x0D; // en=1, rs=1
    data_t[1] = (data & 0xF0) | 0x09; // en=0, rs=1
    data_t[2] = ((data << 4) & 0xF0) | 0x0D; // en=1, rs=1
    data_t[3] = ((data << 4) & 0xF0) | 0x09; // en=0, rs=1
    err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
    if (err != 0) ESP_LOGI(TAG, "Error in sending data");
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(2)); // wait for >1.52ms
}

void lcd_put_cur(int row, int col) {
    uint8_t command = (row == 0) ? (col | 0x80) : (col | 0xC0);
    lcd_send_cmd(command);
}

void lcd_init(void) {
    vTaskDelay(pdMS_TO_TICKS(40)); // wait for >40ms
    lcd_send_cmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(5)); // wait for >4.1ms
    lcd_send_cmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(0.1)); // wait for >100us
    lcd_send_cmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(0.01));
    lcd_send_cmd(0x20); // 4bit mode
    vTaskDelay(pdMS_TO_TICKS(0.01));

    lcd_send_cmd(0x28); // Function set
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_cmd(0x08); // Display off
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_cmd(0x01); // Clear display
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_cmd(0x06); // Entry mode set
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_cmd(0x0C); // Display on
    vTaskDelay(pdMS_TO_TICKS(1));
}

void lcd_send_string(char *str) {
    while (*str) lcd_send_data(*str++);
}

void lcd_send_custom(const char* str, uint8_t pos_x, uint8_t pos_y) {
    lcd_send_cmd(0x40); // Set CGRAM address
    for (int i = 0; i < 8; ++i) {
        lcd_send_data(str[i]);
    }
    lcd_put_cur(pos_x, pos_y);
    lcd_send_data(0);
}

void update_value(char *string, volatile float value, uint8_t pos_x, uint8_t pos_y) {
    sprintf(string, "%5.1f", value);
    lcd_put_cur(pos_x, pos_y);
    lcd_send_string(string);
}
