#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "definitions.h" // SYS function prototypes
#include "click_routines/usb_uart/usb_uart.h"
#include "config/sam_e51_cnano/peripheral/sercom/usart/plib_sercom4_usart.h"
#include <xc.h>

#define MS_TICKS 48000UL
#define LED_FLASH_MS 1000UL
#define MESSAGE_LENGTH 5
#define TIMEOUT_LIMIT 50000
#define TIMEOUT_MS 500
#define VALID_MESSAGE "hello\r\n"

volatile uint32_t msCount = 0;
volatile size_t validMessageCount = 0;

// Commands
uint8_t restart[] = "ATZ\r";
uint8_t join_existing_network[] = "AT+JN\r";
uint8_t disassociate[] = "AT+DASSL\r";

uint8_t messageBuffer[MESSAGE_LENGTH + 1]; // Buffer for received messages

// SysTick Handler for 1ms timer
void SysTick_Handler()
{
    msCount++;
}

// Delay for specified milliseconds
void delayMs(uint32_t milliseconds)
{
    uint32_t start = msCount;
    while ((msCount - start) < milliseconds)
    {
        __WFI();
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

// Initialize the system
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
        PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
    }
}

// Process received messages
void processReceivedMessages()
{
    uint8_t readBuffer[200];
    size_t numBytesRead = readAllBytesWithTimeout(readBuffer, sizeof(readBuffer));

    if (numBytesRead)
    {
        // Remove leading newline if present
        if (readBuffer[0] == '\n')
        {
            memmove(readBuffer, readBuffer + 1, numBytesRead); // Shift the buffer left
            numBytesRead--;                                    // Adjust the count to exclude the newline
        }

        readBuffer[numBytesRead] = '\0'; // Ensure null termination

        // Extract and validate the message part
        const char *messageStart = NULL;

        // Check for UCAST or RAW formats
        if (strstr((char *)readBuffer, "UCAST:") != NULL)
        {
            // Find the '=' symbol which marks the start of the actual message
            messageStart = strchr((char *)readBuffer, '=');
            if (messageStart != NULL)
            {
                messageStart++; // Skip the '=' character
            }
        }
        else if (strstr((char *)readBuffer, "RAW:") != NULL)
        {
            // RAW format, message starts after the last comma
            messageStart = strrchr((char *)readBuffer, ',');
            if (messageStart != NULL)
            {
                messageStart++; // Skip the ',' character
            }
        }

        if (messageStart != NULL)
        {
            // Validate if the extracted message matches "hello"
            if (strcmp(messageStart, VALID_MESSAGE) == 0)
            {
                validMessageCount++;
                printf("Valid message received! : %s\r\nCount: %u\r\n", readBuffer, validMessageCount);
            }
            else
            {
                printf("Invalid message content: %s\r\n", messageStart);
            }
        }
        else
        {
            printf("Invalid message format: %s\r\n", readBuffer);
        }
    }
}

// Main coordinator function
void worker_main(void)
{
    systemInitialize();

    // Restart device
    sendCommandAndReadResponse(restart, "Restarted", messageBuffer, sizeof(messageBuffer));

    // Disassociate from any previous networks
    sendCommandAndReadResponse(disassociate, "Disassociated from networks", messageBuffer, sizeof(messageBuffer));

    // Join own network
    sendCommandAndReadResponse(join_existing_network, "Joined Coordinator network", messageBuffer, sizeof(messageBuffer));

    printf("Listening for messages...\r\n");
    delayMs(500);
    while (1)
    {
        __WFI();

        // Blast messages
        usb_uart_USART_Write((uint8_t *)"AT+UCAST:0000=HELLO\r", strlen("AT+UCAST:0000=HELLO\r")); // using ucast
        // usb_uart_USART_Write((uint8_t *)"AT+RDATAB:05\rHELLO\r", strlen("AT+RDATAB:05\rHELLO\r")); // using rdatab

        while (usb_uart_USART_WriteIsBusy())
            ;
        // Start listening for a message
        if (SERCOM4_USART_Read((void *)messageBuffer, MESSAGE_LENGTH))
        {
            // Wait for the read to complete
            while (SERCOM4_USART_ReadIsBusy())
                ;

            // Null-terminate the received message
            messageBuffer[MESSAGE_LENGTH] = '\0';

            // Print the received message
            printf("Received message: %s\r\n", messageBuffer);
        }

        // handle blinking the light
        handleLEDBlink();
    }
}
