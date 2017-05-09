#pragma once
#include "CIScriptVariable.h"

namespace Str1pper
{
  template<typename T>
  class CScriptVariable : public CIScriptVariable
  {
  private:
    T m_Value;

  public:
    CScriptVariable(T a_Value)
    {
      this->m_Value = a_Value;
    }

  public:
    CScriptVariable()
    {
      this->m_Value = 0;
    }

    virtual ~CScriptVariable()
    {

    }

  public:
    virtual T* GetValuePtr() override
    {
      return &this->m_Value;
    }
  };
}
