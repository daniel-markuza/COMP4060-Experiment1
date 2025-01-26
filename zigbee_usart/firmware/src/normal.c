// /*******************************************************************************
//   USB UART click routine example source file

//   Company
//     Microchip Technology Inc.

//   File Name
//     usb_uart_example.c

//   Summary
//     USB UART click routine example implementation file.

//   Description
//     This file defines the usage of the USB UART click routine APIs.

//   Remarks:
//     None.

//  *******************************************************************************/

// // DOM-IGNORE-BEGIN
// /*
//     (c) 2021 Microchip Technology Inc. and its subsidiaries. You may use this
//     software and any derivatives exclusively with Microchip products.

//     THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
//     EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
//     WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
//     PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
//     WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

//     IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
//     INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
//     WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
//     BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
//     FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
//     ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
//     THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

//     MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
//     TERMS.
// */
// // DOM-IGNORE-END

// /**
//   Section: Included Files
//  */

// #include <stdio.h>
// #include <string.h>
// #include <stdint.h>
// #include "definitions.h" // SYS function prototypes
// #include "click_routines/usb_uart/usb_uart.h"
// #include "config/sam_e51_cnano/peripheral/sercom/usart/plib_sercom4_usart.h"
// #include <xc.h>

// // setup our heartbeat to be 1ms: we overflow at 1ms intervals with a 48MHz clock
// // uses the SysTicks unit so that we get reliable debugging (timer stops on breakpoints)
// // this is a countdown timer that sends an interrupt when 0
// #define MS_TICKS 48000UL

// // number of millisecond between LED flashes
// #define LED_FLASH_MS 1000UL

// // NOTE: this overflows every ~50 days, so I'm not going to care here...
// volatile uint32_t msCount = 0;

// #define MESSAGE_LENGTH 5   // Length of test message "hello"
// #define TIMEOUT_LIMIT 1000 // Arbitrary timeout limit (iterations)

// // Buffer to store received message
// uint8_t messageBuffer[MESSAGE_LENGTH + 1]; // +1 for null termination

// uint8_t restart[] = "ATZ\r";

// uint8_t join_own_network[] = "AT+JN\r";
// uint8_t disassociate[] = "AT+DASSL\r";
// uint8_t change_channel[] = "AT+CCHANGE:10\r"; // that's 10 hex! channels go from 11-26, aka 0B to 1A

// char out_string[200];

// // Fires every 1ms
// void SysTick_Handler()
// {
//     msCount++;
// }

// // Delay for a specified number of milliseconds
// void delay_ms(uint32_t milliseconds)
// {
//     uint32_t targetTime = msCount + milliseconds;
//     while (msCount < targetTime)
//     {
//         __WFI(); // Sleep until the next interrupt
//     }
// }

// size_t readAllBytesWithTimeout(uint8_t *buffer, size_t maxBufferSize)
// {
//     uint8_t tempByte; // Temporary buffer for a single byte
//     size_t count = 0; // Bytes read counter
//     int timeout;      // Timeout counter

//     if (buffer == NULL || maxBufferSize == 0)
//     {
//         return 0; // Invalid buffer or size
//     }

//     while (count < maxBufferSize - 1)
//     { // Leave space for null-termination
//         timeout = 0;

//         // Attempt to read a single byte
//         if (SERCOM4_USART_Read(&tempByte, 1))
//         {
//             // Wait for the read operation to complete with a timeout
//             while (SERCOM4_USART_ReadIsBusy() && (timeout < TIMEOUT_LIMIT))
//             {
//                 __WFI();
//                 timeout++;
//             }

//             // Check if timeout occurred
//             if (timeout >= TIMEOUT_LIMIT)
//             {
//                 break;
//             }

//             // If read was successful, store the byte in the main buffer
//             buffer[count] = tempByte;
//             count++;
//         }
//         else
//         {
//             // I think this is some sort of error state
//             break;
//         }
//     }

//     // Null-terminate the buffer for safety
//     if (count < maxBufferSize)
//     {
//         buffer[count] = '\0';
//     }

//     return count; // Return the total bytes read
// }

// void coordinator_main(void)
// {

//     // LED output
//     PORT_REGS->GROUP[0].PORT_DIRSET = PORT_PA14;
//     PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;

//     // sleep when idling
//     PM_REGS->PM_SLEEPCFG = PM_SLEEPCFG_SLEEPMODE_IDLE;

//     SysTick_Config(MS_TICKS);

//     // sleep for a bit to let the zigbee initialize
//     for (int i = 0; i < 100000; i++)
//         ;
//     printf("booted\r\n");

//     uint8_t read_chars[100] = {0};

//     // Restart device
//     memset(read_chars, '\0', sizeof(read_chars));
//     usb_uart_USART_Write(restart, strlen((char *)restart));
//     while (usb_uart_USART_WriteIsBusy())
//         ;
//     //    size_t numBytesRead = readAllBytesWithTimeout(read_chars, sizeof(read_chars));
//     //    printf("restarted. Read %u bytes:\r\n%s\r\n", numBytesRead, read_chars);

//     // Disassociated from any previously joined network
//     memset(read_chars, '\0', sizeof(read_chars));
//     usb_uart_USART_Write(disassociate, strlen((char *)disassociate));
//     while (usb_uart_USART_WriteIsBusy())
//         ;
//     //    numBytesRead = readAllBytesWithTimeout(read_chars, sizeof(read_chars));
//     //    printf("Disassociated from any previous networks. Read %u bytes:\r\n%s\r\n", numBytesRead, read_chars);

//     // Create network
//     memset(read_chars, '\0', sizeof(read_chars));
//     usb_uart_USART_Write(join_own_network, strlen((char *)join_own_network));
//     while (usb_uart_USART_WriteIsBusy())
//         ;
//     //    numBytesRead = readAllBytesWithTimeout(read_chars, sizeof(read_chars));
//     //    printf("Joined network. Read %u bytes:\r\n%s\r\n", numBytesRead, read_chars);

//     // Change channel
//     //    memset(read_chars, '\0', sizeof(read_chars));
//     //    usb_uart_USART_Write(change_channel, strlen((char *)change_channel));
//     //    while (usb_uart_USART_WriteIsBusy())
//     //        ;
//     //    numBytesRead = readAllBytesWithTimeout(read_chars, sizeof(read_chars));
//     //    printf("Channel changed. Read %u bytes:\r\n%s\r\n", numBytesRead, read_chars);

//     for (int i = 0; i < 100000; i++)
//         ;
//     printf("Listening for messages...\r\n");

//     while (1)
//     {
//         __WFI();

//         // usb_uart_USART_Write((uint8_t *)"AT+UCAST:0000=HELLO\r", strlen("AT+UCAST:0000=HELLO\r"));
//         // while (usb_uart_USART_WriteIsBusy())
//         //     ;
//         // // Start listening for a message
//         // if (SERCOM4_USART_Read((void *)messageBuffer, MESSAGE_LENGTH))
//         // {
//         //     // Wait for the read to complete
//         //     while (SERCOM4_USART_ReadIsBusy())
//         //         ;

//         //     // Null-terminate the received message
//         //     messageBuffer[MESSAGE_LENGTH] = '\0';

//         //     // Print the received message
//         //     printf("Received message: %s\r\n", messageBuffer);
//         // }

//         // handle blinking the light
//         if ((msCount % LED_FLASH_MS) == 0)
//         {
//             PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14; // literally "toggle"
//         }
//     }

//     // while (1)
//     // {
//     // // wait for an interrupt
//     // __WFI();

//     // char response[20] = {0};

//     // // Send the AT command
//     // if (SERCOM4_USART_Write((void *)"AT\r", strlen("AT\r")))
//     // {
//     //     // Wait for the write to complete
//     //     while (SERCOM4_USART_WriteIsBusy())
//     //         ;
//     //     printf("AT command sent\r\n");
//     // }
//     // else
//     // {
//     //     printf("Failed to send AT command.\n");
//     // }

//     // // Read the response from the module
//     // if (SERCOM4_USART_Read((void *)response, 20 - 1))
//     // {
//     //     // Wait for the read to complete
//     //     while (SERCOM4_USART_ReadIsBusy())
//     //         ;

//     //     // Null-terminate the response to make it a valid C string
//     //     response[20 - 1] = '\0';

//     //     // Print the response to the terminal
//     //     printf("Received response: %s\r\n", response);
//     // }
//     // else
//     // {
//     //     printf("Failed to read response.\n");
//     // }

//     // if (!usb_uart_USART_ReadIsBusy())
//     // {
//     //     sprintf(out_string, "%c", read_chars[0]);
//     //     // usb_uart_USART_Write((uint8_t *)out_string,sizeof(out_string));
//     //     printf(out_string);

//     //     // read the next one
//     //     usb_uart_USART_Read((uint8_t *)&read_chars, 1);
//     // }
//     // SERCOM5_USART_Read(&buffer, 10, true);
//     // if (buffer[0] != '\0')
//     // {
//     //     // printf((char*)&buffer); // comment this out if your serial program does a local echo.
//     //     //  send the string to the zigbee one char at a time
//     //     for (int i = 0; i < 10 && buffer[i] != '\0'; i++)
//     //     {
//     //         usb_uart_USART_Write((uint8_t *)&buffer[i], 1);
//     //     }
//     // }

//     // // handle blinking the light
//     // if ((msCount % LED_FLASH_MS) == 0)
//     // {
//     //     PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14; // literally "toggle"
//     // }
//     // }
// }
