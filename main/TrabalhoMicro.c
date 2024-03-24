#include <driver/i2c.h>
#include <driver/gpio.h>

#include <esp_log.h>
#include <esp_system.h>
#include <esp_spiffs.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#include <cJSON.h>
#include <string.h>

#include "include/i2c-lcd.h"
#include "include/nfc.h"


#define TAG "NFC_Tag_System"

typedef enum {
    STATE_IDLE, STATE_WAITING_FOR_USER_INPUT, STATE_WAITING_FOR_NFC, STATE_CREATING_USER, STATE_DELETING_USER
} state_t;

// states prototypes
void state_idle();

void state_waiting_for_user_input();

void state_waiting_for_nfc();

void state_creating_user();

void state_deleting_user();

// config prototypes
void i2c_config();

esp_vfs_spiffs_conf_t file_config();

// files prototypes
uint8_t file_exists(const char *path);

FILE *open_file(const char *path, const char *mode);

void close_file(FILE *file);

cJSON *load_json(const char *path);

void save_json(const char *path, cJSON *json);

// users file functions
void create_users_file(esp_vfs_spiffs_conf_t conf);

void list_users();

cJSON *get_user(const char *nfc_id);

void add_user(const char *name, const char *nfc_id);

void remove_user(const char *nfc_id);

// Global variables
cJSON *users = NULL;
static state_t current_state = STATE_IDLE;

_Noreturn void app_main() {
    // Configuration
    i2c_config();
    lcd_init();
    nfc_init();

    esp_vfs_spiffs_conf_t conf = file_config();
    create_users_file(conf);

    users = load_json("/spiffs/users.json");

    while (1) {
        switch (current_state) {
            case STATE_IDLE:
                state_idle();
                break;
            case STATE_WAITING_FOR_USER_INPUT:
                state_waiting_for_user_input();
                break;
            case STATE_WAITING_FOR_NFC:
                state_waiting_for_nfc();
                break;
            case STATE_CREATING_USER:
                state_creating_user();
                break;
            case STATE_DELETING_USER:
                state_deleting_user();
                break;
            default:
                ESP_LOGE(TAG, "Invalid state!");
        }
    }
}

// config functions

void i2c_config() {
    i2c_config_t i2c_config = {.mode             = I2C_MODE_MASTER, .sda_io_num       = GPIO_NUM_21, .sda_pullup_en    = GPIO_PULLUP_ENABLE, .scl_io_num       = GPIO_NUM_22, .scl_pullup_en    = GPIO_PULLUP_ENABLE, .master.clk_speed = 100000};
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

esp_vfs_spiffs_conf_t file_config() {
    esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs", .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true};

    return conf;
}

// states functions

void state_idle() {
    lcd_clear();
    lcd_send_string("Waiting for NFC...");

    vTaskDelay(pdMS_TO_TICKS(100));
}

void state_waiting_for_user_input() {
    lcd_clear();
    lcd_send_string("Waiting for user input...");

    ESP_LOGI(TAG, "Waiting for user input...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    current_state = STATE_CREATING_USER;
}

void state_waiting_for_nfc() {
    lcd_clear();
    lcd_send_string("Waiting for NFC tag...");

    ESP_LOGI(TAG, "Waiting for NFC tag...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    current_state = STATE_IDLE;
}

void state_creating_user() {
    lcd_clear();
    lcd_send_string("Waiting for user id...");

    ESP_LOGI(TAG, "Creating user...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    current_state = STATE_IDLE;
}

void state_deleting_user() {
    lcd_clear();
    lcd_send_string("Waiting for user id...");

    ESP_LOGI(TAG, "Deleting user...");
    vTaskDelay(pdMS_TO_TICKS(1000)); // Example delay, replace with actual user deletion process
    current_state = STATE_IDLE;
}

// files functions
uint8_t file_exists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

FILE *open_file(const char *path, const char *mode) {
    return fopen(path, mode);
}

void close_file(FILE *file) {
    fclose(file);
}

// JSON functions

cJSON *load_json(const char *path) {
    FILE *file = open_file(path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *) malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    cJSON *json = cJSON_Parse(data);
    free(data);
    close_file(file);
    return json;
}

void save_json(const char *path, cJSON *json) {
    FILE *file = open_file(path, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    char *data = cJSON_Print(json);
    fwrite(data, 1, strlen(data), file);
    free(data);
    close_file(file);
}

// users file functions
void create_users_file(esp_vfs_spiffs_conf_t conf) {
    esp_vfs_spiffs_register(&conf);
    if (!file_exists("/spiffs/users.json")) {
        cJSON *empty_array = cJSON_CreateArray();
        save_json("/spiffs/users.json", empty_array);
        free(empty_array);
    }
}

void list_users() {
    if (!users) {
        ESP_LOGE(TAG, "Failed to load users file");
        return;
    }

    cJSON *user = NULL;
    cJSON_ArrayForEach(user, users) {
        const char *name = cJSON_GetObjectItem(user, "name")->valuestring;
        const char *nfc_id = cJSON_GetObjectItem(user, "nfc_id")->valuestring;
        ESP_LOGI(TAG, "Name: %s, NFC ID: %s", name, nfc_id);
    }
}

cJSON *get_user(const char *nfc_id) {
    if (!users) {
        ESP_LOGE(TAG, "Failed to load users file");
        return NULL;
    }

    cJSON *user = NULL;
    cJSON_ArrayForEach(user, users) {
        const char *nfc_id_user = cJSON_GetObjectItem(user, "nfc_id")->valuestring;
        if (strcmp(nfc_id, nfc_id_user) == 0) {
            const char *name = cJSON_GetObjectItem(user, "name")->valuestring;
            ESP_LOGI(TAG, "Name: %s, NFC ID: %s", name, nfc_id);
            return user;
        }
    }
    ESP_LOGI(TAG, "User not found");
    return NULL;
}

void add_user(const char *name, const char *nfc_id) {
    if (!users) {
        ESP_LOGE(TAG, "Failed to load users file");
        return;
    }

    cJSON *user = cJSON_CreateObject();
    cJSON_AddStringToObject(user, "name", name);
    cJSON_AddStringToObject(user, "nfc_id", nfc_id);
    cJSON_AddItemToArray(users, user);
}

void remove_user(const char *nfc_id) {
    if (!users) {
        ESP_LOGE(TAG, "Failed to load users file");
        return;
    }

    cJSON *user_to_remove = get_user(nfc_id);
    if (user_to_remove) {
        uint8_t index = 0;
        cJSON *user = NULL;
        cJSON_ArrayForEach(user, users) {
            const char *nfc_id_user = cJSON_GetObjectItem(user, "nfc_id")->valuestring;
            if (strcmp(nfc_id, nfc_id_user) == 0) {
                cJSON_DeleteItemFromArray(users, index);
                break;
            }
            index++;
        }
    } else {
        ESP_LOGI(TAG, "User not found");
    }
}
