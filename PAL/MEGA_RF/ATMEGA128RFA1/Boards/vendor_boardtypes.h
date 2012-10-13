/**
 * @file vendor_boardtypes.h
 *
 * @brief vendor-specific PAL board types for ATmega128RFA1
 *
 * This header file contains vendor-specific board types
 * based on ATmega128RFA1.
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

/*
 * any radio module of deRFmega128-series
 * (http://www.dresden-elektronik.de/shop/cat4_32.html)
 * on dresden elektronik RF node/gateway (both to be coming soon,
 * http://www.dresden-elektronik.de/shop/cat4.html?language=en)
 */
#define  deRFmega128_22X00_deRFnode                     (0x80)

/*
 * any radio module of deRFmega128-series
 * (http://www.dresden-elektronik.de/shop/cat4_32.html)
 * onto deRFtoRCB-Adapter
 * (http://www.dresden-elektronik.de/shop/prod126.html)
 */
#define deRFmega128_22X00_deRFtoRCB                     (0x82)

/*
 * any radio module of deRFmega128-series
 * (http://www.dresden-elektronik.de/shop/cat4_32.html)
 * via dresden elektronik RFtoRCB-Adapter
 * (http://www.dresden-elektronik.de/shop/prod126.html)
 * onto dresden elektronik Breakout Board
 * (http://www.dresden-elektronik.de/shop/prod85.html)
 */
#define  deRFmega128_22X00_deRFtoRCB_BREAKOUT_BOARD     (0x84)

/*
 * any radio module of deRFmega128-series
 * (http://www.dresden-elektronik.de/shop/cat4_32.html)
 * via dresden elektronik RFtoRCB-Adapter
 * (http://www.dresden-elektronik.de/shop/prod126.html)
 * onto dresden elektronik Sensor Terminal Board
 * (http://www.dresden-elektronik.de/shop/prod75.html)
 */
#define  deRFmega128_22X00_deRFtoRCB_SENS_TERM_BOARD    (0x86)


#endif  /* VENDOR_BOARDTYPES_H */

/* EOF */
