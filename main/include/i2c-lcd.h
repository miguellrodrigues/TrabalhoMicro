#include <stdlib.h>

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (uint8_t cmd);  // send command to the lcd

void lcd_send_data (uint8_t data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);

void lcd_send_custom (char* str, uint8_t pos_x, uint8_t pos_y);

void update_value(char* string, float valor, uint8_t pos_x, uint8_t pos_y);