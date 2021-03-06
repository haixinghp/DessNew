#ifndef MY_IODEF_H
#define MY_IODEF_H

#include "SAFE_HWType.h"

#define IIC1_SET_SDA_HIGH() M_SDA_FUN_ON
#define IIC1_SET_SCL_HIGH() M_SCL_FUN_ON
#define IIC1_SET_SDA_LOW() M_SDA_FUN_OFF
#define IIC1_SET_SCL_LOW() M_SCL_FUN_OFF

#define IIC1_CHK_SDA() M_SDA_IN_READ()

#endif