#include <Arduino.h>
#include "Rest.h"
#include "API.h"
#include "LEDControll.h"
#include <SPIFFS.h>

Rest* pRest;
LEDCNTRL::LEDControll* pController;
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
    Serial.begin(9600);
    delay(200);

  //  runDebug();
    setupFS();

    pRest = new Rest();
    pController = new LEDCNTRL::LEDControll();
    pAPI = new LEDCNTRL::API(pController);

    int c = pAPI->createChain({ 1 ,18,LEDCNTRL::LED_WS2812B_V2 });
    int s = pAPI->appendStrand(0, { 12,128 });

    pAPI->initChain(c);

    pAPI->setStrandModul(LEDCNTRL::EModul::solidcolor, c, s);

    LEDCNTRL::modul_config_solidColor scfg;

    scfg.solidColor = LEDCNTRL::pixelFromRGB(42,0,71);
    pAPI->setStrandConfig<LEDCNTRL::modul_config_solidColor>(c,s,scfg);



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

