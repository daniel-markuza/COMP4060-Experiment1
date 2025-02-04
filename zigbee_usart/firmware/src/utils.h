#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

// Constants
#define MS_TICKS 48000UL
#define TIMEOUT_MS 500UL
#define LED_FLASH_MS 1000UL

// Common Commands
//  Declare but do not define global variables
extern uint8_t restart[];
extern uint8_t join_own_network[];
extern uint8_t join_existing_network[];
extern uint8_t disassociate[];
extern uint8_t change_channel[];
extern uint8_t network_info[];

// Function declarations
void delayMs(uint32_t milliseconds);
size_t readAllBytesWithTimeout(uint8_t *buffer, size_t maxBufferSize);
void sendCommandAndReadResponse(uint8_t *command, const char *description, uint8_t *readBuffer, size_t bufferSize);
void handleLEDBlink();
void systemInitialize();
void SysTick_Handler(); // Declare SysTick handler
void doInputOutput();
uint32_t getMsCount();

#endif // UTILS_H