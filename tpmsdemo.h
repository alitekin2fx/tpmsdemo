#ifndef __TPMSDEMO_H__
#define __TPMSDEMO_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#define UART_PORT FILE*

void sys_handle_error();
uint32_t sys_get_ticks();

#endif //__TPMSDEMO_H__
