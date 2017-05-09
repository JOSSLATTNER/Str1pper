#include "CIScriptVariable.h"

namespace Str1pper
{
  class CFloatScriptVariable : public CIScriptVariable
  {
  private:
    float m_Value;

  public:
    CFloatScriptVariable(float a_Value);
    virtual ~CFloatScriptVariable();

  public:
    virtual float* GetValue() override;
  };
}
