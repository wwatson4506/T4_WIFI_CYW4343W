// PicoWi DNS example, see https://iosoft.blog/picowi
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
// =====================================================================
// = Modified for testing with Dogbone06 CYW4343W board and T4.1/DB5   =
// =====================================================================

#include <stdio.h>
#include "cyw43_T4_SDIO.h"
#include "event.h"
#include "join.h"
#include "ip.h"
#include "net.h"
#include "udp.h"
#include "dhcp.h"
#include "dns.h"
#include "secrets.h"

W4343WCard wifiCard;
Event dnsevt;

#define EVENT_POLL_USEC     100000
//#define PING_RESP_USEC      200000
#define PING_DATA_SIZE      32
//#define SERVER_NAME         "pool.ntp.org"
#define SERVER_NAME         "www.pjrc.com"
//#define SERVER_NAME         "www.raspberrypi.org"

#define LOCAL_PORT          1234

extern IPADDR my_ip, router_ip, dns_ip, zero_ip;
extern int dhcp_complete;

uint32_t led_ticks, poll_ticks, dns_ticks;
bool ledon=false;
MACADDR mac;

void setup()
{
   Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial && millis() < 5000) {;}
  Serial.printf("%c",12);

  if(CrashReport) {
	Serial.print(CrashReport);
    waitforInput();
  }
  Serial.printf("CPU speed: %ld MHz\n", F_CPU_ACTUAL / 1'000'000);
  pinMode(13, OUTPUT); // For debugging. Temporary usage.

  //////////////////////////////////////////
  //Begin parameters: 
  //SDIO1 (false), SDIO2 (true)
  //WL_REG_ON pin 
  //WL_IRQ pin (-1 to ignore)
  //EXT_LPO pin (optional, -1 to ignore)
  //////////////////////////////////////////
  if(wifiCard.begin(true, 33, 34, -1) == true) { 
    wifiCard.postInitSettings();
    Serial.println("initialization done");
    if (wifiCard.getMACAddress(mac) > 0) {
      Serial.printf("MAC address - ");
      for(uint8_t i = 0; i < 6; i++) {
        Serial.printf("%s%02X", i ? ":" : "", mac[i]);
      }
      Serial.printf("\n");
    }
      wifiCard.getFirmwareVersion();
    } else {
	  Serial.println("initialization failed!");
    }
    
    dnsevt.add_event_handler(join_event_handler);
    dnsevt.add_event_handler(arp_event_handler);
    dnsevt.add_event_handler(dhcp_event_handler);
    dnsevt.add_event_handler(udp_event_handler);
    udp_sock_init(udp_dns_handler, zero_ip, DNS_SERVER_PORT, LOCAL_PORT);
    wifiCard.set_display_mode(DISP_DNS);
    Serial.printf("TEENSY 4.1 DNS TEST\n\n");
    if (!join_start(MY_SSID, MY_PASSPHRASE, SECURITY))
        Serial.printf("Error: can't start network join\n");
    else if (!ip_init(zero_ip))
        Serial.printf("Error: can't start IP stack\n");
    else
    {
        // Additional diagnostic display (uncomment to display DNS info)
//        wifiCard.set_display_mode(DISP_INFO|DISP_JOIN|DISP_ARP|DISP_UDP|DISP_DHCP|DISP_DNS);

        ustimeout(&led_ticks, 0);
        ustimeout(&poll_ticks, 0);
        while (1)
        {
            // Toggle LED at 0.5 Hz if joined, 5 Hz if not
            if (ustimeout(&led_ticks, link_check() > 0 ? 1000000 : 100000))
                digitalWriteFast(13,ledon = !ledon);
            // Get any events, poll the network-join state machine
            if (sdio.wifi_get_irq() || ustimeout(&poll_ticks, EVENT_POLL_USEC))
            {
                dnsevt.pollEvents();
                join_state_poll(MY_SSID, MY_PASSPHRASE, SECURITY);
                ustimeout(&poll_ticks, 0);
            }
            // If link is up, poll DHCP state machine
            if (link_check() > 0)
                dhcp_poll();
            // When DHCP is complete, print IP addresses
            if (dhcp_complete == 1)
            {
                Serial.printf("DHCP complete, IP address ");
                print_ip_addr(my_ip);
                Serial.printf(" router ");
                print_ip_addr(router_ip);
                Serial.printf("\n");
                dhcp_complete = 2;
                ustimeout(&dns_ticks, 0);
            }
            if (dhcp_complete && ustimeout(&dns_ticks, 1000000))
            {
                if (!ip_find_arp(router_ip, mac))
                    ip_tx_arp(mac, router_ip, ARPREQ);
                else
                {
                    dns_tx(mac, dns_ip, LOCAL_PORT, (char *)SERVER_NAME);
                }
            }
        }
    }
}

void loop() {
	
}

void waitforInput()
{
  Serial.println("Press anykey to continue...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}

// EOF
