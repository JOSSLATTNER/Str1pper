#include "CModul.h"

namespace Str1pper
{


  class CScriptInterpreter
  {
  private:
    CModul* m_pCurrentModul;

  public:
      CScriptInterpreter();
      ~CScriptInterpreter();

    public:
      void Update();
  };
}
