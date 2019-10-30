/**
@file			  IOExtenderControl.h
@author			  JGS
@brief			  Motor Control Interface related structure definitions and function prototype declaration
*/

#ifndef I2C_H_
#define I2C_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros

//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
extern void I2C_Init(void);
extern esp_err_t I2C_read(uint8_t slaveAddr, uint8_t* data_rd, size_t size);
extern esp_err_t I2C_read_reg(uint8_t slaveAddr, uint8 reg, uint8_t* data_rd, size_t size);
extern esp_err_t I2C_write(uint8_t slaveAddr, uint8_t* data_wr, size_t size);
extern esp_err_t I2C_write_reg(uint8_t slaveAddr, uint8_t reg, uint8_t* data_wr, size_t size);
esp_err_t I2C_write_byte(uint8_t slaveAddr, uint8_t data_wr);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* I2C_H_ */