#include "Timer.h"

namespace Str1pper
{
  Timer::Timer():
  m_CurrentTick(millis()/1000)
  {}

  Timer::~Timer()
  {}

  void Timer::Tick()
  {

    this->m_PreviousTick = this->m_CurrentTick;
    this->m_CurrentTick = millis();

    Serial.println(this->m_PreviousTick);
    Serial.println(this->m_CurrentTick);
    Serial.println(this->m_CurrentTick - this->m_PreviousTick);
    Serial.println();

  }

  unsigned long Timer::GetDeltaTime()
  {
    return this->m_PreviousTick - this->m_CurrentTick;
  }

  unsigned long  Timer::GetTotalTime()
  {
    return this->m_CurrentTick;
  }
}
