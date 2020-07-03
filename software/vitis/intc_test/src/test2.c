#include "xparameters.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"

#define INTC_DEVICE_ID		  XPAR_INTC_0_DEVICE_ID  // i think this is the ID of the INTC core.
#define INTC_DEVICE_INT_ID	  XPAR_AXI_INTC_0_FIT_TIMER_0_INTERRUPT_INTR // i think this is the ID of the device wired into the INTC core.

int IntcExample(u16 DeviceId);
int SetUpInterruptSystem(XIntc *XIntcInstancePtr);
void DeviceDriverHandler(void *CallbackRef);

static XIntc InterruptController; 
volatile static int InterruptProcessed = FALSE;
static uint32_t led_vec;

int main(void)
{
	int Status;
	
	led_vec = 0xffffff;
	*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR) = 0x00ff & (led_vec >> 0);
	*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR) = 0xffff & (led_vec >> 8);
	
	Status = IntcExample(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Intc Example\r\n");
	return XST_SUCCESS;
}


int IntcExample(u16 DeviceId)
{
	int Status;

	Status = XIntc_Initialize(&InterruptController, DeviceId);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) return XST_FAILURE;
	
	while (1) {
		if (InterruptProcessed) break;
	}
	return XST_SUCCESS;
}


int SetUpInterruptSystem(XIntc *XIntcInstancePtr)
{
	int Status;

	Status = XIntc_Connect(XIntcInstancePtr, INTC_DEVICE_INT_ID, (XInterruptHandler)DeviceDriverHandler, (void *)0);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	Status = XIntc_Start(XIntcInstancePtr, XIN_REAL_MODE); // XIN_SIMULATION_MODE);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	XIntc_Enable(XIntcInstancePtr, INTC_DEVICE_INT_ID);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, XIntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


void DeviceDriverHandler(void *CallbackRef)
{
	InterruptProcessed = TRUE;
	led_vec++;
	*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR) = 0x00ff & (led_vec >> 0);
	*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR) = 0xffff & (led_vec >> 8);
}
