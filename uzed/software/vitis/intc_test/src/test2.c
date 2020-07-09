#include "xparameters.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"

int IntcExample(u16 DeviceId);
int SetUpInterruptSystem(XIntc *XIntcInstancePtr, u16 DeviceId, u8 pps_device_id);
void DeviceDriverHandler(void *CallbackRef);

static XIntc InterruptController; 
volatile static int InterruptProcessed = FALSE;

int main(void)
{
	int Status;
	
	(*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR)) = 0xffff; // set the PMOD LEDs
	
	u16 DeviceId 		= XPAR_INTC_0_DEVICE_ID; 			// i think this is the ID of the INTC core.
	u8  pps_device_id 	= XPAR_AXI_INTC_0_SYSTEM_PPS_INTR;	// i think this is the ID of the device wired into the INTC core.
	xil_printf("starting interrupt system\n\r");
	Status = SetUpInterruptSystem(&InterruptController, DeviceId, pps_device_id);

	if (Status != XST_SUCCESS) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	while(1) {
		if (InterruptProcessed) {
			break;
		}
	}

	xil_printf("Success: InterruptProcessed = TRUE\r\n");
	return XST_SUCCESS;
}


int SetUpInterruptSystem(XIntc *XIntcInstancePtr, u16 DeviceId, u8 pps_device_id)
{
	int Status;

	Status = XIntc_Initialize(XIntcInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	Status = XIntc_Connect(XIntcInstancePtr, pps_device_id, (XInterruptHandler)DeviceDriverHandler, (void *)0);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	Status = XIntc_Start(XIntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	XIntc_Enable(XIntcInstancePtr, pps_device_id);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, XIntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


void DeviceDriverHandler(void *CallbackRef)
{
	InterruptProcessed = TRUE;
	(*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR))--;  // decrement the PMOD LEDs
}

