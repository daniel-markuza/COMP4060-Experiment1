#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "definitions.h" // SYS function prototypes
#include "click_routines/usb_uart/usb_uart.h"
#include <xc.h>
#include "utils.h"

// Define the AT commands
uint8_t set_power_mode_3[] = "ATS39=0003\r"; // Set power mode to 3 (sleep)
uint8_t wakeup_command[] = "+++\r";          // Command to wake up
uint8_t listen_command[] = "AT+POLL\r";      // Command to listen for messages
uint8_t check_voltage[] = "ATS3D?\r";        // Check battery voltage

void worker_main(void)
{
  systemInitialize();

  uint8_t read_chars[100];
  //  uint8_t buffer[10];

  // Restart the module
  usb_uart_USART_Write(restart, strlen((char *)restart));
  while (usb_uart_USART_WriteIsBusy())
    ;

  //  usb_uart_USART_Read((uint8_t *)&read_chars, 2);
  //  while (usb_uart_USART_ReadIsBusy())
  //    ;
  memset(read_chars, '\0', sizeof(read_chars));
  readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));

  // Join the network
  usb_uart_USART_Write(join_own_network, strlen((char *)join_own_network));
  while (usb_uart_USART_WriteIsBusy())
    ;

  memset(read_chars, '\0', sizeof(read_chars));
  readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));

  // Change to a specific channel
  usb_uart_USART_Write(change_channel, strlen((char *)change_channel));
  while (usb_uart_USART_WriteIsBusy())
    ;

  memset(read_chars, '\0', sizeof(read_chars));
  readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));

  printf("Let's go\r\n");

  // Main loop: Wake, listen, sleep
  while (1)
  {
    // Wake up the device
    usb_uart_USART_Write(wakeup_command, strlen((char *)wakeup_command));
    while (usb_uart_USART_WriteIsBusy())
      ;
    memset(read_chars, '\0', sizeof(read_chars));
    readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));

    printf("Awake\r\n");

    // Check battery voltage
    usb_uart_USART_Write(check_voltage, strlen((char *)check_voltage));
    while (usb_uart_USART_WriteIsBusy())
      ;
    printf("Checking voltage\r\n");

    memset(read_chars, '\0', sizeof(read_chars));
    readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));

    printf("Battery Voltage: %s\r\n", read_chars);

    // Set power mode back to sleep (mode 3)
    usb_uart_USART_Write(set_power_mode_3, strlen((char *)set_power_mode_3));
    while (usb_uart_USART_WriteIsBusy())
      ;
    memset(read_chars, '\0', sizeof(read_chars));
    readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));

    // Sleep for 1 minute (60000 milliseconds)
    for (int i = 0; i < 60; i++)
    {
      __WFI();          // Wait for interrupt, power-saving
      handleLEDBlink(); // Maintain LED state
    }
  }
}