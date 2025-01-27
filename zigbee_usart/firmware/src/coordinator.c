#include "utils.h"

#define VALID_MESSAGE "hello\r\n"

volatile uint32_t msCount = 0;
volatile size_t validMessageCount = 0;

// Commands
uint8_t restart[] = "ATZ\r";
uint8_t join_existing_network[] = "AT+EN\r";
uint8_t disassociate[] = "AT+DASSL\r";
uint8_t change_channel[] = "AT+CCHANGE:10\r";

uint8_t messageBuffer[100]; // Buffer for received messages

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
void coordinator_main(void)
{
    systemInitialize();

    // Restart device
    sendCommandAndReadResponse(restart, "Restarted", messageBuffer, sizeof(messageBuffer));

    // Disassociate from any previous networks
    sendCommandAndReadResponse(disassociate, "Disassociated from networks", messageBuffer, sizeof(messageBuffer));

    // Join own network
    sendCommandAndReadResponse(join_existing_network, "Joined network", messageBuffer, sizeof(messageBuffer));

    printf("Listening for messages...\r\n");
    delayMs(500);
    while (1)
    {
        __WFI(); // Wait for interrupt

        processReceivedMessages();
        handleLEDBlink();
    }
}