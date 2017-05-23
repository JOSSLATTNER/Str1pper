#include <Time.h>
#include <ESP8266WiFi.h>

namespace Str1pper
{
  #define MS(t) (t*(1000/CLOCKS_PER_SEC))

  class Timer
  {
  private:
    unsigned long m_PreviousTick;
    unsigned long m_CurrentTick;


  public:
    Timer();
    ~Timer();

  public:
    void Tick();
    unsigned long GetDeltaTime();
    unsigned long GetTotalTime();
  };
}
