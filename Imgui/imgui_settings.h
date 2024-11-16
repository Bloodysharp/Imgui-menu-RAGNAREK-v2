#include "imgui.h"

namespace c
{
	inline ImVec4 accent_color = ImColor(112, 109, 214);

	inline ImVec4 while_color = ImColor(255, 255, 255);
	inline ImVec4 black_color = ImColor(0, 0, 0);

	inline ImVec4 color_bg1 = ImColor(2, 2, 2);
	inline ImVec4 color_bg2 = ImColor(42, 42, 42);

	namespace bg
	{
		inline ImVec4 background = ImColor(21, 21, 21, 255);
		inline ImVec4 border = ImColor(23, 24, 25, 255);
		inline ImVec2 size = ImVec2(838, 600);
		inline float rounding = 12;
	}

	namespace child
	{
		inline ImVec4 background = ImColor(23, 24, 25, 255);
		inline float rounding = 8;
	}

	namespace widget
	{
		inline ImVec4 background = ImColor(28, 28, 35, 255);
		inline ImVec4 selectable = ImColor(37, 37, 47, 255);
		inline ImVec4 popup = ImColor(21, 21, 22, 255);

		inline ImVec4 circle = ImColor(47, 55, 65, 255);
		inline float rounding = 4;
	}

	namespace text
	{
		inline ImVec4 text_active = ImColor(255, 255, 255, 255);
		inline ImVec4 text_hov = ImColor(89, 95, 105, 255);
		inline ImVec4 text = ImColor(50, 54, 59, 255);
	}
}