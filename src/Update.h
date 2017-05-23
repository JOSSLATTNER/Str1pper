#include <string>

namespace Str1pper
{
  class Updater
  {
  public:
    Updater(std::string a_Url);
    ~Updater();

  public:
    bool IsUpdating();
    bool CheckForUpdate();
    
  };
}
