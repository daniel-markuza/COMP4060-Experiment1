#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "definitions.h" // SYS function prototypes
#include "click_routines/usb_uart/usb_uart.h"
#include <xc.h>
#include "utils.h"

void worker_2_main(void)
{
  systemInitialize();

  uint8_t readChars[100];
  uint8_t messageToSend[30];
  char voltage[5] = "0000"; // Buffer for 4-digit voltage plus null terminator

  // Restart the module
  if (!sendCommandAndReadResponse(restart, "Restart device", readChars, sizeof(readChars)))
  {
    printf("Error: Failed to restart device.\r\n");
    return;
  }

  // Join the network
  if (!sendCommandAndReadResponse(join_own_network, "Join own network", readChars, sizeof(readChars)))
  {
    printf("Error: Failed to join network.\r\n");
    return;
  }

  // Change to a specific channel
  if (!sendCommandAndReadResponse(change_channel, "Change channel", readChars, sizeof(readChars)))
  {
    printf("Error: Failed to change channel.\r\n");
    return;
  }

  // Send start message with initial timestamp
  uint32_t startTime = getMsCount();

  sprintf((char *)messageToSend, "2_START:%010lu\r", startTime);
  if (!sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars)))
  {
    printf("Error: Failed to initiate rdatab for start message.\r\n");
    return;
  }

  if (!sendCommandAndReadResponse(messageToSend, "Write start raw message", readChars, sizeof(readChars)))
  {
    printf("Error: Failed to send start message.\r\n");
    return;
  }

  printf("Sent start timestamp: %s\r\n", messageToSend);

  // Main loop: Wake, measure, send, no sleep
  while (1)
  {
    // printf("Printing net\r\n");
    if (!sendCommandAndReadResponse(network_info, "Enter command mode", readChars, sizeof(readChars)))
    {
      printf("Error: Failed to enter command mode.\r\n");
      continue;
    }

    // Check battery voltage
    if (!sendCommandAndReadResponse(check_voltage, "Check device voltage", readChars, sizeof(readChars)))
    {
      printf("Error: Failed to check device voltage.\r\n");
      continue;
    }

    printf("Raw Voltage Response: %s\r\n", readChars);

    // Extract 4-digit voltage using manual parsing
    size_t len = strlen((char *)readChars);
    int found = 0;
    for (size_t i = 0; i <= len - 4; i++)
    {
      // Check if the next 4 characters are digits
      if (isdigit(readChars[i]) && isdigit(readChars[i + 1]) &&
          isdigit(readChars[i + 2]) && isdigit(readChars[i + 3]))
      {
        // Ensure the 4-digit number is not part of a larger number
        if ((i == 0 || !isdigit(readChars[i - 1])) &&
            (i + 4 == len || !isdigit(readChars[i + 4])))
        {
          strncpy(voltage, (char *)&readChars[i], 4);
          voltage[4] = '\0'; // Null-terminate
          found = 1;
          break;
        }
      }
    }

    if (found)
    {
      printf("Extracted Voltage: %s\r\n", voltage);
    }
    else
    {
      printf("Failed to extract voltage.\r\n");
      strcpy(voltage, "0000");
    }

    // Prepare and send the raw message with voltage and timestamp
    uint32_t currentTime = getMsCount();
    sprintf((char *)messageToSend, "2_T%010luV%s\r", currentTime, voltage);

    if (!sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars)))
    {
      printf("Error: Failed to initiate rdatab for voltage message.\r\n");
      continue;
    }
    
    printf("Sending voltage message: %s\r\n", messageToSend);

    if (!sendCommandAndReadResponse(messageToSend, "Write voltage raw message", readChars, sizeof(readChars)))
    {
      printf("Error: Failed to send voltage message.\r\n");
      continue;
    }

    printf("Sent voltage message: %s\r\n", messageToSend);

    // Non-blocking 60-second wait
    delayMs(1000);
  }
}
