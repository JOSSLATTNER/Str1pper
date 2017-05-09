#include "CFloatScriptVariable.h"

namespace Str1pper
{
  CFloatScriptVariable::CFloatScriptVariable(float a_Value)
  {
      m_Value = a_Value;
  }

  CFloatScriptVariable::~CFloatScriptVariable()
  {

  }

  float* CFloatScriptVariable::GetValue()
  {
    return &this->m_Value;
  }
}
