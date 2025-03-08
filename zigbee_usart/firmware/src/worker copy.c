// #include <stdio.h>
// #include <string.h>
// #include <stdint.h>
// #include <ctype.h>
// #include "definitions.h" // SYS function prototypes
// #include "click_routines/usb_uart/usb_uart.h"
// #include <xc.h>
// #include "utils.h"

// // Define the AT commands
// uint8_t set_power_mode_3[] = "ATS39=0003\r";       // Set power mode to 3 (sleep)
// uint8_t wakeup_command[] = "+++\r";                // Command to wake up
// uint8_t listen_command[] = "AT+POLL\r";            // Command to listen for messages
// uint8_t check_voltage[] = "ATS3D?\r";              // Check battery voltage
// uint8_t valid_command_rdatab[] = "AT+RDATAB:06\r"; // Command to send 6 bytes of raw data

// void worker_main(void)
// {
//   systemInitialize();

//   uint8_t readChars[100];
//   uint8_t messageToSend[50];
//   char voltage[5] = "0000"; // Buffer for 4-digit voltage plus null terminator

//   // Restart the module
//   usb_uart_USART_Write(restart, strlen((char *)restart));
//   while (usb_uart_USART_WriteIsBusy())
//     ;
//   memset(readChars, '\0', sizeof(readChars));
//   readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

//   // Join the network
//   usb_uart_USART_Write(join_own_network, strlen((char *)join_own_network));
//   while (usb_uart_USART_WriteIsBusy())
//     ;
//   memset(readChars, '\0', sizeof(readChars));
//   readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

//   // Change to a specific channel
//   usb_uart_USART_Write(change_channel, strlen((char *)change_channel));
//   while (usb_uart_USART_WriteIsBusy())
//     ;
//   memset(readChars, '\0', sizeof(readChars));
//   readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

//   printf("Initialization complete. Starting main loop.\r\n");

//   // Main loop: Wake, listen, sleep
//   while (1)
//   {
//     // Wake up the device
//     usb_uart_USART_Write(wakeup_command, strlen((char *)wakeup_command));
//     while (usb_uart_USART_WriteIsBusy())
//       ;
//     memset(readChars, '\0', sizeof(readChars));
//     readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

//     printf("Awake and checking battery voltage...\r\n");

//     // Check battery voltage
//     usb_uart_USART_Write(check_voltage, strlen((char *)check_voltage));
//     while (usb_uart_USART_WriteIsBusy())
//       ;
//     memset(readChars, '\0', sizeof(readChars));
//     readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

//     printf("Raw Voltage Response: %s\r\n", readChars);

//     // Extract 4-digit voltage using manual parsing
//     size_t len = strlen((char *)readChars);
//     int found = 0;
//     for (size_t i = 0; i <= len - 4; i++)
//     {
//       // Check if the next 4 characters are digits
//       if (isdigit(readChars[i]) && isdigit(readChars[i + 1]) &&
//           isdigit(readChars[i + 2]) && isdigit(readChars[i + 3]))
//       {
//         // Check boundaries to ensure exactly 4 digits (not part of a larger number)
//         if ((i == 0 || !isdigit(readChars[i - 1])) &&
//             (i + 4 == len || !isdigit(readChars[i + 4])))
//         {
//           strncpy(voltage, (char *)&readChars[i], 4);
//           voltage[4] = '\0'; // Null-terminate
//           found = 1;
//           break;
//         }
//       }
//     }

//     if (found)
//     {
//       printf("Extracted Voltage: %s\r\n", voltage);
//     }
//     else
//     {
//       printf("Failed to extract voltage.\r\n");
//       strcpy(voltage, "0000"); // Default value if extraction fails
//     }

//     // Prepare and send the raw message with voltage
//     sprintf((char *)messageToSend, "V%s\r", voltage); // Send voltage as "VXXXX"
//     usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
//     while (usb_uart_USART_WriteIsBusy())
//       ;
//     usb_uart_USART_Write(messageToSend, strlen((char *)messageToSend));
//     while (usb_uart_USART_WriteIsBusy())
//       ;

//     printf("Sent voltage message: %s\r\n", messageToSend);

//     // Set power mode back to sleep (mode 3)
//     usb_uart_USART_Write(set_power_mode_3, strlen((char *)set_power_mode_3));
//     while (usb_uart_USART_WriteIsBusy())
//       ;
//     memset(readChars, '\0', sizeof(readChars));
//     readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

//     handleLEDBlink(); // Maintain LED state
//     delayMs(5000);    // Sleep for 1 minute
//   }
// }

////////////////////////=======================================///////////////////////

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
uint8_t valid_command_rdatab[] = "AT+RDATAB:17\r"; // 17 bytes: 10 for timestamp, 4 for voltage, 3 for formatting

void worker_main(void)
{
  systemInitialize();

  uint8_t readChars[100];
  uint8_t messageToSend[30];
  char voltage[5] = "0000"; // Buffer for 4-digit voltage plus null terminator

  // Restart the module
  usb_uart_USART_Write(restart, strlen((char *)restart));
  while (usb_uart_USART_WriteIsBusy())
    ;
  memset(readChars, '\0', sizeof(readChars));
  readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

  // Join the network
  usb_uart_USART_Write(join_own_network, strlen((char *)join_own_network));
  while (usb_uart_USART_WriteIsBusy())
    ;
  memset(readChars, '\0', sizeof(readChars));
  readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

  // Change to a specific channel
  usb_uart_USART_Write(change_channel, strlen((char *)change_channel));
  while (usb_uart_USART_WriteIsBusy())
    ;
  memset(readChars, '\0', sizeof(readChars));
  readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

  uint32_t startTime = getMsCount();

  // Send start message with initial timestamp
  sprintf((char *)messageToSend, "START:%010lu\r", startTime);
  usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
  while (usb_uart_USART_WriteIsBusy())
    ;
  usb_uart_USART_Write(messageToSend, 17);
  while (usb_uart_USART_WriteIsBusy())
    ;
  printf("Sent start timestamp: %s\r\n", messageToSend);

  // Main loop: Wake, measure, send, sleep
  while (1)
  {
    // Wake up the device
    usb_uart_USART_Write(wakeup_command, strlen((char *)wakeup_command));
    while (usb_uart_USART_WriteIsBusy())
      ;
    memset(readChars, '\0', sizeof(readChars));
    readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

    printf("Awake and checking battery voltage...\r\n");

    // Check battery voltage
    usb_uart_USART_Write(check_voltage, strlen((char *)check_voltage));
    while (usb_uart_USART_WriteIsBusy())
      ;
    memset(readChars, '\0', sizeof(readChars));
    readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

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

    usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
    while (usb_uart_USART_WriteIsBusy())
      ;

    usb_uart_USART_Write(messageToSend, 17); // Send exactly 17 bytes
    while (usb_uart_USART_WriteIsBusy())
      ;

    printf("Sent voltage message: %s\r\n", messageToSend);

    // Set power mode back to sleep (mode 3)
    usb_uart_USART_Write(set_power_mode_3, strlen((char *)set_power_mode_3));
    while (usb_uart_USART_WriteIsBusy())
      ;
    memset(readChars, '\0', sizeof(readChars));
    readAllBytesWithTimeout((uint8_t *)&readChars, sizeof(readChars));

    handleLEDBlink();
    delayMs(5000); // Sleep for 5 seconds
  }
}
