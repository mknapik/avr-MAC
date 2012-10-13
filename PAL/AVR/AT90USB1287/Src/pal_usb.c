/**
 * @file pal_usb.c
 *
 * @brief PAL USB related functions
 *
 * This file implements USB related transmission and reception
 * functions
 *
 * $Id: pal_usb.c 22978 2010-08-18 08:31:23Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================= */

#include <stdint.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "pal_usb.h"
#include "app_config.h"

#ifdef USB0

#if (DEBUG > 1)
# include <stdio.h>
#endif

#if !defined(USB_VID) || !defined(USB_PID) || !defined(USB_RELEASE) ||\
    !defined(USB_VENDOR_NAME) || !defined(USB_PRODUCT_NAME)
#  error "Define USB_VID, USB_PID, USB_RELEASE, USB_VENDOR_NAME and USB_PRODUCT_NAME in app_config.h"
#endif

/* === Types ================================================================ */

struct bcd
{
    uint8_t lowbyte, highbyte;
};

/*
 * USB-specific structures.  Field names (including their "Hungarian
 * notation") are per USB spec.  See the USB 2.0 spec from
 * http://www.usb.org/ for their actual meaning.
 */

/** \todo handle big-endian in uint16_t */
struct device_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    struct bcd bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    struct bcd bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
};

struct configuration_descriptor_header
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
};

struct header_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    struct bcd bcdCDC;
};

struct union_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface0;
};

struct call_management_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
};

struct abstract_control_management_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
};


struct interface_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
};

struct endpoint_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
};

struct configuration_descriptor
{
    struct configuration_descriptor_header h;
    struct
    {
        struct interface_descriptor i;
        struct header_functional_descriptor hf;
        struct union_functional_descriptor uf;
        struct call_management_functional_descriptor cm;
        struct abstract_control_management_functional_descriptor acm;
        struct endpoint_descriptor ep0;
    } i0;
    struct
    {
        struct interface_descriptor i;
        struct endpoint_descriptor ep0;
        struct endpoint_descriptor ep1;
    } i1;
};

/*
 * String descriptor #0 defines the supported language IDs.
 */
struct string_descriptor0
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wLANGID[1];        /* only one language is supported,
                                 * 0x0409 English (USA) */
};

struct string_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
#if CAN_INITIALIZE_FLEXIBLE_ARRAY_MEMBERS
    wchar_t bString[];
#else
    wchar_t bString[USB_STRING_SIZE];
#endif
};

struct line_coding
{
    uint32_t dwDTERate;         /* baud rate */
    uint8_t bCharFormat;        /* 0 - 1 stop bits, 1 - 1.5 stop bits
                                 * 2 - 2 stop bits */
    uint8_t bParityType;        /* 0 - none, 1 - odd, 2 - even,
                                 * 3 - mark, 4 - space */
    uint8_t bDataBits;          /* 5 through 8, or 16 */
};

/*
 * Standard USB 2.0 requests
 */
enum request_type
{
    GET_STATUS,
    CLEAR_FEATURE,
    RESERVED2,
    SET_FEATURE,
    RESERVED4,
    SET_ADDRESS,
    GET_DESCRIPTOR,
    SET_DESCRIPTOR,
    GET_CONFIGURATION,
    SET_CONFIGURATION,
    GET_INTERFACE,
    SET_INTERFACE,
    SYNCH_FRAME
};

/*
 * CDC/PSTN specific requests; these are class specific so only
 * apply if (bmRequestType & 0x60) == 0x20.
 */
enum cdc_request_type
{
    SEND_ENCAPSULATED_COMMAND = 0x00,
    GET_ENCAPSULATED_RESPONSE,
    SET_COMM_FEATURE,
    GET_COMM_FEATURE,
    CLEAR_COMM_FEATURE,
    /* Reserved: 0x05 .. 0x0F */
    SET_AUX_LINE_STATE = 0x10,
    SET_HOOK_STATE,
    PULSE_SETUP,
    SEND_PULSE,
    SET_PULSE_TIME,
    RING_AUX_JACK,
    /* Reserved: 0x16 .. 0x1F */
    SET_LINE_CODING = 0x20,
    GET_LINE_CODING,
    SET_CONTROL_LINE_STATE,
    SEND_BREAK,
    /* Reserved: 0x24 .. 0x2F */
    SET_RINGER_PARMS = 0x30,
    GET_RINGER_PARMS,
    SET_OPERATION_PARMS,
    GET_OPERATION_PARMS,
    SET_LINE_PARMS,
    GET_LINE_PARMS,
    DIAL_DIGITS,
    SET_UNIT_PARAMETER,
    GET_UNIT_PARAMETER,
    CLEAR_UNIT_PARAMETER,
    GET_PROFILE
    /*
     * The CDC class definition lists more requests pertinent to CDC over
     * Ethernet or ATM networks which are not reproduced here.
     */
};

enum descriptor_type
{
    DEVICE_DESC = 1,
    CONFIGURATION_DESC,
    STRING_DESC,
    INTERFACE_DESC,
    ENDPOINT_DESC,
    DEVICE_QUALIFIER_DESC,
    OTHER_SPEED_CONFIGURATION_DESC,
    INTERFACE_POWER_DESC
};

enum epsize
{
    EP8 = 0,
    EP16,
    EP32,
    EP64,
    EP128,                      /* only EP #1 */
    EP256                       /* only EP #1 */
};

enum eptype
{
    EP_CONTROL,
    EP_ISOCHRONOUS,
    EP_BULK,
    EP_INTERRUPT
};

struct epconf
{
    uint8_t uecfg0x;
    uint8_t uecfg1x;
};


/* === Macros =============================================================== */

/* Endpoint numbers; control endpoint is fixed at EP 0. */
#define TX_EP           0x01
#define RX_EP           0x02
#define INT_EP          0x03

/*
 * Usually the USB RX buffer size is defined in the corresponding app_config.h
 * of each application. In order to have proper flow control and buffer size for
 * this particular USB implementation, the buffer size needs to be defined here
 * specifically with a value larger than 64 octets.
 * Therefore the original define for the USB RX buffer size will be undefined
 * AND a new buffer size will be defined.
 */
#ifdef USB_RX_BUF_SIZE
#undef USB_RX_BUF_SIZE
#endif

#define MAX_USB_RX_BUF_SIZE     128
#if  MAX_USB_RX_BUF_SIZE < 64
        /*
         * Each EP bank buffer size is configured to 64 bytes.
         * Therefore the usb_rx_buf size need to be at least 64 bytes,
         * because if the bank is read it needs to be read entirely.
         */
#  error "buffer must be at least endpoint size big"
#endif


/**
 * USB device descriptor
 */
static FLASH_DECLARE(struct device_descriptor dev_desc) =
{
    .bLength = sizeof(struct device_descriptor),
    .bDescriptorType = DEVICE_DESC,
    .bcdUSB = { 0x10, 0x01 } /* USB 1.10 */,
    .bDeviceClass = 0x02 /* CDC */,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = USB_RELEASE,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,
    .bNumConfigurations = 1
};

/**
 * USB configuration descriptor
 */
static FLASH_DECLARE(struct configuration_descriptor conf_desc) =
{
    .h =
    {
        .bLength = sizeof(struct configuration_descriptor_header),
        .bDescriptorType = CONFIGURATION_DESC,
        .wTotalLength = sizeof(struct configuration_descriptor),
        .bNumInterfaces = 2,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = 0x80 /* bus-powered, bit 7 always set */,
        .bMaxPower = 50 /* default of 100 mA */
    },
    .i0 =
    {
        .i =
        {
            .bLength = sizeof(struct interface_descriptor),
            .bDescriptorType = INTERFACE_DESC,
            .bInterfaceNumber = 0,
            .bAlternateSetting = 0,
            .bNumEndpoints = 1,
            .bInterfaceClass = 0x02 /* communication interface class */,
            .bInterfaceSubClass = 0x02 /* abstract control model */,
            .bInterfaceProtocol = 0x01 /* ITU-T V.250 (AT command set) */,
            .iInterface = 0
        },
        .hf =
        {
            .bFunctionLength = sizeof(struct header_functional_descriptor),
            .bDescriptorType = 0x24 /* CS_INTERFACE */,
            .bDescriptorSubtype = 0x00 /* header functional descriptor */,
            .bcdCDC = { 0x10, 0x01 } /* USB 1.10 */
        },
        .uf =
        {
            .bFunctionLength = sizeof(struct union_functional_descriptor),
            .bDescriptorType = 0x24 /* CS_INTERFACE */,
            .bDescriptorSubtype = 0x06 /* union functional descriptor */,
            .bControlInterface = 0,
            .bSubordinateInterface0 = 1
        },
        .cm =
        {
            .bFunctionLength =
            sizeof(struct call_management_functional_descriptor),
            .bDescriptorType = 0x24 /* CS_INTERFACE */,
            .bDescriptorSubtype = 0x01 /* call management functional
                                        * descriptor */,
            .bmCapabilities = 0x03 /*
                                    * D1: 0 - Device sends/receives
                                    *         call management information
                                    *         only over the Communications
                                    *         Class interface.
                                    *     1 - Device can send/receive
                                    *         call management information
                                    *         over a Data Class interface.
                                    * D0: 0 - Device does not handle
                                    *         call management itself.
                                    *     1 - Device handles call
                                    *         management itself.
                                    */,
            .bDataInterface = 0x01
        },
        .acm =
        {
            .bFunctionLength =
            sizeof(struct abstract_control_management_functional_descriptor),
            .bDescriptorType = 0x24 /* CS_INTERFACE */,
            .bDescriptorSubtype = 0x02 /* abstract control management
                                        * functional descriptor */,
            .bmCapabilities = 0x02 /*
                                    * D3: 1 - Device supports the
                                    *         notification
                                    *         Network_Connection.
                                    * D2: 1 - Device supports the
                                    *         request Send_Break
                                    * D1: 1 - Device supports the
                                    *         request combination of
                                    *         Set_Line_Coding,
                                    *         Set_Control_Line_State,
                                    *         Get_Line_Coding, and the
                                    *         notification Serial_State.
                                    * D0: 1 - Device supports the
                                    *         request combination of
                                    *         Set_Comm_Feature,
                                    *         Clear_Comm_Feature, and
                                    *         Get_Comm_Feature.
                                    */
        },
        .ep0 =
        {
            .bLength = sizeof(struct endpoint_descriptor),
            .bDescriptorType = ENDPOINT_DESC,
            .bEndpointAddress = 0x80 | INT_EP /* direction IN */,
            .bmAttributes = 0x03 /* interrupt */,
            .wMaxPacketSize = 16, /* SERIAL_STATE notification is 10
                                   * bytes long */
            /*
             * Some Linux kernels choke on too small interrupt
             * notification intervals.  They internally decode a
             * bandwidth allocation conflict, but the error code is
             * eventually discarded, and an attempt to open() the
             * device node is answered by just EIO.
             */
            .bInterval = 100 /* ms */,
        }
    },
    .i1 =
    {
        .i =
        {
            .bLength = sizeof(struct interface_descriptor),
            .bDescriptorType = INTERFACE_DESC,
            .bInterfaceNumber = 1,
            .bAlternateSetting = 0,
            .bNumEndpoints = 2,
            .bInterfaceClass = 0x0A /* data interface class */,
            .bInterfaceSubClass = 0x00 /* unused */,
            .bInterfaceProtocol = 0x01 /* unused */,
            .iInterface = 0
        },
        .ep0 =
        {
            .bLength = sizeof(struct endpoint_descriptor),
            .bDescriptorType = ENDPOINT_DESC,
            .bEndpointAddress = 0x80 | TX_EP /* direction IN */,
            .bmAttributes = 0x02 /* bulk */,
            .wMaxPacketSize = 64,
            .bInterval = 0 /* ms */,
        },
        .ep1 =
        {
            .bLength = sizeof(struct endpoint_descriptor),
            .bDescriptorType = ENDPOINT_DESC,
            .bEndpointAddress = RX_EP /* direction OUT */,
            .bmAttributes = 0x02 /* bulk */,
            .wMaxPacketSize = 64,
            .bInterval = 0 /* ms */,
        }
    }
};

/*
 * USB String descriptors
 */

static FLASH_DECLARE(struct string_descriptor0 string_descriptor0) =
{
    .bLength = sizeof(struct string_descriptor0),
    .bDescriptorType = STRING_DESC,
    .wLANGID =
    {
        [0] = 0x0409            /* US English */
    }
};

static FLASH_DECLARE(struct string_descriptor vendor_name) =
{
    .bLength = sizeof(struct string_descriptor) +
        sizeof(USB_VENDOR_NAME) - sizeof(wchar_t),
    .bDescriptorType = STRING_DESC,
    .bString = USB_VENDOR_NAME
};

static FLASH_DECLARE(struct string_descriptor product_name) =
{
    .bLength = sizeof(struct string_descriptor) +
        sizeof(USB_PRODUCT_NAME) - sizeof(wchar_t),
    .bDescriptorType = STRING_DESC,
    .bString = USB_PRODUCT_NAME
};


/*
 * Endpoint configuration table for the AT90USB1287.  These data are
 * used to configure the endpoints through configure_ep() at the end
 * of reset.
 */
static FLASH_DECLARE(struct epconf ep_configuration[]) =
{
    [0] =
    {
        .uecfg0x = (EP_CONTROL << EPTYPE0) | 0 /* type: control, dir: OUT */,
        .uecfg1x = (EP64 << EPSIZE0)
    },

    [TX_EP] =
    {
        .uecfg0x = (EP_BULK << EPTYPE0) | _BV(EPDIR) /* type: bulk, dir: IN */,
        .uecfg1x = (EP64 << EPSIZE0) | _BV(EPBK0)
    },

    [RX_EP] =
    {
        .uecfg0x = (EP_BULK << EPTYPE0) | 0 /* type: bulk, dir: OUT */,
        .uecfg1x = (EP64 << EPSIZE0) | _BV(EPBK0)
    },

    [INT_EP] =
    {
        .uecfg0x = (EP_INTERRUPT << EPTYPE0) | _BV(EPDIR) /* type: interupt,
                                                           * dir: IN */,
        .uecfg1x = (EP16 << EPSIZE0)
    }
};

/* === Globals =============================================================*/

/**
 * USB transmit buffer
 * The buffer size is defined in app_config.h
 */
static uint8_t usb_tx_buf[USB_TX_BUF_SIZE];

/**
 * USB receive buffer
 * The buffer size is defined in app_config.h
 */
static uint8_t usb_rx_buf[MAX_USB_RX_BUF_SIZE];

/**
 * Transmit buffer head
 */
static uint8_t usb_tx_buf_head;

/**
 * Transmit buffer tail
 */
static uint8_t usb_tx_buf_tail;

/**
 * Receive buffer head
 */
static uint8_t usb_rx_buf_head;

/**
 * Receive buffer tail
 */
static uint8_t usb_rx_buf_tail;

/**
 * Number of bytes in transmit buffer
 */
static uint8_t usb_tx_count;

/**
 * Number of bytes in receive buffer
 */
static uint8_t usb_rx_count;

/**
 * Connected to USB
 */
static volatile bool usb_connected;

/**
 * Number of current configuration.  0 when unconfigured by the host,
 * 1 when configured.
 */
static uint8_t current_configuration;

/**
 * Remembered state of the DTR signal, from the most recent
 * SET_CONTROL_LINE_STATE request.
 */
static uint8_t dtr_state;

#if (DEBUG > 1)
#define UART_BAUD 9600ul

static int uart_putchar(char c, FILE *stream);

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, 0, _FDEV_SETUP_WRITE);
#endif  /* (DEBUG > 1) */

/* === Prototypes =========================================================== */


/* === Implementation ======================================================= */

#if (DEBUG > 1)
static void
uart_init(void)
{
#if F_CPU < 2000000UL && defined(U2X1)
  UCSR1A = _BV(U2X1);             /* improve baud rate error by using 2x clk */
  UBRR1L = (F_CPU / (8UL * UART_BAUD)) - 1;
#else
  UBRR1L = (F_CPU / (16UL * UART_BAUD)) - 1;
#endif
  UCSR1B = _BV(TXEN1) | _BV(RXEN1); /* tx/rx enable */
}

static int
uart_putchar(char c, FILE *stream)
{

  if (c == '\a')
    {
      fputs("*ring*\n", stderr);
      return 0;
    }

  if (c == '\n')
    uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;

  return 0;
}
#endif  /* (DEBUG > 1) */

/**
 * @brief Initializes USB port
 *
 * This function initializes the USB port. The receive and transmit interrupt
 * pins are made as input pins.
 */
void sio_usb_init(void)
{
#if (DEBUG > 1)
    stdout = &uart_str;
    uart_init();

    printf("Hello, world!\n");
#endif  /* (DEBUG > 1) */

    /*
     * Start PLL, set up for 8 MHz crystal.
     * This affects the USB macro in the AT90USB1287 only.
     */
    PLLCSR = _BV(PLLP1) | _BV(PLLP0) | _BV(PLLE);
    while ((PLLCSR & _BV(PLOCK)) == 0)
    {
        /* wait for PLL lock */
    }

    /*
     * The "OTG" pad is actually just the Vcc pad, so it always has to
     * be enabled.
     */
    USBCON = _BV(USBE) | _BV(OTGPADE) | _BV(VBUSTE);
    USBCON &= ~_BV(FRZCLK);
    UDIEN = _BV(EORSTE) | _BV(SUSPE);
    UHWCON |= _BV(UVREGE);
    UDCON &= ~_BV(DETACH);

    if (USBSTA & _BV(VBUS))
    {
        usb_connected = true;
    }
    else
    {
        usb_connected = false;
    }
}


/**
 * @brief Copies the transmission data to the specified PAL USB buffer
 *
 * This function copies the transmission data to the PAL USB buffer.
 *
 * @param data Pointer to the buffer where the data to be transmitted is present
 * @param length Number of bytes to be transmitted
 *
 * @return number of bytes transmitted
 */
uint8_t sio_usb_tx(uint8_t *data, uint8_t length)
{
    uint8_t bytes_to_be_written;
    uint8_t size;
    uint8_t back;

    /*
     * Calculate available buffer space
     */
    if (usb_tx_buf_tail >= usb_tx_buf_head)
    {
        size = (USB_TX_BUF_SIZE - 1) - (usb_tx_buf_tail - usb_tx_buf_head);
    }
    else
    {
        size = (usb_tx_buf_head - 1) - usb_tx_buf_tail;
    }

    if (size < length)
    {
        /* Not enough buffer space available. Use the remaining size */
        bytes_to_be_written = size;
    }
    else
    {
        bytes_to_be_written = length;
    }

    /* Remember the number of bytes transmitted. */
    back = bytes_to_be_written;
    usb_tx_count += bytes_to_be_written;

    /* The data is copied to the transmit buffer. */
    while (bytes_to_be_written > 0)
    {
        usb_tx_buf[usb_tx_buf_tail] = *data;

        if ((USB_TX_BUF_SIZE - 1) == usb_tx_buf_tail)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            usb_tx_buf_tail = 0;
        }
        else
        {
            /*
             * Increment the index to point the next character to be
             * inserted.
             */
            usb_tx_buf_tail++;
        }

        bytes_to_be_written--;
        data++;
    }

    /*
     * If a transmission needs to be started, pal_usb_handler() takes
     * care about that once it is run.
     */

    return back;
}


/*
 * @brief Puts a character onto USB FIFO
 *
 * This function transmits a byte over usb.
 */
static void usb_putc(void)
{
    if (usb_tx_count > 0)
    {
        /* The number of bytes to be transmitted is decremented. */
        usb_tx_count--;

        /* Write a byte to the endpoint */
        UEDATX = usb_tx_buf[usb_tx_buf_head];

        if ((USB_TX_BUF_SIZE - 1) == usb_tx_buf_head)
        {
            /* Reached the end of buffer, revert back to beginning of buffer */
            usb_tx_buf_head = 0;
        }
        else
        {
            usb_tx_buf_head++;
        }
    }
}


/**
 * @brief Copies the data received from USB to the user specified location
 *
 * This function copies the data received from USB to the user specified
 * location.
 *
 * @param data pointer to the buffer where the received data is to be stored
 * @param max_length maximum length of data to be received
 *
 * @return actual number of bytes received
 */
uint8_t sio_usb_rx(uint8_t *data, uint8_t max_length)
{
    uint8_t data_received = 0;

    if (usb_rx_count == 0)
    {
        /* USB receive buffer is empty. */
        return 0;
    }

    if (MAX_USB_RX_BUF_SIZE <= usb_rx_count)
    {
         /*
          * Bytes between head and tail are overwritten by new data.
          * The oldest data in buffer is the one to which the tail is
          * pointing. So reading operation should start from the tail.
          */
        usb_rx_buf_head = usb_rx_buf_tail;

        /*
         * This is a buffer overflow case. Byt still only bytes equivalent to
         * full buffer size are useful.
         */
        usb_rx_count = MAX_USB_RX_BUF_SIZE;

        /* Bytes received is more than or equal to buffer. */
        if (MAX_USB_RX_BUF_SIZE <= max_length)
        {
             /*
              * Requested receive length (max_length) is more than the
              * max size of receive buffer, but at max the full
              * buffer can be read.
              */
             max_length = MAX_USB_RX_BUF_SIZE;
        }
    }
    else
    {
        /* Bytes received is less than receive buffer maximum length. */
        if (max_length > usb_rx_count)
        {
            /*
             * Requested receive length (max_length) is more than the data
             * present in receive buffer. Hence only the number of bytes
             * present in receive buffer are read.
             */
            max_length = usb_rx_count;
        }
    }

    data_received = max_length;

    while (max_length > 0)
    {
        /* Start to copy from head. */
        *data = usb_rx_buf[usb_rx_buf_head];

        if (++usb_rx_buf_head == MAX_USB_RX_BUF_SIZE)
        {
            usb_rx_buf_head = 0;
        }

        usb_rx_count--;
        data++;
        max_length--;
    }
    return data_received;
}


/*
 * @brief Gets a byte from USB FIFO
 *
 * This function receives a byte from usb.
 */
static void usb_getc(void)
{

    /*
     * Characters can only be read from USB chip, if there is still
     * space in receive buffer.
     */
    if (usb_rx_count < MAX_USB_RX_BUF_SIZE)
    {
        /* The count of characters present in receive buffer is incremented. */
        usb_rx_count++;

        /* Read a character from the endpoint. */
        usb_rx_buf[usb_rx_buf_tail] = UEDATX;
        if ((MAX_USB_RX_BUF_SIZE - 1) == usb_rx_buf_tail)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            usb_rx_buf_tail = 0x00;
        }
        else
        {
            usb_rx_buf_tail++;
        }
    }
}

/**
 * Read one 16-bit word from the current endpoint.
 */
static uint16_t usb_readw(void)
{
    uint16_t w;

    w = UEDATX;
    w |= (uint16_t)UEDATX << 8;

    return w;
}

/**
 * Configure endpoint \c ep.
 */
static bool configure_ep(uint8_t ep)
{
    struct epconf epc;

    PGM_READ_BLOCK(&epc, &ep_configuration[ep], sizeof(struct epconf));

    UENUM = ep;
    UECONX = _BV(EPEN);
    UECFG0X = epc.uecfg0x;
    UECFG1X = epc.uecfg1x | _BV(ALLOC);

    return (UESTA0X & _BV(CFGOK)) != 0;
}

/**
 * Process a standard USB 2.0 setup message.
 */
static bool process_standard_request(uint8_t bmRequestType,
                                     uint8_t bRequest,
                                     uint16_t wValue,
                                     uint16_t wIndex,
                                     uint16_t wLength)
{
    FLASH_STRING_T descriptor;
    size_t dsize;
    bool send_zlp;
    uint8_t i;
#if (DEBUG > 1)
    char *name;
#endif

    switch ((enum request_type)bRequest)
    {
    case CLEAR_FEATURE:
    case SET_FEATURE:
        if ((bmRequestType == 2) && (wValue == 0 /* ENDPOINT_HALT feature */)
            && (wIndex <= INT_EP))
        {
            UEINTX &= ~_BV(RXSTPI);
            UENUM = (uint8_t)wIndex;

            if (bRequest == CLEAR_FEATURE)
            {
                if ((UECONX & _BV(EPEN)) == 0)
                {
                    /* endpoint was disabled, re-enable */
#if (DEBUG > 1)
                    printf("CLEAR_FEATURE(ENDPOINT_HALT): enabling EP %d\n",
                           wIndex);
#endif
                    UECONX |= _BV(EPEN);
                }
            }
            else                /* SET_FEATURE */
            {
                if ((UECONX & _BV(EPEN)) != 0)
                {
                    /* endpoint was enabled, disable */
#if (DEBUG > 1)
                    printf("SET_FEATURE(ENDPOINT_HALT): disabling EP %d\n",
                           wIndex);
#endif
                    UECONX &= ~_BV(EPEN);
                }
            }
            break;
        }
#if (DEBUG > 1)
        printf("Received unsupported %c %s feature, feature selector %d, wIndex %d\n",
               "DIE"[bmRequestType],
               (bRequest == CLEAR_FEATURE? "clear": "set"),
               wValue, wIndex);
#endif
        return false;

    case GET_CONFIGURATION:
        if (bmRequestType != 0x80)
        {
            return false;
        }
#if (DEBUG > 1)
        printf("Received get configuration\n");
#endif
        UEINTX &= ~_BV(RXSTPI);
        UEDATX = current_configuration;
        UEINTX &= ~_BV(TXINI);
        UEINTX &= ~_BV(FIFOCON);
        while ((UEINTX & _BV(RXOUTI)) == 0)
            /* wait */;
        UEINTX &= ~_BV(RXOUTI);
        UEINTX &= ~_BV(FIFOCON);
        break;

    case SET_CONFIGURATION:
        if (bmRequestType != 0x00)
        {
            return false;
        }
        if ((wValue != 0x00 /* return to address state */) &&
            (wValue != 0x01 /* only supported configuration */))
        {
            return false;
        }
#if (DEBUG > 1)
        printf("Received set configuration %d\n", wValue);
#endif
        current_configuration = wValue;
        UEINTX &= ~_BV(RXSTPI);
        /* Send a zero-length packet in response. */
        UEINTX &= ~_BV(TXINI);
        break;

    case GET_DESCRIPTOR:
        if (bmRequestType != 0x80)
        {
            return false;
        }
        switch (wValue >> 8)    /* descriptor type */
        {
        case DEVICE_DESC:
            descriptor = (FLASH_STRING_T)&dev_desc;
            dsize = sizeof(dev_desc);
            if (wIndex != 0)
            {
                return false;
            }
#if (DEBUG > 1)
            printf("Received get device descriptor, returning %d bytes\n",
                   dsize);
#endif
            break;

        case CONFIGURATION_DESC:
            descriptor = (FLASH_STRING_T)&conf_desc;
            dsize = sizeof(conf_desc);
            if (wIndex != 0)
            {
                return false;
            }
#if (DEBUG > 1)
            printf("Received get configuration descriptor, "
                   "returning %d bytes\n", dsize);
#endif
            break;

        case STRING_DESC:
            switch (wValue & 0xFF)
            {
            case 0:
                descriptor = (FLASH_STRING_T)&string_descriptor0;
                break;

            case 1:
                descriptor = (FLASH_STRING_T)&vendor_name;
                break;

            case 2:
                descriptor = (FLASH_STRING_T)&product_name;
                break;

            default:
#if (DEBUG > 1)
            printf("Received unhandled get string descriptor index %d,"
                   " LANGID 0x%04x\n",
                   wValue & 0xFF, wIndex);
#endif
                return false;
            }
            /* Determine actual size by fetching the .bLength field. */
            dsize = PGM_READ_BYTE((const uint8_t *)descriptor);
#if (DEBUG > 1)
            printf("Received get string descriptor index %d, LANGID 0x%04x, "
                   "returning %d bytes\n", wValue & 0xFF, wIndex, dsize);
#endif
            break;

        default:
#if (DEBUG > 1)
            printf("Received unhandled get descriptor request 0x%02x\n",
                   wValue >> 8);
#endif
            return false;
        }
        UEINTX &= ~_BV(RXSTPI);
        if (wLength > dsize)
        {
            /* Limit transfer size to actual descriptor size. */
            wLength = dsize;
        }
        send_zlp = false;
        while (wLength != 0)
        {
            uint8_t amnt = 0;

            while ((UEINTX & _BV(TXINI)) == 0)
                /* wait */;

            while (wLength != 0 && amnt++ < 64)
            {
                char c = PGM_READ_BYTE(descriptor);
                descriptor++;
                UEDATX = (uint8_t)c;

                wLength--;
            }
            UEINTX &= ~_BV(TXINI);
            /*
             * If this packet was exactly the endpoint size, a zero-length
             * packet might be required, unless there is more data to
             * transfer (in which case send_zlp will be assigned to again
             * later).
             */
            send_zlp = amnt == 64;
        }
        if ((UEINTX & _BV(RXOUTI)) != 0)
        {
            /* transfer aborted */
            UEINTX &= ~_BV(RXOUTI);
            UEINTX &= ~_BV(FIFOCON);
            break;
        }
        if (send_zlp)
        {
            while ((UEINTX & _BV(TXINI)) == 0)
                /* wait */;
            UEINTX &= ~_BV(TXINI);
        }
        while ((UEINTX & _BV(RXOUTI)) == 0)
            /* wait */;
        UEINTX &= ~_BV(RXOUTI);
        UEINTX &= ~_BV(FIFOCON);
        break;

    case SET_ADDRESS:
        if (bmRequestType != 0x00)
        {
            return false;
        }
        if (wValue > 127)
        {
            /* invalid address */
            return false;
        }
#if (DEBUG > 1)
        printf("Received set address %d\n", wValue);
#endif
        UDADDR = wValue;
        UEINTX &= ~_BV(RXSTPI);
        /* send zero-length packet in response */
        UEINTX &= ~_BV(TXINI);
        while ((UEINTX & _BV(TXINI)) == 0)
            /* wait */;
        /* Now that this SETUP is dealt with, enable address. */
        UDADDR |= _BV(ADDEN);
        break;

    case GET_STATUS:
        i = 0;
        switch (bmRequestType)
        {
        case 0x80:              /* device status */
            /** \todo add support for self-powered flag */
            if (wIndex != 0)
            {
                return false;
            }
#if (DEBUG > 1)
            name = "device";
#endif
            break;

        case 0x82:              /* endpoint status */
            if (wIndex > INT_EP)
            {
                return false;
            }
            UENUM = (uint8_t)wIndex;
            if ((UECONX & _BV(EPEN)) == 0)
            {
                i = 1;          /* endpoint halted */
            }
#if (DEBUG > 1)
            name = "endpoint";
#endif
            break;

        case 0x81:              /* interface status */
            if (wIndex > 1)
            {
                return false;
            }
#if (DEBUG > 1)
            name = "interface";
#endif
            break;

        default:
            return false;
        }
#if (DEBUG > 1)
        printf("received get %s status, returning 0x%04x\n",
               name, i);
#endif
        UEINTX &= ~_BV(RXSTPI);
        UEDATX = i;
        UEDATX = 0;
        UEINTX &= ~_BV(TXINI);
        while ((UEINTX & _BV(RXOUTI)) == 0)
            /* wait */;
        UEINTX &= ~_BV(RXOUTI);
        UEINTX &= ~_BV(FIFOCON);
        break;

    case SET_INTERFACE:
        if (bmRequestType != 0x01)
        {
            return false;
        }
        if ((wIndex != 0) && (wIndex != 1))
        {
#if (DEBUG > 1)
            printf("Received SET_INTERFACE for a non-existant interface %d\n",
                   wIndex);
#endif
            return false;
        }
        /* There is only a default interface. */
        if (wValue != 0)
        {
#if (DEBUG > 1)
            printf("Received SET_INTERFACE for a interface %d to "
                   "unsupported altsetting %d\n",
                   wIndex, wValue);
#endif
            return false;
        }
        UEINTX &= ~_BV(RXSTPI);
        /* Send a zero-length packet in response. */
        UEINTX &= ~_BV(TXINI);
#if (DEBUG > 1)
        printf("Received SET_INTERFACE, interface %d, altsetting %d\n",
               wIndex, wValue);
#endif
        break;

    default:
#if (DEBUG > 1)
        printf("Received unhandled standard setup request "
               "bmRequestType = 0x%02x, bRequest = 0x%02x, "
               "wValue = 0x%04x, wIndex = 0x%04x, wLength = 0x%04x\n",
               bmRequestType, bRequest, wValue, wIndex, wLength);
#endif
        return false;
    }

    return true;
}


/**
 * Process a CDC class specific setup message.
 */
static bool process_class_request(uint8_t bmRequestType,
                                  uint8_t bRequest,
                                  uint16_t wValue,
                                  uint16_t wLength)
{
#if (DEBUG > 1)
    char b[64];
#endif
    uint8_t i;
    static struct line_coding lcoding =
        {
            /* Just some defaults that make sense. */
            .dwDTERate = 9600,  /** \todo handle big-endian and struct
                                 * padding */
            .bCharFormat = 0,
            .bParityType = 0,
            .bDataBits = 8
        };
    uint8_t *bp;

    switch ((enum cdc_request_type)bRequest)
    {
    case SEND_ENCAPSULATED_COMMAND:
        if (bmRequestType != 0x21)
        {
            return false;
        }
        /*
         * This request is mandatory for an abstract control model device (see
         * USB PSTN, 6.2.2).  It is meant to take modem AT-style commands.  It
         * is implemented here, but effectively ignored as there is nothing
         * like a telephone modem attached.
         */
        UEINTX &= ~_BV(RXSTPI);
        for (i = 0; i < 64 && wLength != 0; i++, wLength--)
        {
#if (DEBUG > 1)
            b[i] = UEDATX;
#else
            (void)UEDATX;
#endif
        }
#if (DEBUG > 1)
        printf("Received send encapsulated command, cmd = %s\n",
               b);
#endif
        /* send zero-length packet in response */
        UEINTX &= ~_BV(TXINI);
        while ((UEINTX & _BV(TXINI)) == 0)
            /* wait */;
        /*
         * Construct and transmit a RESPONSE_AVAILABLE notification
         * message to the interrupt endpoint.
         */
        UENUM = INT_EP;
        if (((UEINTX & _BV(RWAL)) == 0) ||
            ((UEINTX & _BV(TXINI)) == 0))
        {
            /*
             * Interrupt endpoint not ready for transmission.  No
             * chance to transmit the notification.
             */
            break;
        }
        UEDATX = 0xA1;          /* bmRequestType = 10100001B */
        UEDATX = 0x01;          /* bNotificationCode = RESPONSE_AVAILABLE */
        UEDATX = 0x00;          /* wValue = 0 */
        UEDATX = 0x00;
        UEDATX = 0x00;          /* wIndex = Interface */
        UEDATX = 0x00;
        UEDATX = 0x00;          /* wLength = 0 */
        UEDATX = 0x00;
        UEINTX &= ~_BV(FIFOCON);
        while ((UEINTX & _BV(TXINI)) == 0)
            /* wait */;
#if (DEBUG > 1)
        printf("Sent RESPONSE_AVAILABLE notification\n");
#endif
        break;

    case GET_ENCAPSULATED_RESPONSE:
        if (bmRequestType != 0xA1)
        {
            return false;
        }
        /*
         * Also mandatory per USB PSTN, 6.2.2.
         */
#if (DEBUG > 1)
        printf("Received get encapsulated response, sending ERROR\n");
#endif
        UEINTX &= ~_BV(RXSTPI);
        UEDATX = 'E';
        UEDATX = 'R';
        UEDATX = 'R';
        UEDATX = 'O';
        UEDATX = 'R';
        UEINTX &= ~_BV(TXINI);
        while ((UEINTX & _BV(TXINI)) == 0)
            /* wait */;
        break;

    case SET_LINE_CODING:
        if ((bmRequestType != 0x21) ||
            (wLength != sizeof(struct line_coding)))
        {
            return false;
        }
        /*
         * Not outright mandatory, but usual CDC host OS drivers will only
         * accept a generic CDC device if bmCapabilities in the abstract
         * control management functional descriptor is set which indicates
         * that set line coding, get line coding, and set control line state
         * are supported.
         *
         * "Line coding" corresponds to the notion of line bit sizes etc.,
         * i.e. the usual "9600,8N1" setup.  Any line coding is silently
         * accepted here and returned upon request, without actually changing
         * anything.
         */
        UEINTX &= ~_BV(RXSTPI);
        while ((UEINTX & _BV(RXOUTI)) == 0)
            /* wait */;
        for (bp = (uint8_t *)&lcoding; wLength != 0; wLength--)
        {
            *bp++ = UEDATX;
        }
        UEINTX &= ~_BV(RXOUTI);
        UEINTX &= ~_BV(TXINI);
        while ((UEINTX & _BV(TXINI)) == 0)
            /* wait */;
        /* Sanity */
        if (lcoding.bParityType > 4)
        {
            lcoding.bParityType = 0;
        }
        if (lcoding.bCharFormat > 2)
        {
            lcoding.bCharFormat = 0;
        }
        if ((lcoding.bDataBits < 5) || (lcoding.bDataBits > 8))
        {
            lcoding.bDataBits = 8;
        }
#if (DEBUG > 1)
        printf("Received set line coding: %" PRIu32 " Bd, %d%c%d%s\n",
               lcoding.dwDTERate,
               lcoding.bDataBits, "NOEMS"[lcoding.bParityType],
               (lcoding.bCharFormat >> 1) + 1,
               (lcoding.bCharFormat & 2)? ".5": "");
#endif
        break;

    case GET_LINE_CODING:
        if ((bmRequestType != 0xA1) ||
            (wLength != sizeof(struct line_coding)))
        {
            return false;
        }
        UEINTX &= ~_BV(RXSTPI);
        for (bp = (uint8_t *)&lcoding; wLength != 0; wLength--)
        {
            UEDATX = *bp++;
        }
        UEINTX &= ~_BV(TXINI);
        UEINTX &= ~_BV(FIFOCON);
        while ((UEINTX & _BV(RXOUTI)) == 0)
            /* wait */;
        UEINTX &= ~_BV(RXOUTI);
        UEINTX &= ~_BV(FIFOCON);
#if (DEBUG > 1)
        printf("Received get line coding\n");
#endif
        break;

    case SET_CONTROL_LINE_STATE:
        if (bmRequestType != 0x21)
        {
            return false;
        }
        /*
         * No data associated, everyting is passed in wValue.
         */
        UEINTX &= ~_BV(RXSTPI);
        /* send zero-length packet in response */
        UEINTX &= ~_BV(TXINI);
        while ((UEINTX & _BV(TXINI)) == 0)
            /* wait */;
#if (DEBUG > 1)
        printf("Received set control line state: %sRTS, %sDTR\n",
               (wValue & 2)? "": "n",
               (wValue & 1)? "": "n");
#endif
        if (dtr_state != (wValue & 1))
        {
            dtr_state = (wValue & 1);
            /*
             * The DTR signal changed state.  Construct and transmit a
             * SERIAL_STATE notification to the interrupt endpoint.
             * This essentially mimics the functionality of a "null
             * modem": DTR is mirrored back to DSR and DCD.
             */
            UENUM = INT_EP;
            if (((UEINTX & _BV(RWAL)) == 0) ||
                ((UEINTX & _BV(TXINI)) == 0))
            {
                /*
                 * Interrupt endpoint not ready for transmission.  No
                 * chance to transmit the notification.
                 */
                break;
            }
            UEDATX = 0xA1;          /* bmRequestType = 10100001B */
            UEDATX = 0x20;          /* bNotificationCode = SERIAL_STATE */
            UEDATX = 0x00;          /* wValue = 0 */
            UEDATX = 0x00;
            UEDATX = 0x00;          /* wIndex = Interface */
            UEDATX = 0x00;
            UEDATX = 0x02;          /* wLength = 2 */
            UEDATX = 0x00;
            /*
             * D0: bRxCarrier aka. DCD
             * D1: bTxCarrier aka. DSR
             */
            i = dtr_state? 0x03: 0x00;
            UEDATX = i;
            UEDATX = 0x00;
            UEINTX &= ~_BV(FIFOCON);
            while ((UEINTX & _BV(TXINI)) == 0)
                /* wait */;
#if (DEBUG > 1)
            printf("Sent SERIAL_STATE notification, state = 0x%04x\n",
                   i);
#endif
        }
        break;

    default:
#if (DEBUG > 1)
        printf("Unhandled CDC control request 0x%02x\n",
               bRequest);
#endif
        return false;
    }

    return true;
}


/**
 * Process a control message on endpoint 0.
 */
static bool process_setup_message(void)
{
    /*
     * Standard fields present in all control packets.
     */
    uint8_t bmRequestType = UEDATX;
    uint8_t bRequest = UEDATX;
    uint16_t wValue = usb_readw();
    uint16_t wIndex = usb_readw();
    uint16_t wLength = usb_readw();

    switch (bmRequestType & 0x60)
    {
    case 0x00:
        return process_standard_request(bmRequestType, bRequest,
                                        wValue, wIndex, wLength);

    case 0x20:
        return process_class_request(bmRequestType, bRequest,
                                        wValue, wLength);

    case 0x40:
#if (DEBUG > 1)
        printf("Unhandled vendor request: "
               "bmRequestType = 0x%02x, bRequest = 0x%02x\n",
               bmRequestType, bRequest);
#endif
        return false;

    default:
#if (DEBUG > 1)
        printf("Reserved request: "
               "bmRequestType = 0x%02x, bRequest = 0x%02x\n",
               bmRequestType, bRequest);
#endif
        return false;
    }
}


/**
 * @brief Services data transmission or reception on USB
 *
 * This function polls for usb for completion of reception or transmission of
 * a packet on USB.
 */
void usb_handler(void)
{
    bool send_zlp;

    if (!usb_connected)
    {
        return;
    }

    /*
     * Look out for any setup message on the control endpoint.
     */
    UENUM = 0;
    if (UEINTX & _BV(RXSTPI))
    {
        if (!process_setup_message())
        {
            /*
             * This sends a STALL acknowledgement packet.  As this is
             * a control endpoint, the hardware automatically clears
             * the STALLRQ bit afterwards.
             */
#if (DEBUG > 1)
            printf("Stalling on unhandled setup message\n");
#endif
            UECONX |= _BV(STALLRQ);
            UEINTX &= ~_BV(RXSTPI);
        }
    }

    if (current_configuration == 0)
    {
        /* Not yet enumerated, do not attempt to transfer data. */
        return;
    }

    /*
     * Look out for data on the RX (i. e. OUT) endpoint.
     */
    UENUM = RX_EP;
    while (UEINTX & _BV(RXOUTI))
    {
        /*
         * Ensure that the usb_rx_buf gets not overwritten,
         * if it has not been read before.
         */
        if (MAX_USB_RX_BUF_SIZE - usb_rx_count < 64)
        {
            break;
        }
        UEINTX &= ~_BV(RXOUTI);
        while (UEINTX & _BV(RWAL))
        {
            usb_getc();
        }
        UEINTX &= ~_BV(FIFOCON);
#if (DEBUG > 1)
        printf("RX_EP reception handled\n");
#endif
    }

    /*
     * Transmit data, if any, to the TX (i. e. IN) endpoint.
     */
    UENUM = TX_EP;
    send_zlp = false;
    while (usb_tx_count > 0)
    {
        send_zlp = false;
        while ((usb_tx_count > 0) && ((UEINTX & _BV(RWAL)) != 0))
        {
            usb_putc();
        }
        if ((UEINTX & _BV(RWAL)) == 0)
        {
            send_zlp = true;
        }
        UEINTX &= ~_BV(FIFOCON);
#if (DEBUG > 1)
        printf("TX_EP transmission initiated\n");
#endif
    }
    if (send_zlp)
    {
        while ((UEINTX & _BV(RWAL)) == 0)
            /* wait */;
        UEINTX &= ~_BV(FIFOCON);
#if (DEBUG > 1)
        printf("zlp sent\n");
#endif
    }
}

ISR(USB_GEN_vect)
{
#if (DEBUG > 1)
    bool msg_sent = false;
#endif

    if (USBINT & _BV(VBUSTI))
    {
        USBINT &= ~_BV(VBUSTI);
        if (USBSTA & _BV(VBUS))
        {
            /* handle powerup */
            usb_connected = true;

            /*
             * Start PLL, set up for 8 MHz crystal
             */
            PLLCSR = _BV(PLLP1) | _BV(PLLP0) | _BV(PLLE);
            while ((PLLCSR & _BV(PLOCK)) == 0)
            {
                /* wait for PLL lock */
            }

            UDIEN = _BV(EORSTE) | _BV(SUSPE);
            USBCON &= ~_BV(FRZCLK);
            UHWCON |= _BV(UVREGE);
            UDCON &= ~_BV(DETACH);
#if (DEBUG > 1)
            if (!msg_sent)
            {
                msg_sent = true;
                printf("USB_GEN_vect...");
            }
            printf(" Vbus low->high");
#endif
        }
        else
        {
            /* handle powerdown */
            usb_connected = false;

            UDCON |= _BV(DETACH);
            UHWCON &= ~_BV(UVREGE);
            USBCON |= _BV(FRZCLK);
            UDIEN = 0;
#if (DEBUG > 1)
            if (!msg_sent)
            {
                msg_sent = true;
                printf("USB_GEN_vect...");
            }
            printf(" Vbus high->low");
#endif
            PLLCSR = 0;
            dtr_state = 0;
        }
    }
    if ((UDINT & _BV(EORSTI)) && (UDIEN & _BV(EORSTE)))
    {
        UDINT &= ~_BV(EORSTI);
        /* handle end-of-reset */

        (void)configure_ep(0);  /* EP 0: fixed as control endpoint */
        (void)configure_ep(TX_EP);
        (void)configure_ep(RX_EP);
        (void)configure_ep(INT_EP);
#if (DEBUG > 1)
        if (!msg_sent)
        {
            msg_sent = true;
            printf("USB_GEN_vect...");
        }
        printf(" end of reset, endpoints initialized");
#endif
    }
    if ((UDINT & _BV(SUSPI)) && (UDIEN & _BV(SUSPE)))
    {
        UDINT &= ~_BV(SUSPI);
        /* handle suspend */
        USBCON |= _BV(FRZCLK);
        UDIEN = _BV(WAKEUPE);
        PLLCSR = 0;
        dtr_state = 0;
#if (DEBUG > 1)
        printf(" suspended");
#endif
    }
    if ((UDINT & _BV(WAKEUPI)) && (UDIEN & _BV(WAKEUPE)))
    {
        UDINT &= ~_BV(WAKEUPI);
        /* handle wakeup */
        /*
         * Start PLL, set up for 8 MHz crystal
         */
        PLLCSR = _BV(PLLP1) | _BV(PLLP0) | _BV(PLLE);
        while ((PLLCSR & _BV(PLOCK)) == 0)
        {
            /* wait for PLL lock */
        }

        UDIEN = _BV(EORSTE) | _BV(SUSPE);
        USBCON &= ~_BV(FRZCLK);
#if (DEBUG > 1)
        if (!msg_sent)
        {
            msg_sent = true;
            printf("USB_GEN_vect...");
        }
        printf(" woke up");
#endif
    }
#if (DEBUG > 1)
    if (msg_sent)
    {
        putchar('\n');
    }
#endif
}



#endif /* USB0 */

/* EOF */
