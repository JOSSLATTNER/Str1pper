#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include "CCommand.h"
#include "CIScriptVariable.h"

namespace Str1pper
{
  class CModul
  {
  private:
    std::string m_Name;
    std::vector<CCommand*> m_pCommands;
    std::unordered_map<std::string,CIScriptVariable*> m_pStorage;
    
  public:
    CModul();
    ~CModul();

  public:
    void Update();
  };
}
