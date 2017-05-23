#pragma once


#include <string>
//#include <vector>
//#include <tuple>
#include <unordered_map>
#include "CCommand.h"
#include "CIScriptVariable.h"
#include "CScriptVariable.h"

namespace Str1pper
{



  class CModul
  {
  private:
    //std::string m_Name;
  //  std::vector<CCommand*> m_pCommands;
    std::unordered_map<std::string,CIScriptVariable*> m_pStorage;

  public:
    CModul();
    ~CModul();

  public:
    void Update();

    template<typename T>
    T* GetVariablePtr(std::string a_rName)
    {
      if(this->m_pStorage.count(a_rName) < 1)
      {
        this->m_pStorage[a_rName] = new CScriptVariable<T>();
      }

      T* tmp =  static_cast<float*>(this->m_pStorage[a_rName]->GetValuePtr());
      return  tmp;
    }

    template<typename T>
    void SetVariable(std::string a_Name, T a_Value)
    {
      T* pTmp =  this->GetVariablePtr<T>(a_Name);

      *pTmp = a_Value;
    }

  };
}
