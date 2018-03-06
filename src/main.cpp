#include <Arduino.h>
#include "Rest.h"
#include "API.h"
#include "LEDControl.h"
#include <SPIFFS.h>

LEDCNTRL::Rest* pRest;
LEDCNTRL::LEDControl* pController;
LEDCNTRL::API* pAPI;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setupFS()
{
  Serial.println("Try to Mount SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS Mounted!");
  listDir(SPIFFS, "/", 0);
}



void setup()
{
    Serial.begin(115200);
    delay(200);

  //  runDebug();
    setupFS();
    pController = new LEDCNTRL::LEDControl();
    pAPI = new LEDCNTRL::API(pController);
    pRest = new LEDCNTRL::Rest(pAPI);
   

    int c = pAPI->createChain({ 1 ,18,LEDCNTRL::LED_types::LED_TYPE_WS2812b_V2 });
    int s = pAPI->appendStrip(0, { 12,128 });

    pAPI->initChain(c);

    pAPI->setStripModul(LEDCNTRL::EModul::solidcolor, c, s);

    LEDCNTRL::modul_config_solidColor scfg;

    scfg.solidColor = LEDCNTRL::pixelFromRGB(42,0,71);
    pAPI->setStripConfig<LEDCNTRL::modul_config_solidColor>(c,s,scfg);



    Serial.println();
    Serial.println("Running Firmware.");
}

void loop()
{
  while(true)
  {
    pController->update();
  }
}

