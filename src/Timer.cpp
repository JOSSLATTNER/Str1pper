#include "Timer.h"

namespace Str1pper
{
  Timer::Timer()
  {

    m_CurrentTick = millis();

  }

  Timer::~Timer()
  {}

  void Timer::Tick()
  {

    this->m_PreviousTick = this->m_CurrentTick;
    this->m_CurrentTick = millis();
    this->m_DeltaTime = this->m_CurrentTick - this->m_PreviousTick;


    //  Serial.println("------Time------");
    //  Serial.println(this->m_PreviousTick);
    //  Serial.println(this->m_CurrentTick);
    //  Serial.println(this->m_DeltaTime);
    //  Serial.println();

  }

  float Timer::GetDeltaTime()
  {
    return this->m_DeltaTime;
  }

  unsigned long  Timer::GetTotalTime()
  {
    return this->m_CurrentTick;
  }
}
