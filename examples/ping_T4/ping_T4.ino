// PicoWi PING example, see https://iosoft.blog/picowi
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

#include "cyw43_T4_SDIO.h"
#include "ip.h"
#include "join.h"
#include "secrets.h"

W4343WCard wifiCard;
MACADDR mac;

#define EVENT_POLL_USEC     100000
#define PING_RESP_USEC      300000 
#define PING_DATA_SIZE      32

extern Event ipevt;
extern uint32_t ping_tx_time, ping_rx_time;

// IP address of this unit (must be unique on network)
IPADDR myip   = IPADDR_VAL(192, 168, 0, 110);
// IP address of Access Point (NON-DNS)
IPADDR hostip = IPADDR_VAL(192,168,0,105); //Your unique IP address on your local network.

BYTE ping_data[PING_DATA_SIZE];
uint32_t led_ticks, poll_ticks, ping_ticks;
bool ledon=false;
int i, ping_state=0, t;

void setup()
{
  Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial && millis() < 5000) {}
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
  if (wifiCard.begin(true, 33, 34, -1) == true) { 

    wifiCard.postInitSettings();
    
    Serial.println("initialization done");

  if (wifiCard.getMACAddress(mac) > 0) {
    Serial.printf("MAC address - ");
    for (uint8_t i = 0; i < 6; i++) {
      Serial.printf("%s%02X", i ? ":" : "", mac[i]);
    }
    Serial.printf("\n");
  }

    wifiCard.getFirmwareVersion();
  } else {
    Serial.println("initialization failed!");
  }
  Serial.println("Setup complete");

  for(i=0; i<(int)sizeof(ping_data); i++)
    ping_data[i] = (BYTE)i;
  ipevt.add_event_handler(icmp_event_handler);
  ipevt.add_event_handler(arp_event_handler);
  ipevt.add_event_handler(join_event_handler);

  if(!join_start(MY_SSID, MY_PASSPHRASE, SECURITY))
    Serial.printf("Error: can't start network join\n");
  else if(!ip_init(myip))
    Serial.printf("Error: can't start IP stack\n");
        ustimeout(&led_ticks, 0);
        ustimeout(&poll_ticks, 0);
        while (1)
        {
            // Toggle LED at 0.5 Hz if joined, 5 Hz if not
            if (ustimeout(&led_ticks, link_check() > 0 ? 1000000 : 100000))
            {
                digitalWriteFast(13,ledon = !ledon);
                ustimeout(&ping_ticks, 0);
                // If LED is on, and we have joined a network..
                if (ledon && link_check()>0)
                {
                    // If not ARPed, send ARP request
                    if (!ip_find_arp(hostip, mac))
                    {
                        ip_tx_arp(mac, hostip, ARPREQ);
                        ping_state = 1;
                    }
                    // If ARPed, send ICMP request
                    else
                    {
                        ip_tx_icmp(mac, hostip, ICREQ, 0, ping_data, sizeof(ping_data));
                        ping_rx_time = 0;
                        ping_state = 2;
                   }
                }
            }
            // If awaiting ARP response..
            if (ping_state == 1)
            {
                // If we have an ARP response, send ICMP request
                if (ip_find_arp(hostip, mac))
                {
                    ustimeout(&ping_ticks, 0);
                    ip_tx_icmp(mac, hostip, ICREQ, 0, ping_data, sizeof(ping_data));
                    ping_rx_time = 0;
                    ping_state = 2;
                }
            }
            // Check for timeout on ARP or ICMP request
            if ((ping_state==1 || ping_state==2) && 
                ustimeout(&ping_ticks, PING_RESP_USEC))
            {
                Serial.printf("%s timeout\n", ping_state==1 ? "ARP" : "ICMP");
                ping_state = 0;
            }
            // If ICMP response received, LED off, print time
            else if (ping_state == 2 && ping_rx_time)
            {
                t = (ping_rx_time - ping_tx_time + 50) / 100;
                Serial.printf("Round-trip time %d.%d ms\n", t/10, t%10);
                digitalWriteFast(13,LOW);
                ping_state = 0;
            }
            // Get any events, poll the network-join state machine
            if (sdio.wifi_get_irq() || ustimeout(&poll_ticks, EVENT_POLL_USEC))
            {
                ipevt.pollEvents();
                join_state_poll((char *)MY_SSID, (char *)MY_PASSPHRASE, SECURITY);
                ustimeout(&poll_ticks, 0);
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
