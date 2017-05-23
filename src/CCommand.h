#pragma once
#include "Enums.h"
#include <string>
//#include <vector>
//#include <tuple>
#include <unordered_map>
#include "CIScriptVariable.h"

namespace Str1pper
{
  class CCommand
  {
  private:
    //Operator

    EOPERATOR m_Operator;
    std::unordered_map<std::string,CIScriptVariable*>* a_pStorage;

  public:
    CCommand();
    ~CCommand();

  public:
    void Execute();
  };
}
