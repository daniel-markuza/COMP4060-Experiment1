#include "utils.h"
#include "stdio.h"
#include "string.h"
#include "definitions.h"
#include "click_routines/usb_uart/usb_uart.h"
#include <ctype.h>

void extract_number_from_read_chars(char *src, char *dest, size_t dest_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dest_size - 1; i++) {
        if (isdigit(src[i])) { // Only copy numeric characters
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0'; // Null terminate the cleaned string
}


#define MS_TICKS 48000UL

// number of millisecond between LED flashes
#define HEARTBEAT    5000UL

// Commands
uint8_t set_power_mode_3[] = "ATS39=0000\r"; // Set power mode to 3 (sleep)
uint8_t wakeup_command[] = "+++\r";          // Command to wake up
uint8_t listen_command[] = "AT+POLL\r";      // Command to listen for messages
uint8_t valid_command_rdatab[] = "AT+RDATAB:";
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
          memset(read_chars, '\0', sizeof(read_chars));
          // Check battery voltage
          usb_uart_USART_Write(check_voltage, strlen((char *)check_voltage));
          while (usb_uart_USART_WriteIsBusy())
             ;
          printf("Checking voltage\r\n");
          //This replys with "OK" instead of actual value randomly, might be related to issue?
          memset(read_chars, '\0', sizeof(read_chars));
          readAllBytesWithTimeout((uint8_t *)&read_chars, sizeof(read_chars));
          printf("%s", read_chars);
          elapsedTime = (getMsCount()-startTime)/60000 + 1;

            char formattedTime[6];
            sprintf(formattedTime, "%04lu", elapsedTime);  // Format elapsed time with leading zeros

            //char space[] = ", ";
            uint8_t messageBuffer[200];  // Buffer for final message
            memset(messageBuffer, '\0', sizeof(messageBuffer));

            char extracted_number[20];  // Buffer for extracted number
    memset(extracted_number, 0, sizeof(extracted_number));

    extract_number_from_read_chars((char *)read_chars, extracted_number, sizeof(extracted_number));

    snprintf((char *)messageBuffer, sizeof(messageBuffer), "%s, %s", formattedTime, extracted_number);



            char sizeStr[10];
            sprintf(sizeStr, "%02zu", strlen((char *)messageBuffer));
            strcat((char *)messageBuffer, "\r");

            uint8_t finalCommand[100];
            memset(finalCommand, 0, sizeof(finalCommand));

            strcpy((char *)finalCommand, "AT+RDATAB:");
            strcat((char *)finalCommand, sizeStr);  // Append message size
            strcat((char *)finalCommand, "\r");    // Append carriage return

            printf("Final Command: %s\n", finalCommand);
            printf("Concatenated Message: %s\n", messageBuffer);

          printf("%d", strlen((char *)messageBuffer));

          usb_uart_USART_Write(finalCommand, strlen((char *)finalCommand));
          while (usb_uart_USART_WriteIsBusy())
              ;

          usb_uart_USART_Write(messageBuffer, strlen((char *)messageBuffer));
          while (usb_uart_USART_WriteIsBusy())
                ;

          printf("Sent heartbeat message\r\n");


          printf("Time elapsed since start (minutes): %ld \r\n", elapsedTime);
        }
    }
}