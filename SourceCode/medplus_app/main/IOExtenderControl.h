/**
 @file			  IOExtenderControl.h
 @author			  JGS
 @brief			  Motor Control Interface related structure definitions and function prototype declaration
 */

#ifndef IOEC_H_
#define IOEC_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
#define extIO_FanPort 1
#define extIO_FanPin 3

#define extIO_PiezoPort 1
#define extIO_PiezoPin 2

#define extIO_RFIDRSTPort 1
#define extIO_RFIDRSTPin 0

#define extIO_RFIDIRQPort 1
#define extIO_RFIDIRQPin 1

#define extIO_ProxIntPort 1
#define extIO_ProxIntPin 4

#define extIO_OLEDRstPort 1
#define extIO_OLEDRstPin 5

#define extIO_STEPRstPort 1
#define extIO_STEPRstPin 6

#define extIO_STEPSleepPort 1
#define extIO_STEPSleepPin 7

#define extIO_STEPFltPort 2
#define extIO_STEPFltPin 0

#define extIO_STEPDecayPort 2
#define extIO_STEPDecayPin 1

#define extIO_STEPDirPort 2
#define extIO_STEPDirPin 2

#define extIO_STEPEnaPort 2
#define extIO_STEPEnaPin 3

#define extIO_STEPMode0Port 2
#define extIO_STEPMode0Pin 4

#define extIO_STEPMode1Port 2
#define extIO_STEPMode1Pin 5

#define extIO_STEPMode2Port 2
#define extIO_STEPMode2Pin 6

#define extIO_STEPHomePort 2
#define extIO_STEPHomePin 7

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void IOEC_Init(void);
void IOEC_ControlExternalOutput(uint8_t ioPortToSet, uint8_t ioPinToSet,
		uint8_t ioLevelReq);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* IOEC_H_ */
