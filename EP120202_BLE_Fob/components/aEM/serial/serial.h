#ifndef __SERIAL_H__
#define __SERIAL_H__

#define DEBUG_INSTANCE  0

#define SERIAL_LOG_USING_TASK   1

//#define UART_BPS    115200
// #define UART_BPS    230400
//#define UART_BPS    345600
//#define UART_BPS    460800
// #define UART_BPS    921600
 #define UART_BPS    2000000
void serial_init(void);
void serial_debug_init(void);

#endif
