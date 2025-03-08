#include "utils.h"
#include "definitions.h" // For SYS function prototypes
#include <stdio.h>
#include <string.h>
#include <xc.h>
#include "click_routines/usb_uart/usb_uart.h"

// Global variable for milliseconds counter
volatile uint32_t msCount = 0;

// Commands
uint8_t restart[] = "ATZ\r";
uint8_t join_own_network[] = "AT+EN\r";
// uint8_t join_existing_network[] = "AT+JPAN:20,4ED7\r";
uint8_t join_existing_network[] = "AT+JPAN:20,2EAD\r";
uint8_t network_info[] = "AT+N?\r";
uint8_t disassociate[] = "AT+DASSL\r";
uint8_t change_channel[] = "AT+CCHANGE:14\r";
char out_string[200];
uint8_t io_buffer[10];
uint8_t io_read_chars[100];

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
        ;
}

void doInputOutput()
{
    if (!usb_uart_USART_ReadIsBusy())
    {
        sprintf(out_string, "%c", io_read_chars[0]);
        // usb_uart_USART_Write((uint8_t *)out_string,sizeof(out_string));
        printf(out_string);

        // read the next one
        usb_uart_USART_Read((uint8_t *)&io_read_chars, 1);
    }
    SERCOM5_USART_Read(&io_buffer, 10, true);
    if (io_buffer[0] != '\0')
    {
        // printf((char*)&buffer); // comment this out if your serial program does a local echo.
        //  send the string to the zigbee one char at a time
        for (int i = 0; i < 10 && io_buffer[i] != '\0'; i++)
        {
            usb_uart_USART_Write((uint8_t *)&io_buffer[i], 1);
        }
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
        delayMs(50);
    while (usb_uart_USART_WriteIsBusy())
        ;
    readAllBytesWithTimeout(readBuffer, bufferSize);
        delayMs(50);
    printf("%s:\r\n%s\r\n", description, readBuffer);
    delayMs(50);
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

uint32_t getMsCount()
{
    return msCount;
}