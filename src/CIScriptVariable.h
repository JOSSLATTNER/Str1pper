#pragma once

namespace Str1pper
{
  class CIScriptVariable
  {
  public:
    virtual ~CIScriptVariable(){}

  public:
    virtual void* GetValuePtr() = 0;
  };
}
