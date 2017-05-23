#include "Updater.h"

namespace Str1pper
{

    Updater::Updater(std::string a_URL, float a_UpdateInterval):
    m_URL(a_URL), m_IsUpdateing(false),m_UpdateInterval(a_UpdateInterval),m_CurrentTime(0)
    {

    }

    Updater::~Updater()
    {

    }

    bool Updater::IsUpdating()
    {

    }

    bool Updater::CheckForUpdate(unsigned long a_deltaTime)
    {

      if(this->m_CurrentTime > this->m_UpdateInterval)
      {
        this->m_CurrentTime = 0;
        Serial.println("Update");
        t_httpUpdate_return ret = ESPhttpUpdate.update(this->m_URL.c_str());
        //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

        switch(ret)
        {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                break;
        }
      }

      this->m_CurrentTime += a_deltaTime;
      //  Serial.println(a_deltaTime);
      //  Serial.println(this->m_CurrentTime);
      //  Serial.println(this->m_UpdateInterval);

  }
}
