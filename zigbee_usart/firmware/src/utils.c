#include "utils.h"
#include "definitions.h" // For SYS function prototypes
#include <stdio.h>
#include <string.h>
#include <xc.h>

// Global variable for milliseconds counter
volatile uint32_t msCount = 0;

// SysTick Handler for 1ms timer
void SysTick_Handler()
{
    msCount++;
}

// Delay for a specified number of milliseconds
void delayMs(uint32_t milliseconds)
{
    uint32_t start = msCount;
    while ((msCount - start) < milliseconds)
    {
        __WFI(); // Enter low-power mode while waiting
    }
}

// Function to read all bytes with timeout
size_t readAllBytesWithTimeout(uint8_t *buffer, size_t maxBufferSize)
{
    uint8_t tempByte;
    size_t count = 0;
    uint32_t startTime = msCount;

    if (buffer == NULL || maxBufferSize == 0)
    {
        printf("Invalid buffer or size.\r\n");
        return 0;
    }

    while (count < maxBufferSize - 1)
    {
        if (SERCOM4_USART_Read(&tempByte, 1))
        {
            while (SERCOM4_USART_ReadIsBusy())
            {
                if ((msCount - startTime) >= TIMEOUT_MS)
                {
                    buffer[count] = '\0';
                    return count;
                }
            }

            buffer[count] = tempByte;
            count++;
            startTime = msCount; // Reset start time for the next byte
        }
        else
        {
            break;
        }
    }

    buffer[count] = '\0'; // Null-terminate the buffer
    return count;
}

// Send a command and read the response
void sendCommandAndReadResponse(uint8_t *command, const char *description, uint8_t *readBuffer, size_t bufferSize)
{
    memset(readBuffer, '\0', bufferSize);
    usb_uart_USART_Write(command, strlen((char *)command));
    while (usb_uart_USART_WriteIsBusy())
        ;
    delayMs(500);
    size_t numBytesRead = readAllBytesWithTimeout(readBuffer, bufferSize);
    printf("%s: Read %u bytes:\r\n%s\r\n", description, numBytesRead, readBuffer);
}

// Blink the LED
void handleLEDBlink()
{
    if ((msCount % LED_FLASH_MS) == 0)
    {
        PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14; // Toggle the LED
    }
}

// System initialization
void systemInitialize()
{
    // Set up LED output
    PORT_REGS->GROUP[0].PORT_DIRSET = PORT_PA14;
    PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;

    // Sleep when idle
    PM_REGS->PM_SLEEPCFG = PM_SLEEPCFG_SLEEPMODE_IDLE;

    // Configure SysTick
    SysTick_Config(MS_TICKS);

    // Allow ZigBee module to initialize
    delayMs(1000);
    printf("System initialized.\r\n");
}