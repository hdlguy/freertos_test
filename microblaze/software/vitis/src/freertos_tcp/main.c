#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "fpga.h"
#include "xuartlite_l.h"
#include <stdio.h>
#include <stdlib.h>

#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9


static TaskHandle_t xCommTask;
static QueueHandle_t xQueue = NULL;
char HWstring[15] = "Hello World";

//static BaseType_t write_led ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
static BaseType_t write_led ( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
//static BaseType_t read_led  ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
static BaseType_t read_led  ( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
void vCommandConsoleTask( void *pvParameters );


static const CLI_Command_Definition_t write_led_cmd =
{
    "write_led",
    "write_led <val>: Writes four-bit <val> to the Arty LEDs\n\r",
	write_led,
    1
};


static const CLI_Command_Definition_t read_led_cmd =
{
    "read_led",
    "read_led : Reads the four-bit value assigned to the Arty LEDs\n\r",
	read_led,
    0
};


int main( void )
{
	xil_printf("\n\r");
	xil_printf( "Freertos CLI example main\n\r" );

	FreeRTOS_CLIRegisterCommand( &write_led_cmd );
	FreeRTOS_CLIRegisterCommand( &read_led_cmd );

	xTaskCreate( vCommandConsoleTask, ( const char * ) "COMM", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 0, &xCommTask );

	xQueue = xQueueCreate( 	10, sizeof( HWstring ) );
	configASSERT( xQueue );

	vTaskStartScheduler();

	for( ;; );  // never gets here.
}




BaseType_t write_led( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	int8_t *pcParameter1;
	BaseType_t xParameter1StringLength;
	uint32_t *regptr = (uint32_t *)XPAR_M00_AXI_BASEADDR;

	pcParameter1 = FreeRTOS_CLIGetParameter ((char *)pcCommandString, 1, &xParameter1StringLength);
	pcParameter1[ xParameter1StringLength ] = 0x00; // null terminate

	sprintf((char *)pcWriteBuffer, "%s\r\n", (char *)pcCommandString);
	regptr[LED_CONTROL] = atoi((char *)pcParameter1);

	return(pdFALSE);
}




BaseType_t read_led ( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	uint32_t *regptr = (uint32_t *)XPAR_M00_AXI_BASEADDR;
	uint32_t temp =  (unsigned int)(0x0f & regptr[LED_CONTROL]);
	sprintf((char *)pcWriteBuffer, "led = %d = 0x%02x\n\r", (int)temp, (unsigned int)temp);

	return(pdFALSE);
}



#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

static const int8_t * const pcWelcomeMessage = (int8_t*)"FreeRTOS command server.Type Help to view a list of registered commands.\n\r";

void vCommandConsoleTask( void *pvParameters )
{
	//Peripheral_Descriptor_t xConsole;
	u32 uart_ptr = XPAR_UARTLITE_0_BASEADDR;
	int8_t cRxedChar, cInputIndex = 0;
	BaseType_t xMoreDataToFollow;
	/* The input and output buffers are declared static to keep them off the stack. */
	static int8_t pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];

    /* This code assumes the peripheral being used as the console has already
    been opened and configured, and is passed into the task as the task
    parameter.  Cast the task parameter to the correct type. */
    //xConsole = ( Peripheral_Descriptor_t ) pvParameters;

    /* Send a welcome message to the user knows they are connected. */
    //FreeRTOS_write( xConsole, pcWelcomeMessage, strlen( pcWelcomeMessage ) );
	xil_printf("%s", pcWelcomeMessage);

    for( ;; )
    {
        /* This implementation reads a single character at a time.  Wait in the
        Blocked state until a character is received. */
        //FreeRTOS_read( xConsole, &cRxedChar, sizeof( cRxedChar ) );
    	cRxedChar = XUartLite_RecvByte(uart_ptr);  // this waits until there is a character available.


        if ( ( cRxedChar == '\r' ) || ( cRxedChar == '\n' ))
        {
            /* A newline character was received, so the input command string is
            complete and can be processed.  Transmit a line separator, just to
            make the output easier to read. */
            //FreeRTOS_write( xConsole, "\r\n", strlen( "\r\n" );
        	xil_printf("\r\n");

            /* The command interpreter is called repeatedly until it returns
            pdFALSE.  See the "Implementing a command" documentation for an
            exaplanation of why this is. */
            do
            {
                /* Send the command string to the command interpreter.  Any
                output generated by the command interpreter will be placed in the
                pcOutputString buffer. */
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                              (
                                  (char *)pcInputString,   /* The command string.*/
                                  (char *)pcOutputString,  /* The output buffer. */
                                  MAX_OUTPUT_LENGTH/* The size of the output buffer. */
                              );

                /* Write the output generated by the command interpreter to the
                console. */
                //FreeRTOS_write( xConsole, pcOutputString, strlen( pcOutputString ) );
                xil_printf("%s", pcOutputString);

            } while( xMoreDataToFollow != pdFALSE );

            /* All the strings generated by the input command have been sent.
            Processing of the command is complete.  Clear the input string ready
            to receive the next command. */
            cInputIndex = 0;
            memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
        }
        else
        {
            /* The if() clause performs the processing after a newline character
            is received.  This else clause performs the processing if any other
            character is received. */

            if( cRxedChar == '\r' )
            {
                /* Ignore carriage returns. */
            }
            else if ( cRxedChar == 0x7f )
            {
                /* Backspace was pressed.  Erase the last character in the input
                buffer - if there are any. */
                if( cInputIndex > 0 )
                {
                    cInputIndex--;
                    pcInputString[ cInputIndex ] = ' ';
                }
            }
            else
            {
                /* A character was entered.  It was not a new line, backspace
                or carriage return, so it is accepted as part of the input and
                placed into the input buffer.  When a n is entered the complete
                string will be passed to the command interpreter. */
                if( cInputIndex < MAX_INPUT_LENGTH )
                {
                    pcInputString[ cInputIndex ] = cRxedChar;
                    cInputIndex++;
                }
            }
        }
        xil_printf("%s\r", pcInputString);
    }
}

//// Let's make a task that takes input from the console.
//#define TEST_BUFFER_SIZE 100
//
//void vCommandConsoleTask( void *pvParameters )
//{
//    uint32_t Index;
//    u32 uart_ptr = XPAR_UARTLITE_0_BASEADDR;
//    char char_temp;
//    BaseType_t xMoreDataToFollow;
//    static char pcOutputString[TEST_BUFFER_SIZE], pcInputString[TEST_BUFFER_SIZE];
//
//    print("Type commands.\n\r");
//
//    Index = 0;
//    for(;;){
//        char_temp = XUartLite_RecvByte(uart_ptr);  // this waits until there is a character available.
//        pcInputString[Index] = char_temp;
//        if (Index != (TEST_BUFFER_SIZE-1)) Index++;
//
//        if (char_temp=='\r'){ // carriage return detected
//            Index = 0;
//
//            do {
//                /* Send the command string to the command interpreter.  Any
//                output generated by the command interpreter will be placed in the
//                pcOutputString buffer. */
//                xMoreDataToFollow = FreeRTOS_CLIProcessCommand (pcInputString, pcOutputString, TEST_BUFFER_SIZE);
//
//                xil_printf("%s\n\r", pcOutputString);
//
//            } while( xMoreDataToFollow != pdFALSE );
//
//        }
//    }
//
//}
