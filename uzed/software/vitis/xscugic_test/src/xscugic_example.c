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
#define INTC_DEVICE_INT_ID	XPAR_FABRIC_PL_PS_IRQ07_INTR // 0x0E

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

	(*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR)) = 0xff;

	// Setup an assert call back to get some info if we assert.
	Xil_AssertSetCallback(AssertPrint);

	xil_printf("GIC Example Test\r\n");

	// Run the Gic example , specify the Device ID generated in xparameters.h
	Status = ScuGicExample(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GIC Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is an example of how to use the interrupt controller driver
* (XScuGic) and the hardware device.  This function is designed to
* work without any hardware devices to cause interrupts. It may not return
* if the interrupt controller is not properly connected to the processor in
* either software or hardware.
*
* This function relies on the fact that the interrupt controller hardware
* has come out of the reset state such that it will allow interrupts to be
* simulated by the software.
*
* @param	DeviceId is Device ID of the Interrupt Controller Device,
*		typically XPAR_<INTC_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note		None.
*
******************************************************************************/
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

	u8 iPriority, iTrigger;
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&iPriority,&iTrigger);
	xil_printf("iPriority = 0x%x, iTrigger = 0x%x\r\n", iPriority, iTrigger);
	iTrigger = 0x03;
	XScuGic_SetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID, iPriority, iTrigger);
	XScuGic_GetPriorityTriggerType(&InterruptController,INTC_DEVICE_INT_ID,&iPriority,&iTrigger);
	xil_printf("iPriority = 0x%x, iTrigger = 0x%x\r\n", iPriority, iTrigger);


	// Enable the interrupt for the device and then cause (simulate) an interrupt so the handlers will be called
	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);

//	// Simulate the Interrupt
//	Status = XScuGic_SoftwareIntr(&InterruptController,	INTC_DEVICE_INT_ID, XSCUGIC_SPI_CPU0_MASK);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}

	uint32_t loopcount = 0;
	while (1) {
		if (InterruptProcessed) {
			break;
		}
		if (0==(loopcount%10000000)) {
			xil_printf("InterruptController.UnhandledInterrupts = %d\r\n", InterruptController.UnhandledInterrupts);
			xil_printf("InterruptController.IsReady = 0x%x\r\n", InterruptController.IsReady);
			xil_printf("InterruptController.Config->HandlerTable[121] = 0x%x\r\n", InterruptController.Config->HandlerTable[121]);
			xil_printf("DeviceDriverHandler = %p\r\n", DeviceDriverHandler);
		}
		loopcount++;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function connects the interrupt handler of the interrupt controller to
* the processor.  This function is separate to allow it to be customized for
* each application.  Each processor or RTOS may require unique processing to
* connect the interrupt handler.
*
* @param	XScuGicInstancePtr is the instance of the interrupt controller
*		that needs to be worked on.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{

	//* Connect the interrupt controller interrupt handler to the hardware
	//* interrupt handling logic in the ARM processor.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, 
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			XScuGicInstancePtr);

	//* Enable interrupts in the ARM
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function is designed to look like an interrupt handler in a device
* driver. This is typically a 2nd level handler that is called from the
* interrupt controller interrupt handler.  This handler would typically
* perform device specific processing such as reading and writing the registers
* of the device to clear the interrupt condition and pass any data to an
* application using the device driver.  Many drivers already provide this
* handler and the user is not required to create it.
*
* @param	CallbackRef is passed back to the device driver's interrupt
*		handler by the XScuGic driver.  It was given to the XScuGic
*		driver in the XScuGic_Connect() function call.  It is typically
*		a pointer to the device driver instance variable.
*		In this example, we do not care about the callback
*		reference, so we passed it a 0 when connecting the handler to
*		the XScuGic driver and we make no use of it here.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void DeviceDriverHandler(void *CallbackRef)
{
	/*
	 * Indicate the interrupt has been processed using a shared variable
	 */
	InterruptProcessed = TRUE;

	(*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR)) -= 1;
}
