#include "stdint.h

uint32_t MiniUart_Initialize(void);
void MiniUart_SendString(int8_t* s);
void MiniUart_SendChar(uint8_t c);
uint32_t MiniUart_ReadChar(uint32_t block);
