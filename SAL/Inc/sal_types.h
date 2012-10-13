/**
 * @file sal_types.h
 *
 * @brief This file contains defines for SAL types.
 *
 * $Id: sal_types.h 21702 2010-04-19 15:17:16Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef SAL_TYPES_H
#define SAL_TYPES_H

/* === INCLUDES ============================================================ */

/* SAL types for security: */
/** Dummy SAL type */
#define NO_SAL                          (0x00)
/** SAL with transceiver based AES via SPI,
  * such as AT86RF231, AT86RF212, etc.
  */
#define AT86RF2xx                       (0x01)
/** SAL with single chip transceiver based AES */
#define ATMEGARF_SAL                    (0x02)
/** AES software implementation */
#define SW_AES_SAL                      (0x03)
/** SAL with ATxmega family based AES */
#define ATXMEGA_SAL                     (0x04)

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SAL_TYPES_H */
/* EOF */
