#include "xparameters.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"

#define INTC_DEVICE_ID		  XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	  XPAR_INTC_0_UARTLITE_0_VEC_ID

int IntcExample(u16 DeviceId);
int SetUpInterruptSystem(XIntc *XIntcInstancePtr);
void DeviceDriverHandler(void *CallbackRef);

static XIntc InterruptController; 
volatile static int InterruptProcessed = FALSE;

int main(void)
{
	int Status;
	
	*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR) = 0x81;   // clear LEDs
	*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR) = 0x8181; // clear LEDs
	
	/* Run the Intc example , specify the Device ID generated in xparameters.h */
	Status = IntcExample(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Intc Example\r\n");
	return XST_SUCCESS;

}


//* @param	DeviceId is Device ID of the Interrupt Controller Device,
//*		typically XPAR_<INTC_instance>_DEVICE_ID value from	xparameters.h.
int IntcExample(u16 DeviceId)
{
	int Status;

	/* Initialize the interrupt controller driver so that it is ready to use. */
	Status = XIntc_Initialize(&InterruptController, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/* Perform a self-test to ensure that the hardware was built correctly. */
	Status = XIntc_SelfTest(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	// Setup the Interrupt System.
	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	
	//  Simulate the Interrupt.
	Status = XIntc_SimulateIntr(&InterruptController, INTC_DEVICE_INT_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	
	// Wait for the interrupt to be processed.
	while (1)
	{
		if (InterruptProcessed) {
			break;
		}
	}
	return XST_SUCCESS;
}

//* This function connects the interrupt handler of the interrupt controller to
//* the processor.  This function is separate to allow it to be customized for
//* each application.  Each processor or RTOS may require unique processing to
//* connect the interrupt handler.
int SetUpInterruptSystem(XIntc *XIntcInstancePtr)
{
	int Status;
	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(XIntcInstancePtr, INTC_DEVICE_INT_ID,
				   (XInterruptHandler)DeviceDriverHandler,
				   (void *)0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specify simulation mode so that
	 * an interrupt can be caused by software rather than a real hardware
	 * interrupt.
	 */
	Status = XIntc_Start(XIntcInstancePtr, XIN_SIMULATION_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Enable the interrupt for the device and then cause (simulate) an
	 * interrupt so the handlers will be called.
	 */
	XIntc_Enable(XIntcInstancePtr, INTC_DEVICE_INT_ID);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				XIntcInstancePtr);


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
*		handler by the XIntc driver.  It was given to the XIntc driver
*		in the XIntc_Connect() function call.  It is typically a pointer
*		to the device driver instance variable if using the Xilinx
*		Level 1 device drivers.  In this example, we do not care about
*		the callback reference, so we passed it a 0 when connecting the
*		handler to the XIntc driver and we make no use of it here.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void DeviceDriverHandler(void *CallbackRef)
{
	InterruptProcessed = TRUE;
	*((uint32_t*)XPAR_AXI_GPIO_0_BASEADDR) = 0x21;
	*((uint32_t*)XPAR_AXI_GPIO_1_BASEADDR) = 0x6543;
}
