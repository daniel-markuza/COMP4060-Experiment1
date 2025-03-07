#include "utils.h"
#include "stdio.h"
#include "string.h"
#include "definitions.h"
#include "click_routines/usb_uart/usb_uart.h"

#define MS_TICKS 48000UL

// number of millisecond between LED flashes
#define HEARTBEAT    60000UL

// Commands
uint8_t set_power_mode_3[] = "ATS39=0000\r"; // Set power mode to 3 (sleep)
uint8_t wakeup_command[] = "+++\r";          // Command to wake up
uint8_t listen_command[] = "AT+POLL\r";      // Command to listen for messages
uint8_t valid_command_rdatab[] = "AT+RDATAB:02\r";
uint8_t valid_message_rdatab[] = "hi\r";
uint8_t check_voltage[] = "ats3d?\r";
char out_string[200];


time_t t;
struct tm *tm_info;


uint8_t messageBuffer[100]; // Buffer for received messages

// Main worker function
void worker_main(void)
{
    systemInitialize();

  uint8_t read_chars[100];
  uint8_t buffer[10];

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

    printf("%s", "here");
    int i = 10;
    printf("%d", i);
    uint32_t startTime = getMsCount();
    uint32_t elapsedTime = getMsCount();

    while(1)
    {
        // wait for an interrupt
        __WFI();


        if (!usb_uart_USART_ReadIsBusy()){
            sprintf(out_string,"%c", read_chars[0]);
            //usb_uart_USART_Write((uint8_t *)out_string,sizeof(out_string));
            printf(out_string);

            // read the next one
            usb_uart_USART_Read((uint8_t *)&read_chars,1);
        }
        SERCOM5_USART_Read(&buffer, 10, true);
        if(buffer[0] != '\0'){
            //printf((char*)&buffer); // comment this out if your serial program does a local echo.
            // send the string to the zigbee one char at a time
            for(int i = 0; i < 10 && buffer[i] != '\0'; i++){
                usb_uart_USART_Write((uint8_t*)&buffer[i],1);
            }
        }

        // handle blinking the light
        if ((getMsCount() % HEARTBEAT) == 0)
        {
          usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
          while (usb_uart_USART_WriteIsBusy())
              ;

          usb_uart_USART_Write(valid_message_rdatab, strlen((char *)valid_message_rdatab));
          while (usb_uart_USART_WriteIsBusy())
                ;

          printf("Sent heartbeat message\r\n");
          // Check battery voltage
          usb_uart_USART_Write(check_voltage, strlen((char *)check_voltage));
          while (usb_uart_USART_WriteIsBusy())
             ;
          printf("Checking voltage\r\n");
          elapsedTime = (getMsCount()-startTime)/60000 + 1;

          printf("Time elapsed since start (minutes): %ld \r\n", elapsedTime);
        }
    }
}