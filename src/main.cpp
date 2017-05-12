#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <stdlib.h>
#include <cmath>
//#include <vector>
#include <functional>
#include "Color.h"

extern "C" {
#include "user_interface.h"
}

//WIFI SETTINGS
const char* ssid = "o2-WLAN51";
const char* password = "6689171153799911";
const bool apMode = false;
const char WiFiAPPSK[] = "";

//NEOPIXEL CONFIG
#define LEDPIN 0
#define NUMPIXELS 300
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
Str1pper::Color hsv2rgb_smooth(Str1pper::Color& c );
//WEBSERVER
AsyncWebServer server(80);
AsyncWebSocket ws("/stripcontroll");

struct color
{
  float r;
  float g;
  float b;
};

//MODULES
double _Timer;
Str1pper::Color _Color;

typedef std::function<Str1pper::Color(unsigned int)> ModuleLoop_t;

ModuleLoop_t module_solid = [&](unsigned int i) -> Str1pper::Color
{
  return _Color;
};

ModuleLoop_t module_blink = [](unsigned int i) -> Str1pper::Color
{
  float f = sin(_Timer) * 0.5f + 0.5f;
  return { _Color.r * f, _Color.g * f, _Color.b * f };
};

ModuleLoop_t module_sin = [&](unsigned int i) -> Str1pper::Color
{
  float steps = NUMPIXELS / 3.0f;
  float currentStep = i / steps;

  float f = sin(currentStep * 6.28f - 1.57f + _Timer) * 0.5f + 0.5f;
  return { _Color.r * f, _Color.g * f, _Color.b * f };
};

ModuleLoop_t module_plasma = [&](unsigned int i) -> Str1pper::Color
{
  float steps = NUMPIXELS / 1.0f;
  float currentStep = i / steps;
  float cx = currentStep + 0.5f * sin(_Timer / 5.0f);
  float cy = currentStep + 0.5f * cos(_Timer / 3.0f);

  return { cx, cy, 0};
};

ModuleLoop_t module_hsv = [&](unsigned int i) -> Str1pper::Color
{
  float currentStep = i / NUMPIXELS;

  Str1pper::Color hsv;

//  hsv.r = currentStep;
  //hsv.g = 1.0f;
//  hsv.b = 1.0f;

  return hsv2rgb_smooth(hsv);
};



size_t ModuleIndex = 0;
ModuleLoop_t Modules[] = {
  module_solid,
  module_blink,
  module_sin,
  module_plasma
};

template<typename T>
T clamp(T x, T a, T b)
{
  return (x > b) ? b : (x < a) ? a : x;
}

Str1pper::Color hsv2rgb_smooth(Str1pper::Color& c )
{
  // Str1pper::Color hsv = {0.0, 4.0, 2.0};
  // Str1pper::Color hsvmod = hsv + 6.0f * c.r;
  //
  // float r = fmod((double)hsvmod.r,6.0);
  // float g = fmod((double)hsvmod.g,6.0);
  // float b = fmod((double)hsvmod.b,6.0);
  //
  // r = clamp( abs(r-3.0)-1.0, 0.0, 1.0 );
  // g = clamp( abs(g-3.0)-1.0, 0.0, 1.0 );
  // b = clamp( abs(b-3.0)-1.0, 0.0, 1.0 );
  //
  //
  // Str1pper::Color s(r,g,b);
  //
  // s = s + hsv;

	//rgb = rgb*rgb*(3.0-2.0*rgb); // cubic smoothing

	return 0;//s;//c.z * mix( vec3(1.0), rgb, c.y);
}
//
// color HSVToRGB(color hsv)
// {
//     color rgb
// }

void setupWifi()
{
  if (apMode)
  {
    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "ESP8266 Thing " + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i = 0; i < AP_NameString.length(); i++)
      AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);

    Serial.printf("Connect to Wi-Fi access point: %s\n", AP_NameChar);
    Serial.println("and open http://192.168.4.1 in your browser");
  }
  else
  {
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", ssid);
    if (String(WiFi.SSID()) != String(ssid)) {
      WiFi.begin(ssid, password);
    }

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.print("Connected! Open http://");
    Serial.print(WiFi.localIP());
    Serial.println(" in your browser");
  }
}

void registerEvents()
{
  //  server.on("/color", HTTP_POST, [](AsyncWebServerRequest *request){
  //    AsyncWebParameter* r = request->getParam("r",true);
  //    AsyncWebParameter* g = request->getParam("g",true);
  //    AsyncWebParameter* b = request->getParam("b",true);
   //
  //    setColor(r->value().toInt(),g->value().toInt(),b->value().toInt(), 128);
  //    request->send(200, "text/plain", "");
  //  });
   //
  //  server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request){
  //      setColor(255,0,0,255);
  //      request->send(200, "text/plain", "Red");
  //  });
}

void parseCommand(String a_rJsonString)
{
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& jobj = jsonBuffer.parseObject(a_rJsonString);
  Serial.println(a_rJsonString);

  if(!jobj.success())
  {
    Serial.println("parseObject() failed");
    return;
  }

  if(jobj["command"] == "SET_MODULE")
  {
      ModuleIndex = jobj["data"]["module"];
      Serial.print("ModulIndex:");
      Serial.print(ModuleIndex);
  }
  else if(jobj["command"] == "SET_COLOR")
  {
      float r =  jobj["data"]["rgb"]["r"];
      float g =  jobj["data"]["rgb"]["g"];
      float b =  jobj["data"]["rgb"]["b"];

      _Color = { r, g, b };
  }
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT)
  {
     //client connected
     os_printf("ws[%s][%u] connect\n", server->url(), client->id());
   }
   else if(type == WS_EVT_DISCONNECT)
   {
     //client disconnected
     os_printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
   }
   else if(type == WS_EVT_ERROR)
   {
     //error was received from the other end
     os_printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
   }
   else if(type == WS_EVT_DATA)
   {
     //data packet
     AwsFrameInfo * info = (AwsFrameInfo*)arg;
     if(info->final && info->index == 0 && info->len == len)
     {
       //the whole message is in a single frame and we got all of it's data
       os_printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
       if(info->opcode == WS_TEXT)
       {
         data[len] = 0;
         String json = String((char*)data);

         parseCommand(json);
       }
     }

   }
   delay(1);
}


void setupWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setupWebCommuncation()
{
  setupWebSocket();
  server.serveStatic("/", SPIFFS, "/");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/image", SPIFFS, "/image");
  server.serveStatic("/css", SPIFFS, "/css");

  registerEvents();
  server.begin();
  Serial.println("HTTP server started");
}

void setupLeds()
{
  pixels.begin();
  pixels.show();
  Serial.println("Leds started");
}

void setupFS()
{
  SPIFFS.begin();
   {
     Dir dir = SPIFFS.openDir("/");
     while (dir.next()) {
       String fileName = dir.fileName();
       size_t fileSize = dir.fileSize();
       Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
     }
     Serial.printf("\n");
   }
}

void setup(void)
{
  Serial.begin(115200);
  delay(100);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();

  setupFS();
  setupLeds();
  setupWifi();
  setupWebCommuncation();
}

void loop(void)
{
  for (size_t i = 0; i < NUMPIXELS; ++i)
  {
      Str1pper::Color c = Modules[ModuleIndex](i);
      pixels.setPixelColor(i, c.r * 255, c.g * 255, c.b * 255);
  }

  _Timer += 0.016f;
  pixels.show();
}
