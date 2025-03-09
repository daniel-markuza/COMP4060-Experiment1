#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "definitions.h" // SYS function prototypes
#include "click_routines/usb_uart/usb_uart.h"
#include <xc.h>
#include "utils.h"

void worker_1_main(void)
{
  systemInitialize();
  uint8_t readChars[100];

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

  uint8_t messageToSend[30];
  char voltage[5] = "0000"; // Buffer for 4-digit voltage plus null terminator

  // Send start message with initial timestamp
  uint32_t startTime = getMsCount();

  sprintf((char *)messageToSend, "1_START:%010lu\r", startTime);
  if (!sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars)) ||
      !sendCommandAndReadResponse(messageToSend, "Write raw start message", readChars, sizeof(readChars)))
  {
    printf("Error: Failed to send start message.\r\n");
    return;
  }

  printf("Sent start timestamp: %s\r\n", messageToSend);

  // Main loop: Wake, measure, send, sleep
  while (1)
  {
    if (!sendCommandAndReadResponse(network_info, "Wake device up by checking network info", readChars, sizeof(readChars)))
    {
      printf("Error: Failed to wake device.\r\n");
      continue;
    }

    delayMs(1000); // Ensure device is fully awake

    printf("Awake and checking battery voltage...\r\n");

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

    uint32_t currentTime = getMsCount();
    sprintf((char *)messageToSend, "1_T%010luV%s\r", currentTime, voltage);

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

    // Set power mode back to sleep (mode 3)
    if (!sendCommandAndReadResponse(set_power_mode_3, "Sleep device", readChars, sizeof(readChars)))
    {
      printf("Error: Failed to set device to sleep mode.\r\n");
      continue;
    }

    // Non-blocking 60-second wait
    delayMs(60000);
  }
}