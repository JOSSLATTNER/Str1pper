#include "Color.h"

namespace Str1pper
{
  Color::Color()
  {

  }

  Color::Color(float a_rgb):
  r(a_rgb),g(a_rgb),b(a_rgb)
  {

  }

  Color::Color(float a_R, float a_G, float a_B):
  r(a_R),g(a_G),b(a_B)
  {

  }

  Color::Color(const Color& a_rColor):
  r(a_rColor.r),g(a_rColor.g),b(a_rColor.b)
  {

  }

  Color::~Color()
  {

  }

}
