#include "Enums.h"

namespace Str1pper
{
  class CCommand
  {
  private:
    ECommandType m_CommandType;

  public:
    CCommand();
    ~CCommand();

  public:
    void Execute();
  };
}
