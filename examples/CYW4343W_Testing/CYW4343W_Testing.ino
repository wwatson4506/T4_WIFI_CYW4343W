#include "cyw43_T4_SDIO.h"
#include "secrets.h"
#include "T4_SDIO.h"
#include "SdioRegs.h"
#include "ip.h"
#include "event.h"

#define VERSION "1.00"

extern T4_SDIO sdio;
Event myevt;
IOCTL_MSG rxmesg;

U32DATA u32d;

// Create an instance of wifiCard.
W4343WCard wifiCard;

//Setup default static ip address.
IPADDR myip = IPADDR_VAL(192,168,0,110);
IPADDR hostaddr = IPADDR_VAL(192,168,0,1);
extern int display_mode;
MACADDR mac;

BYTE tx_buff[TXDATA_LEN];

uint32_t n=0;
uint32_t readResponse = 0;
int val = 0;

void setup()
{
  Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial) {}

  Serial.printf("%c\nCYW4343W testing v" VERSION "\n",12);

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
    Serial.println("initialization done");
  } else {
    Serial.println("initialization failed!");
  }

  wifiCard.postInitSettings();

  Serial.println("Setup complete");

  if (wifiCard.getMACAddress(mac) > 0) {
    Serial.printf("MAC address - ");
    for (uint8_t i = 0; i < 6; i++) {
      Serial.printf("%s%02X", i ? ":" : "", mac[i]);
    }
    Serial.printf("\n");
  }
  wifiCard.getFirmwareVersion();
/*
  //Read chip ID 
  //This was 4 byte read in the Zero code, causes extra bytes in the buffer - only if first CMD53 executed. 
  //Changing size to < 4 "fixes" it. Debug code sdio.c 3909 formats value as %4x, so use size 2
  n = sdio.wifi_data_read(SD_FUNC_BAK, SB_32BIT_WIN, u32d.bytes, 2);
  Serial.printf("\n*************\nCardID: %ld\n*************\n", u32d.uint32 & 0xFFFF);
  Serial.printf("n = %x\n",n);

  n = sdio.wifi_reg_read(SD_FUNC_BUS, BUS_SPEED_CTRL_REG, 1);
  Serial.printf("n = %d\n",n&0xff);
  n = sdio.wifi_reg_read(SD_FUNC_BUS, BUS_SPEED_CTRL_REG, 1);
  Serial.printf("n = %d\n",n&0xff);

  display_mode |= DISP_ARP;
  display_mode |= DISP_REG;
*/
  myevt.add_event_handler(icmp_event_handler);
  myevt.add_event_handler(arp_event_handler);
  waitforInput();
// Use "secrets.h" to set MY_SSID, MY_PASSPHRASE, SECURITY.
  wifiCard.JoinNetworks(MY_SSID, MY_PASSPHRASE, SECURITY);
//  begin(myip); // Required default ip address.
//  n = ip_make_arp(tx_buff, mac, hostaddr, ARPREQ);
//  wifiCard.display(DISP_ARP,(char *)tx_buff);
//  n = ip_make_arp(tx_buff, mac, hostaddr, ARPRESP);
//  Serial.printf("n = %d\n",n);
//  waitforInput();
}

void loop() {
//val = myevt.event_poll();
//Serial.printf("val = %d\n",val);
  wifiCard.join_network_poll();
//Serial.printf("loop()\n"):
//delay(1000);
}

void waitforInput()
{
  Serial.println("Press anykey to continue");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
