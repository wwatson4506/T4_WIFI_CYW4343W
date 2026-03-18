#include "cyw43_T4_SDIO.h"
#include "join.h"
#include "secrets.h"

#define VERSION "1.00"
#define EVENT_POLL_USEC     100000

W4343WCard wifiCard;
MACADDR mac;
Event myevt;
uint32_t led_ticks, poll_ticks;
bool ledon=false;

void setup()
{
  Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial && millis() < 5000) {}

  Serial.printf("%c\nCYW4343W JOIN NETWORK v" VERSION "\n",12);

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
    
    Serial.println("Initialization Done");

  if (wifiCard.getMACAddress(mac) > 0) {
    Serial.printf("MAC address - ");
    for (uint8_t i = 0; i < 6; i++) {
      Serial.printf(SER_YELLOW "%s%02X", i ? ":" : "", mac[i]);
    }
    Serial.printf(SER_RESET "\n");
  }
    wifiCard.getFirmwareVersion();
  } else {
    Serial.println("initialization failed!");
  }

  Serial.println(SER_GREEN "Setup complete" SER_RESET);
  Serial.printf(SER_YELLOW "To display network activity after network is joined,\n");
  Serial.printf("set '#define USE_ACTIVITY_DISPLAY false' to true in\n");
  Serial.printf("the file 'misc_defs.h'.\n");
  Serial.printf("This can display a pretty heavy spew of information.\n");
  Serial.printf("Especially when we are being ARP'ed by the host machine and/or multicast is enabled.\n");
  Serial.printf("The host machine will ARP us approximately every 5 minutes or so.\n");
  Serial.printf("You will need to set your SSID and passphrase in the\n");
  Serial.printf("'secrets.h' file located in the src folder.\n" SER_RESET);
  waitforInput();

  // Add our event handler to the array of event handlers.
  myevt.add_event_handler(join_event_handler);
  delayMicroseconds(1000);

  // Use "secrets.h" to set MY_SSID, MY_PASSPHRASE, SECURITY.
  if(!join_start(MY_SSID, MY_PASSPHRASE, SECURITY)) {
    Serial.printf("Error: can't start network join\n");
    while(1);
  }
	  
  ustimeout(&led_ticks, 0);
  ustimeout(&poll_ticks, 0);
}

void loop() {
    // Toggle LED at 1 Hz if joined, 5 Hz if not
    if(ustimeout(&led_ticks, link_check() > 0 ? 500000 : 100000))
      digitalWriteFast(13,(ledon = !ledon));
                
    // Get any events, poll the joining state machine
    if(ustimeout(&poll_ticks, EVENT_POLL_USEC)) {
      myevt.pollEvents();
      join_state_poll(MY_SSID, MY_PASSPHRASE, SECURITY);
      ustimeout(&poll_ticks, 0);
    }

}

void waitforInput()
{
  Serial.println("Press anykey to join network...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
