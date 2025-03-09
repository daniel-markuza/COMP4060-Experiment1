
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

  // Restart the module
  sendCommandAndReadResponse(restart, "restart device", readChars, sizeof(readChars));

  // Join the network
  sendCommandAndReadResponse(join_own_network, "Join own network", readChars, sizeof(readChars));

  // Change to a specific channel
  sendCommandAndReadResponse(change_channel, "Change channel", readChars, sizeof(readChars));

  uint8_t messageToSend[30];
  char voltage[5] = "0000"; // Buffer for 4-digit voltage plus null terminator

  // Send start message with initial timestamp
  uint32_t startTime = getMsCount();

  sprintf((char *)messageToSend, "2_START:%010lu\r", startTime);
  sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars));
  sendCommandAndReadResponse(messageToSend, "Write raw message", readChars, sizeof(readChars));

  printf("Sent start timestamp: %s\r\n", messageToSend);

  // Main loop: Wake, measure, send, sleep
  while (1)
  {
    sendCommandAndReadResponse(enter_command_mode, "Enter command mode", readChars, sizeof(readChars));
    delayMs(1000); // Mode switch delay

    sendCommandAndReadResponse(check_voltage, "Check device voltage", readChars, sizeof(readChars));
    printf("Raw Voltage Response: %s\r\n", readChars);

    // Extract 4-digit voltage using manual parsing
    size_t len = strlen((char *)readChars);
    int found = 0;
    for (size_t i = 0; i <= len - 4; i++)
    {
      if (isdigit(readChars[i]) && isdigit(readChars[i + 1]) &&
          isdigit(readChars[i + 2]) && isdigit(readChars[i + 3]))
      {
        if ((i == 0 || !isdigit(readChars[i - 1])) &&
            (i + 4 == len || !isdigit(readChars[i + 4])))
        {
          strncpy(voltage, (char *)&readChars[i], 4);
          voltage[4] = '\0';
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
    sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars));
    sendCommandAndReadResponse(messageToSend, "Write raw message", readChars, sizeof(readChars));

    printf("Sent voltage message: %s\r\n", messageToSend);

    delayMs(60000); // Wait for 60 seconds before sending again
  }
}
