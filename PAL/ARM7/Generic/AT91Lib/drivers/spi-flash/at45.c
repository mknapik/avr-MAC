/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 * ----------------------------------------------------------------------------
 * Copyright (c) 2006, Atmel Corporation

 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation and/or
 * other materials provided with the distribution.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "at45.h"
#include <board.h>
#include <utility/assert.h>

#include <string.h>

//------------------------------------------------------------------------------
//         Internal definitions
//------------------------------------------------------------------------------

/// Number of dataflash which can be recognized.
#define NUMDATAFLASH    (sizeof(at45Devices) / sizeof(At45Desc))

//------------------------------------------------------------------------------
//         Internal variables
//------------------------------------------------------------------------------

static const At45Desc at45Devices[] = {
    {  512,  264,  9, 0x0C, "AT45DB011B"},
    { 1024,  264,  9, 0x14, "AT45DB021B"},
    { 2048,  264,  9, 0x1C, "AT45DB041B"},
    { 4096,  264,  9, 0x24, "AT45DB081B"},
    { 4096,  528, 10, 0x2C, "AT45DB161B"},
    { 8192,  528, 10, 0x34, "AT45DB321B"},
    { 8192, 1056, 11, 0x3C, "AT45DB642D"},
    {16384, 1056, 11, 0x10, "AT45DB1282"},
    {16384, 2112, 12, 0x18, "AT45DB2562"},
    {32768, 2112, 12, 0x20, "AT45DB5122"}
};

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Initializes an AT45 instance and configures SPI chip select register.
/// Always returns 0.
/// \param pAt45  Pointer to the At45 instance to initialize.
/// \param pSpid  Pointer to the underlying SPI driver.
/// \param spiCs  Chip select value to connect to the At45.
//------------------------------------------------------------------------------
unsigned char AT45_Configure(At45 *pAt45, Spid *pSpid, unsigned char spiCs)
{
    SpidCmd *pCommand;

    // Sanity checks
    ASSERT(pSpid, "AT45_Configure: pSpid is 0.\n\r");
    ASSERT(pAt45, "AT45_Configure: pAt45 is 0.\n\r");

    // Initialize the At45 instance
    pAt45->pSpid = pSpid;
    pAt45->pDesc = 0;
    memset(pAt45->pCmdBuffer, 0, 8);
    pAt45->pSend = NULL;
    pAt45->pBusy = NULL;

    // Initialize the spidCmd structure
    pCommand = &(pAt45->command);
    pCommand->pCmd = pAt45->pCmdBuffer;
    pCommand->callback = 0;
    pCommand->pArgument = 0;
    pCommand->spiCs = spiCs;

    return 0;
}

//------------------------------------------------------------------------------
/// This function returns 1 if the At45 driver is not executing any command;
/// otherwise it returns 0.
/// \param pAt45  Pointer to an At45 instance.
//------------------------------------------------------------------------------
unsigned char AT45_IsBusy(At45 *pAt45)
{
    if (pAt45->pBusy)
    {
        return pAt45->pBusy(pAt45->pSpid);
    }
    return 0;
}

//------------------------------------------------------------------------------
/// Sends a command to the dataflash through the SPI. The command is identified
/// by its command code and the number of bytes to transfer (1 + number of
/// address bytes + number of dummy bytes). If data needs to be received, then
/// a data buffer must be provided.
/// This function does not block; its optional callback will be invoked when
/// the transfer completes.
/// \param pAt45  Pointer to an At45 driver instance.
/// \param cmd  Command code.
/// \param cmdSize  Size of command code + address bytes + dummy bytes.
/// \param pData  Data buffer.
/// \param dataSize  Number of data bytes to send/receive.
/// \param address  Address at which the command is performed if meaningful.
/// \param callback  Optional callback to invoke at end of transfer.
/// \param pArgument  Optional parameter to the callback function.
//------------------------------------------------------------------------------
unsigned char AT45_SendCommand(
    At45 *pAt45,
    unsigned char cmd,
    unsigned char cmdSize,
    unsigned char *pData,
    unsigned int dataSize,
    unsigned int address,
    SpidCallback callback,
    void *pArgument)
{
    SpidCmd *pCommand;
    const At45Desc *pDesc = pAt45->pDesc;
    unsigned int dfAddress = 0;

    // Sanity checks
    ASSERT(pAt45, "AT45_Command: pAt45 is 0.\n\r");
    ASSERT(pDesc || (cmd == AT45_STATUS_READ),
           "AT45_Command: Device has no descriptor, only STATUS_READ command allowed\n\r");

    // Check if the SPI driver is available
    if (AT45_IsBusy(pAt45)) {

        return AT45_ERROR_LOCK;
    }

    // Compute command pattern
    pAt45->pCmdBuffer[0] = cmd;

    // Add address bytes if necessary
    if (cmdSize > 1) {

        ASSERT(pDesc, "AT45_Command: No descriptor for dataflash.\n\r");
        dfAddress =
            ((address / (pDesc->pageSize)) << pDesc->pageOffset)
             + (address % (pDesc->pageSize));

        // Write address bytes
        if (pDesc->pageNumber >= 16384) {

            pAt45->pCmdBuffer[1] = ((dfAddress & 0x0F000000) >> 24);
            pAt45->pCmdBuffer[2] = ((dfAddress & 0x00FF0000) >> 16);
            pAt45->pCmdBuffer[3] = ((dfAddress & 0x0000FF00) >> 8);
            pAt45->pCmdBuffer[4] = ((dfAddress & 0x000000FF) >> 0);
    
            if ((cmd != AT45_CONTINUOUS_READ) && (cmd != AT45_PAGE_READ)) {
    
                cmdSize++;
            }
        }
        else {

            pAt45->pCmdBuffer[1] = ((dfAddress & 0x00FF0000) >> 16);
            pAt45->pCmdBuffer[2] = ((dfAddress & 0x0000FF00) >> 8);
            pAt45->pCmdBuffer[3] = ((dfAddress & 0x000000FF) >> 0);
        }
    }

    // Update the SPI Transfer descriptors
    pCommand = &(pAt45->command);
    pCommand->cmdSize = cmdSize;
    pCommand->pData = pData;
    pCommand->dataSize = dataSize;
    pCommand->callback = callback;
    pCommand->pArgument = pArgument;

    // Send Command and data through the SPI
    if (pAt45->pSend) {
        if (pAt45->pSend(pAt45->pSpid, pCommand)) {
            return AT45_ERROR_SPI;
        }
    }
    else {
       return AT45_ERROR_SPI;
    }

    return 0;
}

//------------------------------------------------------------------------------
/// This function returns the At45Desc structure corresponding to the device
/// connected
/// It automatically initializes pAt45->pDesc field structure.
/// This function shall be called by the application before AT45_SendCommand.
/// Returns 0 if successful; Otherwise, returns AT45_ERROR_LOCK if the At45
/// driver is in use or AT45_ERROR_SPI if there was an error with the SPI driver.
/// \param pAt45  Pointer to an AT45 driver instance.
/// \param status  Device status register value.
//------------------------------------------------------------------------------
const At45Desc * AT45_FindDevice(At45 *pAt45, unsigned char status)
{
    unsigned int i;
    unsigned char id = AT45_STATUS_ID(status);

    // Check if status is all one; in which case, it is assumed that no device
    // is connected
    if (status == 0xFF) {

        return 0;
    }

    // Look in device array
    i = 0;
    pAt45->pDesc = 0;
    while ((i < NUMDATAFLASH) && !(pAt45->pDesc)) {

        if (at45Devices[i].id == id) {

            pAt45->pDesc = &(at45Devices[i]);
        }
        i++;
    }

    return pAt45->pDesc;
}

