#include "utils.h"
#include "stdio.h"
#include "string.h"
#include "definitions.h"
#include "click_routines/usb_uart/usb_uart.h"

// Commands
uint8_t valid_message_ucast[] = "AT+UCAST:0000=hi\r";
uint8_t valid_command_rdatab[] = "AT+RDATAB:02\r";
uint8_t valid_message_rdatab[] = "hi\r";
int ucast_flag=0;

uint8_t messageBuffer[100]; // Buffer for received messages

// Main worker function
void worker_main(void)
{
    systemInitialize();

    // Disassociate device
    sendCommandAndReadResponse(disassociate, "Disassociated from previous network", messageBuffer, sizeof(messageBuffer));

    // Restart device
    sendCommandAndReadResponse(restart, "Restarted", messageBuffer, sizeof(messageBuffer));

    // Join existing network
    sendCommandAndReadResponse(join_existing_network, "Joined existing network", messageBuffer, sizeof(messageBuffer));

    uint32_t lastSendTime = getMsCount(); // Track last send time

    int i=1000;
    if(ucast_flag==1)
    {
      while (i > 0)
      {
        	__WFI(); // Wait for interrupt (SysTick will update msCount)

        	if ((getMsCount() - lastSendTime) >= 150) // Ensure precise 50ms timing
        	{
            	usb_uart_USART_Write(valid_message_ucast,17);
        		while(usb_uart_USART_WriteIsBusy())
          			;

            	printf("Sent message %d\r\n", i--);
            	lastSendTime = getMsCount(); // Reset send time
        	}
    	}
    }
    else
    {
      while (i > 0)
      {
        	__WFI(); // Wait for interrupt (SysTick will update msCount)

        	if ((getMsCount() - lastSendTime) >= 150) // Ensure precise 50ms timing
        	{
            	usb_uart_USART_Write(valid_command_rdatab, strlen((char *)valid_command_rdatab));
            	while (usb_uart_USART_WriteIsBusy())
                	;

             	usb_uart_USART_Write(valid_message_rdatab, strlen((char *)valid_message_rdatab));
            	while (usb_uart_USART_WriteIsBusy())
                	;

            	printf("Sent message %d\r\n", i--);
            	lastSendTime = getMsCount(); // Reset send time
        	}
    	}
    }
}