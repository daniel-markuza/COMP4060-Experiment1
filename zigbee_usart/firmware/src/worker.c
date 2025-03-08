
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "definitions.h" // SYS function prototypes
#include "click_routines/usb_uart/usb_uart.h"
#include <xc.h>
#include "utils.h"

// Define the AT commands
uint8_t set_power_mode_3[] = "ATS39=0003\r";       // Set power mode to 3 (sleep)
uint8_t wakeup_command[] = "+++\r";                // Command to wake up
uint8_t check_voltage[] = "ATS3D?\r";              // Check battery voltage
uint8_t valid_command_rdatab[] = "AT+RDATAB:11\r"; // 11 hex -> 17 decimal bytes: 10 for timestamp, 4 for voltage, 3 for formatting
uint8_t readChars[100];

void worker_main(void)
{
  systemInitialize();

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

  sprintf((char *)messageToSend, "START:%010lu\r", startTime);
  sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars));
  // usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
  // while (usb_uart_USART_WriteIsBusy())
  //   ;
  sendCommandAndReadResponse(messageToSend, "Write raw message", readChars, sizeof(readChars));

  printf("Sent start timestamp: %s\r\n", messageToSend);

  // Main loop: Wake, measure, send, sleep
  while (1)
  {
    sendCommandAndReadResponse(wakeup_command, "Wake device up", readChars, sizeof(readChars));

    printf("Awake and checking battery voltage...\r\n");

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
    sprintf((char *)messageToSend, "T%010luV%s\r", currentTime, voltage);
    sendCommandAndReadResponse(valid_command_rdatab, "Start rdatab command", readChars, sizeof(readChars));
    // usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
    // while (usb_uart_USART_WriteIsBusy())
    //   ;
    // delayMs(5);
    sendCommandAndReadResponse(messageToSend, "Write raw message", readChars, sizeof(readChars));

    printf("Sent voltage message: %s\r\n", messageToSend);

    // Set power mode back to sleep (mode 3)
    sendCommandAndReadResponse(set_power_mode_3, "Sleep device", readChars, sizeof(readChars));

    delayMs(60000); // Sleep for 60 seconds
  }
}
