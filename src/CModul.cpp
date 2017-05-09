#include "CModul.h"

namespace Str1pper
{
  CModul::CModul()
  {

  }
  CModul::~CModul()
  {

  }

  void CModul::Update()
  {
    int len = this->m_pCommands.size();
    for(int i = 0; i < len; ++i)
    {
      this->m_pCommands[i]->Execute();
    }
  }
}
