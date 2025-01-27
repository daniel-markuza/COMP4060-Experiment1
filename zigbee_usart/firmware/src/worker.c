#include "utils.h"

volatile uint32_t msCount = 0;
volatile size_t validMessageCount = 0;

// Commands
uint8_t restart[] = "ATZ\r";
uint8_t join_existing_network[] = "AT+JN\r";
uint8_t disassociate[] = "AT+DASSL\r";
uint8_t *valid_message = "AT+UCAST:0000=HELLO\r";

uint8_t messageBuffer[100]; // Buffer for received messages

// Main coordinator function
void coordinator_main(void)
{
    systemInitialize();

    // Restart device
    sendCommandAndReadResponse(restart, "Restarted", messageBuffer, sizeof(messageBuffer));

    // Disassociate from any previous networks
    sendCommandAndReadResponse(disassociate, "Disassociated from networks", messageBuffer, sizeof(messageBuffer));

    // Join own network
    sendCommandAndReadResponse(join_existing_network, "Joined network", messageBuffer, sizeof(messageBuffer));

    printf("Listening for messages...\r\n");
    delayMs(500);
    while (1)
    {
        __WFI(); // Wait for interrupt

        // Blast messages
        usb_uart_USART_Write((uint8_t *)valid_message, strlen((char *)valid_message)); // Using UCAST
        while (usb_uart_USART_WriteIsBusy())
            ;

        handleLEDBlink();
    }
}