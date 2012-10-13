/**
 * @file config.h
 *
 * @brief Header file that contains PLATFORM definitions.
 *
 * $Id: config.h,v 1.1.4.1 2010/09/07 17:39:31 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * @name Definitions of the different PLATFORM types.
 *
 * Each PLATFORM provide a different set of resources.
 * To differ between them this macros are defined.
 *
 * Since Radio Controller Board and deRFtoRCB Adapter are identically
 * only RADIO_CONTROLLER_BOARD is defined to refer to Radio Controller
 * Board and deRFtoRCBAdapter.
 *
 * @{
 */
/** Definition of PLATFORM Sensor Terminal Board */
#define SENS_TERM_BOARD          (1)
/** Definition of PLATFORM Radio Controller Board AND deRFtoRCBAdapter */
#define RADIO_CONTROLLER_BOARD   (2)
/** Definition of PLATFORM deRFnode */
#define DE_RF_NODE               (3)
/** @} */

#endif /* CONFIG_H_ */
