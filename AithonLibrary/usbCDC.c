#include "Aithon.h"

/*
 * Endpoints to be used for USBD1.
 */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1
#define USBD1_INTERRUPT_REQUEST_EP      2

SerialUSBDriver SDU1;

/*
 * USB Device Descriptor.
 */
static const uint8_t _vcom_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0110,        /* bcdUSB (1.1).                    */
                         0x02,          /* bDeviceClass (CDC).              */
                         0x00,          /* bDeviceSubClass.                 */
                         0x00,          /* bDeviceProtocol.                 */
                         0x40,          /* bMaxPacketSize.                  */
                         0x0483,        /* idVendor (ST).                   */
#ifdef _AI_IS_BOOTLOADER
                         0x5741,        /* idProduct.                       */
#else
                         0x5740,        /* idProduct.                       */
#endif
                         0x0200,        /* bcdDevice.                       */
                         1,             /* iManufacturer.                   */
                         2,             /* iProduct.                        */
                         3,             /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor _vcom_device_descriptor = {
  sizeof _vcom_device_descriptor_data,
  _vcom_device_descriptor_data
};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t _vcom_configuration_descriptor_data[67] = {
  /* Configuration Descriptor.*/
  USB_DESC_CONFIGURATION(67,            /* wTotalLength.                    */
                         0x02,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         0,             /* iConfiguration.                  */
                         0xC0,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x01,          /* bNumEndpoints.                   */
                         0x02,          /* bInterfaceClass (Communications
                                           Interface Class, CDC section
                                           4.2).                            */
                         0x02,          /* bInterfaceSubClass (Abstract
                                         Control Model, CDC section 4.3).   */
                         0x01,          /* bInterfaceProtocol (AT commands,
                                           CDC section 4.4).                */
                         0),            /* iInterface.                      */
  /* Header Functional Descriptor (CDC section 5.2.3).*/
  USB_DESC_BYTE         (5),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x00),         /* bDescriptorSubtype (Header
                                           Functional Descriptor.           */
  USB_DESC_BCD          (0x0110),       /* bcdCDC.                          */
  /* Call Management Functional Descriptor. */
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (Call Management
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bmCapabilities (D0+D1).          */
  USB_DESC_BYTE         (0x01),         /* bDataInterface.                  */
  /* ACM Functional Descriptor.*/
  USB_DESC_BYTE         (4),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x02),         /* bDescriptorSubtype (Abstract
                                           Control Management Descriptor).  */
  USB_DESC_BYTE         (0x02),         /* bmCapabilities.                  */
  /* Union Functional Descriptor.*/
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x06),         /* bDescriptorSubtype (Union
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bMasterInterface (Communication
                                           Class Interface).                */
  USB_DESC_BYTE         (0x01),         /* bSlaveInterface0 (Data Class
                                           Interface).                      */
  /* Endpoint 2 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_INTERRUPT_REQUEST_EP|0x80,
                         0x03,          /* bmAttributes (Interrupt).        */
                         0x0008,        /* wMaxPacketSize.                  */
                         0xFF),         /* bInterval.                       */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x01,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0x0A,          /* bInterfaceClass (Data Class
                                           Interface, CDC section 4.5).     */
                         0x00,          /* bInterfaceSubClass (CDC section
                                           4.6).                            */
                         0x00,          /* bInterfaceProtocol (CDC section
                                           4.7).                            */
                         0x00),         /* iInterface.                      */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_DATA_AVAILABLE_EP,       /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
  /* Endpoint 1 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_DATA_REQUEST_EP|0x80,    /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00)          /* bInterval.                       */
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor _vcom_configuration_descriptor = {
  sizeof _vcom_configuration_descriptor_data,
  _vcom_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t _vcom_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t _vcom_string1[] = {
  USB_DESC_BYTE(38),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
  'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
  'c', 0, 's', 0
};

/*
 * Device Description string.
 */
static const uint8_t _vcom_string2[] = {
  USB_DESC_BYTE(56),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'h', 0, 'i', 0, 'b', 0, 'i', 0, 'O', 0, 'S', 0, '/', 0,
  'R', 0, 'T', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0,
  'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0,
  'o', 0, 'r', 0, 't', 0
};

/*
 * Serial Number string.
 */
static const uint8_t _vcom_string3[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor _vcom_strings[] = {
  {sizeof _vcom_string0, _vcom_string0},
  {sizeof _vcom_string1, _vcom_string1},
  {sizeof _vcom_string2, _vcom_string2},
  {sizeof _vcom_string3, _vcom_string3}
};

/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *_get_descriptor(USBDriver *usbp, uint8_t dtype, uint8_t dindex, uint16_t lang)
{
   (void)usbp;
   (void)lang;
   switch (dtype) {
   case USB_DESCRIPTOR_DEVICE:
      return &_vcom_device_descriptor;
   case USB_DESCRIPTOR_CONFIGURATION:
      return &_vcom_configuration_descriptor;
   case USB_DESCRIPTOR_STRING:
   if (dindex < 4)
      return &_vcom_strings[dindex];
   }
   return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config = {
   USB_EP_MODE_TYPE_BULK,
   NULL,
   sduDataTransmitted,
   sduDataReceived,
   0x0040,
   0x0040,
   &ep1instate,
   &ep1outstate,
   2,
   NULL
};

/**
 * @brief   IN EP2 state.
 */
static USBInEndpointState ep2instate;

/**
 * @brief   EP2 initialization structure (IN only).
 */
static const USBEndpointConfig ep2config = {
   USB_EP_MODE_TYPE_INTR,
   NULL,
   sduInterruptTransmitted,
   NULL,
   0x0010,
   0x0000,
   &ep2instate,
   NULL,
   1,
   NULL
};

/*
 * Handles the USB driver global events.
 */
static void _usb_event(USBDriver *usbp, usbevent_t event) {
	switch (event) {
	case USB_EVENT_CONFIGURED:
		chSysLockFromIsr();

		/* Enables the endpoints specified into the configuration.
		Note, this callback is invoked from an ISR so I-Class functions
		must be used.*/
		usbInitEndpointI(usbp, USBD1_DATA_REQUEST_EP, &ep1config);
		usbInitEndpointI(usbp, USBD1_INTERRUPT_REQUEST_EP, &ep2config);

		/* Resetting the state of the CDC subsystem.*/
		sduConfigureHookI(&SDU1);

		chSysUnlockFromIsr();
		return;
	case USB_EVENT_RESET:
	case USB_EVENT_ADDRESS:
	case USB_EVENT_SUSPEND:
	case USB_EVENT_WAKEUP:
	case USB_EVENT_STALLED:
		return;
	}
	return;
}

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig _serusbcfg = {
   &USBD1,
   USBD1_DATA_REQUEST_EP,
   USBD1_DATA_AVAILABLE_EP,
   USBD1_INTERRUPT_REQUEST_EP
};

uint8_t _resetSequence[3] = {0, 2, 3};
uint8_t _resetIndex = 0;
static bool_t _request_hook(USBDriver *usbp) {
   // The uploader program will set both the RTS and DTR bits to tell
   // the board to reset in order to be programmed.
	if (((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS) &&
		(usbp->setup[1] == CDC_SET_CONTROL_LINE_STATE)) {
      uint8_t ctrlState = (usbp->setup[2] & 0x3);
      if (_resetSequence[_resetIndex] == ctrlState)
         _resetIndex++;
      else if (_resetIndex > 0 && _resetSequence[_resetIndex-1] != ctrlState)
         _resetIndex = 0;
      if (_resetIndex == 3)
         chEvtSignal(_aithon_thd, (eventmask_t)1);
	}
	return sduRequestsHook(usbp);
}

/*
 * USB driver configuration.
 */
const USBConfig _usbcfg = {
   _usb_event,
   _get_descriptor,
   _request_hook,
   NULL
};

void _usbcdc_init(void)
{
   sduObjectInit(&SDU1);
   sduStart(&SDU1, &_serusbcfg);
   usbDisconnectBus(_serusbcfg.usbp);
   chThdSleepMilliseconds(10);
   usbStart(_serusbcfg.usbp, &_usbcfg);
   usbConnectBus(_serusbcfg.usbp);
}

void _usb_stop(void)
{
   usbDisconnectBus(_serusbcfg.usbp);
   // delayS(1);
   sduStop(&SDU1);
   usbStop(_serusbcfg.usbp);
}