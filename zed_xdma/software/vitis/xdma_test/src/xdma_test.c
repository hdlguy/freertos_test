#include <stdio.h>
#include <stdlib.h>
#include "xil_io.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xscugic.h"

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
// pl_ps interrupt ID[15:0] = 91:84, 68:64, 63:61
#define INTC_DEVICE_INT_ID	84 // 0x0E

int ScuGicExample(u16 DeviceId);
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr);
void DeviceDriverHandler(void *CallbackRef);

XScuGic InterruptController; 	     /* Instance of the Interrupt Controller */
static XScuGic_Config *GicConfig;    /* The configuration parameters of the controller */

volatile static int InterruptProcessed = FALSE;

static void AssertPrint(const char8 *FilenamePtr, s32 LineNumber){
	xil_printf("ASSERT: File Name: %s ", FilenamePtr);
	xil_printf("Line Number: %d\r\n",LineNumber);
}


int main(void)
{
	int Status;

	*((uint32_t *)XPAR_AXI_GPIO_0_BASEADDR) = 0xff;
	*((uint32_t *)XPAR_AXI_GPIO_1_BASEADDR) = 0xffff;

	Xil_AssertSetCallback(AssertPrint);

	xil_printf("GIC Example Test\r\n");

	Status = ScuGicExample(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GIC Example Test\r\n");
	return XST_SUCCESS;
}


int ScuGicExample(u16 DeviceId)
{
	int Status;

	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = XScuGic_SelfTest(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID,
			   (Xil_ExceptionHandler)DeviceDriverHandler,
			   (void *)&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// make the interrupt rising edge triggered.
	u8 priority_val, trigger_val;
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&priority_val,&trigger_val);
	xil_printf("priority = 0x%x, trigger = 0x%x\r\n", priority_val, trigger_val);
	trigger_val = 0x03;
	XScuGic_SetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,priority_val,trigger_val);
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&priority_val,&trigger_val);
	xil_printf("priority = 0x%x, trigger = 0x%x\r\n", priority_val, trigger_val);

	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);

//	// Simulate the Interrupt
//	Status = XScuGic_SoftwareIntr(&InterruptController,	INTC_DEVICE_INT_ID, XSCUGIC_SPI_CPU0_MASK);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}

	while (1) {
		if (InterruptProcessed) {
			break;
		}
	}
	return XST_SUCCESS;
}


int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, 
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			XScuGicInstancePtr);
	Xil_ExceptionEnable();
	return XST_SUCCESS;
}


void DeviceDriverHandler(void *CallbackRef)
{
	InterruptProcessed = TRUE;	
	*((uint32_t *)XPAR_AXI_GPIO_0_BASEADDR) -= 1;
}
