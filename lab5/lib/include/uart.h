#ifndef __UART_H__
#define __UART_H__

#include "aux.h"
#include "exception.h"
#include "gpio.h"
#include "mmio.h"
#include "task.h"

#define MAX_BUF_SIZE 0x1000
#define RX (1 << 0)  // read
#define TX (1 << 1)  // write

void uart_init();
void uart_write(char c);
void async_uart_write(char c);
void async_uart_putc(char* buf, uint32_t size);
void uart_read(char* buf, uint32_t size);
void async_uart_read(char* buf, uint32_t size);
void uart_flush();
void uart_write_string(char* str);
void uart_puth(uint32_t d);
void uart_putc(char* buf, uint32_t size);
void delay(uint32_t t);

void uart_enable_aux_int();
void uart_enable_int(uint32_t type);
void uart_disable_int(uint32_t type);
void uart_int_handler();

void uart_write_callback();
void uart_read_callback();

#endif