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

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "cyw43_T4_SDIO.h"
#include "misc_defs.h"
#include "SdioRegs.h"
#include "T4_SDIO.h"

class T4_SDIO;

// Read a register
uint32_t T4_SDIO::wifi_reg_read(int func, uint32_t addr, int nbytes) {

    uint32_t val = 0, ret = 0;

    wifi_data_read(func, addr, (uint8_t *)&val, nbytes);
    ret = val;
    
    // Debug info below.
    Serial.printf("wifi_reg_read: len %u %s 0x%04lX -> 0x%02lX\n", 
        nbytes,
        wifi_func_str(func),
        addr,
        ret);
    return (ret);
}

// Read data block using SDIO
int T4_SDIO::wifi_data_read(int func, int addr, uint8_t *dp, int nbytes) {
  uint32_t ret = 0;

  ret = wifiCard.cardCMD53_read((uint32_t) func, (uint32_t)addr, (uint8_t *)dp, (uint32_t)nbytes, false);
/*
  Serial.printf("wifi_data_read: func = %s, addr = 0x%8.8X, *dp = 0x%8.8X, nbytes = %d\n",
         wifi_func_str(func),
         addr,
         *dp,
         nbytes);
*/
  return (ret); // Return actual bytes read.
}

// Write a register
int T4_SDIO::wifi_reg_write(int func, uint32_t addr, uint32_t val, int nbytes) {
  uint32_t ret = 0;
    
  // Debug info below.
  Serial.printf("wifi_reg_write: len %u %s 0x%04lX <- 0x%02lX\n",
         nbytes,
         wifi_func_str(func),
         addr,
         val);
  if (nbytes > 1) {
    ret = wifi_data_write(func, addr, (uint8_t *)&val, nbytes);
  } else {
	ret = wifiCard.cardCMD52_write(func, addr, (uint8_t)val);  
  }

    return (ret);
}

// Write a data block using SDIO
int T4_SDIO::wifi_data_write(int func, int addr, uint8_t *dp, int nbytes) {
  uint32_t ret = 0;

  // Debug info below.
/*
  Serial.printf("wifi_data_write: len %u %s 0x%04lX <- 0x%02lX\n",
         nbytes,
         wifi_func_str(func),
         addr,
         *dp);
*/
  ret = wifiCard.cardCMD53_write(func, addr, dp, nbytes, false);


  return (ret); // Return actual bytes written.
}

// Poll for WiFi receive data event, with timeout (NOT USED!!!)
bool T4_SDIO::wifi_rx_event_wait(int msec, uint8_t evt) {

    bool ok;
    
    while (!(ok = (wifi_reg_read(SD_FUNC_BUS, BUS_SPI_STATUS_REG, 1) & evt) == evt) && msec--)
        delayMicroseconds(1000);
    return (ok);
}

// Read & check a masked value, return zero if incorrect
bool wifi_reg_read_check(int func, int addr, uint32_t mask, uint32_t val, int nbytes)
{
    return((wifi_reg_read(func, addr, nbytes) & mask) == val);
}

// Check register value every msec, until correct or timeout
bool T4_SDIO::wifi_reg_val_wait(int ms, int func, int addr, uint32_t mask, uint32_t val, int nbytes)
{
    bool ok;

    while (!(ok=wifi_reg_read_check(func, addr, mask, val, nbytes)) && ms--)
        delayMicroseconds(1000);
    return(ok);
}

// Get state of IRQ pin (NOT USED!!!)
bool T4_SDIO::wifi_get_irq(void) {

  return (digitalReadFast(34));
/*
#if SD_IRQ_ASSERT
    return (io_in(SD_IRQ_PIN));
#else
    return (!io_in(SD_IRQ_PIN));
#endif    
*/
}

// Return string with function name
char *T4_SDIO::wifi_func_str(int func)
{
    func &= SD_FUNC_MASK;
    return (func == SD_FUNC_BUS ? (char *)"Bus" :
            func == SD_FUNC_BAK ? (char *)"Bak" :
            func == SD_FUNC_RAD ? (char *)"Rad" : (char *)"???");
}

// EOF
