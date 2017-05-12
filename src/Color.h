#pragma once

namespace Str1pper
{
  class Color
  {
  public:
    float r;
    float g;
    float b;

  public:
    Color();
    Color(float a_rgb);
    Color(float a_R, float a_G, float a_B);
    Color(const Color& a_rColor);
    ~Color();

  public:

    Color& operator=(Color a_Color)
    {
      this->r = a_Color.r;
      this->g = a_Color.g;
      this->g = a_Color.b;
      return *this;
    };


    Color& operator+ (const Color& a_rColor)
    {
      this->r += a_rColor.r;
      this->g += a_rColor.g;
      this->b += a_rColor.b;

      return *this;
    };

    Color& operator+ (const float a_Value)
    {
      this->r += a_Value;
      this->g += a_Value;
      this->b += a_Value;

      return *this;
    };

    Color& operator+ (const double a_Value)
    {
      this->r += a_Value;
      this->g += a_Value;
      this->b += a_Value;

      return *this;
    };
  };
}
