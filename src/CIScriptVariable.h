namespace Str1pper
{
  class CIScriptVariable
  {
  public:
    virtual ~CIScriptVariable(){}

  public:
    virtual void* GetValue() = 0;
  };
}
