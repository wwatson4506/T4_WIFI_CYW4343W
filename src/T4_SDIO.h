// PicoWi half-duplex SPI interface, see https://iosoft.blog/picowi
//
// Copyright (c) 2022, Jeremy P Bentham
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef T4_SDIO_H
#define T4_SDIO_H

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif
//int wifi_setup(void);
//int wifi_start(void);
int wifi_data_read(int func, int addr, uint8_t *dp, int nbytes);
int wifi_data_write(int func, int addr, uint8_t *dp, int nbytes);
uint32_t wifi_reg_read(int func, uint32_t addr, int nbytes);
int wifi_reg_write(int func, uint32_t addr, uint32_t val, int nbytes);
bool wifi_rx_event_wait(int msec, uint8_t evt);
bool wifi_reg_val_wait(int ms, int func, int addr, uint32_t mask, uint32_t val, int nbytes);
bool wifi_get_irq(void);
char *wifi_func_str(int func);
#ifdef __cplusplus
}
#endif

#include  "SdioCard.h"

#define MAX_BLOCKLEN    64

// Union to handle 8/16/32 bit conversions
typedef union
{
    int32_t  int32;
    uint32_t uint32;
    uint32_t uint24 : 24;
    uint16_t uint16;
    uint8_t  uint8;
    uint8_t  bytes[4];
} U32DATA;

// Structures for SPI communication
#define SPI_TEST_VALUE 0xfeedbead

typedef struct
{
    uint32_t len : 11, addr : 17, func : 2, incr : 1, wr : 1;
} SPI_MSG_HDR;

// SPI message
typedef union
{
    SPI_MSG_HDR hdr;
    uint32_t vals[2];
    uint8_t bytes[2000];
} SPI_MSG;

class T4_SDIO;
class T4_SDIO {
  public:
  //int wifi_setup(void);
  //int wifi_start(void);
  int wifi_data_read(int func, int addr, uint8_t *dp, int nbytes);
  int wifi_data_write(int func, int addr, uint8_t *dp, int nbytes);
  uint32_t wifi_reg_read(int func, uint32_t addr, int nbytes);
  int wifi_reg_write(int func, uint32_t addr, uint32_t val, int nbytes);
  bool wifi_rx_event_wait(int msec, uint8_t evt);
  bool wifi_reg_read_check(int func, int addr, uint32_t mask, uint32_t val, int nbytes);
  bool wifi_reg_val_wait(int ms, int func, int addr, uint32_t mask, uint32_t val, int nbytes);
  bool wifi_get_irq(void);
  char *wifi_func_str(int func);
  protected:
  private:
};

extern T4_SDIO sdio;

#endif
// EOF
