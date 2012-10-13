/**
 * @file vendor_boardtypes.h
 *
 * @brief vendor-specific PAL board types for AT91SAM7S256
 *
 * This header file contains vendor-specific board types
 * based on AT91SAM7S256.
 *
 * $Id: vendor_boardtypes.h,v 1.1.4.1 2010/09/07 17:38:26 dam Exp $
 *
 * @author
 *      dresden elektronik: http://www.dresden-elektronik.de
 *      Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 * Licensed under dresden elektronik's Limited License Agreement --> deEULA.txt
 */

/* Prevent double inclusion */
#ifndef VENDOR_BOARDTYPES_H
#define VENDOR_BOARDTYPES_H


/* === Macros =============================================================== */

/* Boards for AT86RF212 */
/*
 * USB Radio Stick deRFusb-14E001
 * (http://www.dresden-elektronik.de/shop/prod117.html)
 */
#define deRFusb_14E001                     (0x80)


/* Boards for AT86RF231 */
/*
 * USB Radio Stick deRFusb-24E001
 * (http://www.dresden-elektronik.de/shop/prod118.html)
 */
#define deRFusb_24E001                     (0x81)


#endif  /* VENDOR_BOARDTYPES_H */

/* EOF */
