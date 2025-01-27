#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

// Constants
#define MS_TICKS 48000UL
#define TIMEOUT_MS 500
#define LED_FLASH_MS 1000UL

// Function declarations
void delayMs(uint32_t milliseconds);
size_t readAllBytesWithTimeout(uint8_t *buffer, size_t maxBufferSize);
void sendCommandAndReadResponse(uint8_t *command, const char *description, uint8_t *readBuffer, size_t bufferSize);
void handleLEDBlink();
void systemInitialize();
void SysTick_Handler(); // Declare SysTick handler

#endif // UTILS_H