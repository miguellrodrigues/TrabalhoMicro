//
// Created by miguel on 23/03/24.
//

#include <include/nfc.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <freertos/task.h>
#include <stdint.h>

#define SLAVE_ADDRESS_NFC 0x20
#define I2C_NUM I2C_NUM_0

static const char *TAG = "NFC";

esp_err_t err;

void nfc_init(void) {
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
    err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_NFC, data, 4, 1000);
    if (err != 0) ESP_LOGI(TAG, "Error in initializing NFC");
}

uint8_t *nfc_read() {
    uint8_t *data = malloc(4*sizeof(uint8_t));
    err = i2c_master_read_from_device(I2C_NUM, SLAVE_ADDRESS_NFC, data, 4, 1000);
    if (err != 0) ESP_LOGI(TAG, "Error in reading data");
    return data;
}

esp_err_t err;
