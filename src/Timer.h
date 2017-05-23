#include <Time.h>
#include <functional>
#include <ESP8266WiFi.h>

namespace Str1pper
{
  // #define MS(t) (t*(1000/CLOCKS_PER_SEC))
  // typedef std::function<void()> TimerCallback_t

  class Timer
  {
  private:
    unsigned long m_PreviousTick;
    unsigned long m_CurrentTick;
    unsigned long m_DeltaTime;

  public:
    Timer();
    ~Timer();

  public:
    void Tick();
    float GetDeltaTime();
    unsigned long GetTotalTime();
  };
}
