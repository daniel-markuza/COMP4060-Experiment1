#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"
#include "definitions.h" // For SYS function prototypes

uint8_t messageBuffer[100]; // Buffer for received messages

#define PATTERN_LENGTH 14 // Length of "RAW:-xxx,hi\r\n"

void processReceivedMessages()
{
    uint8_t tempByte;
    char slidingWindow[PATTERN_LENGTH + 1] = {0}; // Stores last received bytes (safe for "RAW:-XXX,hi\r\n")
    char rssiBuffer[4] = {0};                     // Buffer for extracted RSSI (max 3 digits + null)
    size_t rssiCount = 0;                         // Count of valid RSSI messages

    while (SERCOM4_USART_Read(&tempByte, 1)) // Read one byte at a time
    {
        // Wait until the read operation is complete
        while (SERCOM4_USART_ReadIsBusy())
            ;

        // Shift window left and add new byte
        memmove(slidingWindow, slidingWindow + 1, sizeof(slidingWindow) - 1);
        slidingWindow[sizeof(slidingWindow) - 2] = tempByte; // Store new byte in the second-last position

        // Ensure sliding window is null-terminated correctly
        slidingWindow[sizeof(slidingWindow) - 1] = '\0'; //

        // Look for "RAW:" in the sliding window
        char *rawStart = strstr(slidingWindow, "RAW:");
        if (rawStart != NULL)
        {
            // Find the position of the comma `,` which separates RSSI from the message
            char *commaPos = strchr(rawStart, ',');
            if (commaPos != NULL)
            {
                // Calculate RSSI length (must be between "RAW:-" and `,`)
                int rssiLength = commaPos - (rawStart + 4); // "RAW:-" is 5 chars long

                // Validate RSSI is 2 or 3 digits
                if (rssiLength >= 2 && rssiLength <= 3)
                {
                    strncpy(rssiBuffer, rawStart + 4, rssiLength); // Copy RSSI value
                    rssiBuffer[rssiLength] = '\0';                 // Null terminate

                    // Increment count
                    rssiCount++;

                    // Print RSSI and count
                    printf("Valid RSSI received: %s dBm, Count: %u\r\n", rssiBuffer, rssiCount);
                }
            }
        }
        handleLEDBlink();
    }
}

// Main coordinator function
void coordinator_main(void)
{
    systemInitialize();

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
    }
}