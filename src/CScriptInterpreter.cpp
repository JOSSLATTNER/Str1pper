#include "CScriptInterpreter.h"

namespace Str1pper
{
  CScriptInterpreter::CScriptInterpreter()
  {

  }

  CScriptInterpreter::~CScriptInterpreter()
  {

  }

  void CScriptInterpreter::Update()
  {
    this->m_pCurrentModul->Update();
  }
}
