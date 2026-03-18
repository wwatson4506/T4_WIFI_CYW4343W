#include "cyw43_T4_SDIO.h"
#include "scan.h"

W4343WCard wifiCard;
MACADDR mac;

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
}

void loop() {
  int entries = ScanNetworks();
  if(entries < 0) {
	Serial.printf("Scan error occured...");
  } else {
	Serial.printf("Number of scan entries: %d, Unfiltered scan (duplicate and hidden entries shown!)\n", entries);
  }
  Serial.printf("Wait for next scan...\n");
  delay(10000);
}

void waitforInput()
{
  Serial.println("Press anykey to continue...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
