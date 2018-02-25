#pragma once 
namespace LEDCNTRL
{
	typedef union {
		struct {
			uint8_t r, g, b, w;
		};
		uint32_t num;
	} pixelColor_t;


	inline pixelColor_t pixelFromRGB(uint8_t r, uint8_t g, uint8_t b)
	{
		pixelColor_t v;
		v.r = r;
		v.g = g;
		v.b = b;
		v.w = 0;
		return v;
	}

	inline pixelColor_t pixelFromRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
	{
		pixelColor_t v;
		v.r = r;
		v.g = g;
		v.b = b;
		v.w = w;
		return v;
	}
}