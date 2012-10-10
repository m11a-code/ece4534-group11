#ifndef __maindefs
#define __maindefs

#ifdef __XC8
#include <xc.h>
#ifdef _18F45J10
#define __USE18F45J10 1
#else
#ifdef _18F2680
//#define __MASTER2680
//#define __SLAVE2680
#define __MOTOR2680
#define __USE18F2680 1
#else
#ifdef _18F26J50
#define __USE18F26J50
#endif
#endif
#endif
#else
#ifdef __18F45J10
#define __USE18F45J10 1
#else
#ifdef __18F2680
#define __USE18F2680 1
#endif
#endif
#include <p18cxxx.h>
#endif

// Message type definitions
#define MSGT_TIMER0 10
#define MSGT_TIMER1 11
#define MSGT_MAIN1 20
#define	MSGT_OVERRUN 30
#define MSGT_UART_DATA 31
#define MSGT_I2C_DBG 41
#define	MSGT_I2C_DATA 40
#define MSGT_I2C_RQST 42
#define MSGT_I2C_MASTER_SEND_COMPLETE 43
#define MSGT_I2C_MASTER_SEND_FAILED 44
#define MSGT_I2C_MASTER_RECV_COMPLETE 45
#define MSGT_I2C_MASTER_RECV_FAILED 46

//I2C bus message parameters
#define I2C_MSG_SIZE 4
#define SONAR_MSG_TYPE 0x10
#define ENCODERS_MSG_TYPE 0x11
#define CAMERA_MSG_TYPE 0x12
#define SONAR_EMPTY_MSG_TYPE 0x50
#define ENCODERS_EMPTY_MSG_TYPE 0x51
#define CAMERA_EMPTY_MSG_TYPE 0x52
#define GENERIC_EMPTY_MSG_TYPE 0x53

//I2C slave addresses
#define SONAR_ADDR 0x9A
#define CAMERA_ADDR 0x9C
#define ENCODERS_ADDR 0x9E

#endif

#ifdef __USE18F26J50
#pragma config XINST = OFF
#endif

