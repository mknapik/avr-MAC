/**
 * @file vendor_boardtypes.h
 *
 * @brief vendor-specific PAL board types for AT91SAM7X512
 *
 * This header file contains vendor-specific board types
 * based on AT91SAM7X512.
 *
 * $Id: vendor_boardtypes.h,v 1.1.4.2 2010/09/09 12:55:54 dam Exp $
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


/*
 * dresden elektronik deRFarm7 radio module 15-series
 * on dresden elektronik RF node/gateway (both to be coming soon,
 * http://www.dresden-elektronik.de/shop/cat4.html?language=en)
 */
#define deRFarm7_15X00_deRFnode                     (0x80)

/*
 * dresden elektronik deRFarm7 radio module 15-series (to be coming soon,
 * http://www.dresden-elektronik.de/shop/cat4.html?language=en)
 * onto dresden elektronik RFtoRCB-Adapter
 * (http://www.dresden-elektronik.de/shop/prod126.html)
 */
#define deRFarm7_15X00_deRFtoRCB                    (0x82)

/*
 * dresden elektronik deRFarm7 radio module 15-series (to be coming soon,
 * http://www.dresden-elektronik.de/shop/cat4.html?language=en)
 * via dresden elektronik RFtoRCB-Adapter
 * (http://www.dresden-elektronik.de/shop/prod126.html)
 * onto dresden elektronik Breakout Board
 * (http://www.dresden-elektronik.de/shop/prod85.html)
 */
#define deRFarm7_15X00_deRFtoRCB_BREAKOUT_BOARD     (0x84)

/*
 * dresden elektronik deRFarm7 radio module 15-series (to be coming soon,
 * http://www.dresden-elektronik.de/shop/cat4.html?language=en)
 * via dresden elektronik RFtoRCB-Adapter
 * (http://www.dresden-elektronik.de/shop/prod126.html)
 * onto dresden elektronik Sensor Terminal Board
 * (http://www.dresden-elektronik.de/shop/prod75.html)
 */
#define deRFarm7_15X00_deRFtoRCB_SENS_TERM_BOARD    (0x86)


#endif  /* VENDOR_BOARDTYPES_H */
/* EOF */
