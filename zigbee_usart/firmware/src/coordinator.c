#include "utils.h"
#include "stdio.h"
#include "string.h"
#include "definitions.h" // For SYS function prototypes

// volatile uint32_t msCount = 0;
volatile size_t validMessageCount = 0;

// Commands
// uint8_t restart[] = "ATZ\r";
// uint8_t join_existing_network[] = "AT+EN\r";
// uint8_t disassociate[] = "AT+DASSL\r";
// uint8_t change_channel[] = "AT+CCHANGE:14\r";

uint8_t messageBuffer[100]; // Buffer for received messages
//

//#define VALID_MESSAGE_PATTERN "=hi\r\n"
#define VALID_MESSAGE_PATTERN_UCAST "=hi\r\n"
#define VALID_MESSAGE_PATTERN_RDATAB ",hi\r\n"

#define PATTERN_LENGTH 5 // Length of "=hi\r\n"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PATTERN_LENGTH 10
#define RSSI_LENGTH 5

void processReceivedMessages()
{
    uint8_t tempByte;
    char slidingWindow[PATTERN_LENGTH + RSSI_LENGTH] = {0}; //stores last received bytes
    char rssiBuffer[RSSI_LENGTH] = {0}; //buffer for extracted RSSI
    size_t windowIndex = 0;  //tracks position in the window

    while (SERCOM4_USART_Read(&tempByte, 1)) 
    {
        //wait until the read operation is complete
        while (SERCOM4_USART_ReadIsBusy());

        //shift window left and add the new byte at the end
        memmove(slidingWindow, slidingWindow + 1, PATTERN_LENGTH + RSSI_LENGTH - 1);
        slidingWindow[PATTERN_LENGTH + RSSI_LENGTH - 1] = tempByte;

        //check if the sliding window matches the expected pattern
        if (memcmp(slidingWindow, VALID_MESSAGE_PATTERN_RDATAB, PATTERN_LENGTH) == 0)
        {
            validMessageCount++;
            printf("Valid message received! Count: %u\r\n", validMessageCount);

            //extract RSSI (assume it is the last numeric value in the buffer)
            char *rssiPtr = strrchr(slidingWindow, ',');
            if (rssiPtr && *(rssiPtr + 1) != '\0')
            {
                strncpy(rssiBuffer, rssiPtr + 1, RSSI_LENGTH - 1);
                rssiBuffer[RSSI_LENGTH - 1] = '\0';
                printf("RSSI: %s dBm\r\n", rssiBuffer);
            }

            //clear the sliding window
            memset(slidingWindow, 0, PATTERN_LENGTH + RSSI_LENGTH);
        }
    }
}

// Main coordinator function
void coordinator_main(void)
{
    systemInitialize();

    uint8_t test[] = "ATS12?";
    sendCommandAndReadResponse(test, "testing", messageBuffer, sizeof(messageBuffer));
    
    uint8_t RSSI[] = "ATS10?";
    sendCommandAndReadResponse(RSSI, "testing if RSSI is enabled", messageBuffer, sizeof(messageBuffer));
    
    sprintf(RSSI, "ATS10=<%s OR 0x1000>!", messageBuffer);
    sendCommandAndReadResponse(RSSI, "enable RSSI", messageBuffer, sizeof(messageBuffer));

    // Restart device
    sendCommandAndReadResponse(restart, "Restarted", messageBuffer, sizeof(messageBuffer));

    // Join own network
    sendCommandAndReadResponse(join_own_network, "Joined own network", messageBuffer, sizeof(messageBuffer));

    // Change channel
    sendCommandAndReadResponse(change_channel, "channel changed", messageBuffer, sizeof(messageBuffer));

    // Print network info
    sendCommandAndReadResponse(network_info, "Network info", messageBuffer, sizeof(messageBuffer));

    printf("Listening for messages...\r\n");

    while (1)
    {
        processReceivedMessages();
        //        handleLEDBlink();
    }
}