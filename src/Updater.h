#include <string>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

namespace Str1pper
{
  class Updater
  {
  private:
     std::string m_URL;
     bool m_IsUpdateing;
    //
     unsigned long m_CurrentTime;
     float m_UpdateInterval;

  public:
    Updater(std::string a_Url, float a_UpdateInterval);
    ~Updater();

  public:
    bool IsUpdating();
    bool CheckForUpdate(unsigned long a_deltaTime);
  };
}
