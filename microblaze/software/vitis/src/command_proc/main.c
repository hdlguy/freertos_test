#include "xparameters.h"
#include "fpga.h"
#include "xstatus.h"
#include "xuartlite_l.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define UARTLITE_BASEADDR	   XPAR_UARTLITE_0_BASEADDR
#define TEST_BUFFER_SIZE 100

typedef enum {reg_wr, reg_rd, spi_wr, spi_rd, led_wr, nop} command_t;

char SendBuffer[TEST_BUFFER_SIZE]; /* Buffer for Transmitting Data */
char RecvBuffer[TEST_BUFFER_SIZE]; /* Buffer for Receiving Data */
char char_temp;

// This function receives the command string and returns a command type and an array of 4 numeric arguments.
void parse_command(char *command_string, command_t *cmd, int *cmd_args)
{
	int i;
	char command[100];
	//char output_string[100];

	*command = '\0';
	cmd_args[0] = cmd_args[1] = cmd_args[2] = cmd_args[3] = 0;
	sscanf(command_string, "%s %x %x %x %x", command, &(cmd_args[0]), &(cmd_args[1]), &(cmd_args[2]), &(cmd_args[3]) );

	if (strcmp("reg_wr",command)==0){
		*cmd = reg_wr;
	} else if (strcmp("reg_rd",command)==0){
		*cmd = reg_rd;
	} else if (strcmp("spi_wr",command)==0){
		*cmd = spi_wr;
	} else if (strcmp("spi_rd",command)==0){
		*cmd = spi_rd;
	} else if (strcmp("led_wr",command)==0){
		*cmd = led_wr;
	} else {
		*cmd = nop;
	}

	// Clear the command string for next time.
	for(i=0;i<TEST_BUFFER_SIZE;i++) command_string[i] = 0;
}


int main(void)
{
	uint32_t Index;
    uint32_t* regptr = (uint32_t *) XPAR_M00_AXI_BASEADDR;
    u32 uart_ptr = XPAR_UARTLITE_0_BASEADDR;
    command_t cmd;
    int cmd_args[4];
    char output_string[100];

	print("Type commands.\n\r");

	Index = 0;
	for(;;){
		char_temp = XUartLite_RecvByte(uart_ptr);  // this waits until there is a character available.
    	//*gpio_ptr = char_temp;  // write character to LED's.
		RecvBuffer[Index] = char_temp;
		if (Index==(TEST_BUFFER_SIZE-1)) Index = 0; else Index++;
		if (char_temp=='\r'){ // carriage return detected
			Index = 0;
			//process the command line.
			parse_command(RecvBuffer, &cmd, cmd_args);

			// for now, just print something to indicate that the command is parsed.
			if (cmd==reg_wr) {
				sprintf(output_string, "command = reg_wr, args = 0x%x 0x%x 0x%x 0x%x\n\r", cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3]);
			} else if (cmd==reg_rd) {
				sprintf(output_string, "command = reg_rd, args = 0x%x 0x%x 0x%x 0x%x\n\r", cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3]);
			} else if (cmd==spi_wr) {
				sprintf(output_string, "command = spi_wr, args = 0x%x 0x%x 0x%x 0x%x\n\r", cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3]);
			} else if (cmd==spi_rd) {
				sprintf(output_string, "command = spi_rd, args = 0x%x 0x%x 0x%x 0x%x\n\r", cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3]);
			} else if (cmd==led_wr) {
				sprintf(output_string, "command = led_wr, args = 0x%x 0x%x 0x%x 0x%x\n\r", cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3]);
				regptr[LED_CONTROL] = cmd_args[0];
			} else {
				sprintf(output_string, "command = nop, args = 0x%x 0x%x 0x%x 0x%x\n\r", cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3]);
			}
			print(output_string);
		}
	}

	return XST_SUCCESS;
}
