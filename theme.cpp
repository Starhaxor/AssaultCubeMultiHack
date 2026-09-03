#include "pch.h"

#include "imgui.h"

#include "theme.h"

namespace theme
{
	void ApplyHeritageCream()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* c = style.Colors;

		// Clean dark theme with a restrained gold accent.
		// Large surfaces stay near-neutral dark, gold is only an accent.
		const ImVec4 text       = ImVec4(0.93f, 0.91f, 0.86f, 1.00f);
		const ImVec4 textDim    = ImVec4(0.48f, 0.46f, 0.42f, 1.00f);
		const ImVec4 surface    = ImVec4(0.075f, 0.070f, 0.065f, 0.98f);
		const ImVec4 surface2   = ImVec4(0.105f, 0.098f, 0.088f, 1.00f);
		const ImVec4 surface3   = ImVec4(0.150f, 0.138f, 0.120f, 1.00f);
		const ImVec4 surfaceHov = ImVec4(0.205f, 0.185f, 0.150f, 1.00f);
		const ImVec4 line       = ImVec4(0.300f, 0.250f, 0.170f, 1.00f);
		const ImVec4 gold       = ImVec4(0.870f, 0.640f, 0.220f, 1.00f);
		const ImVec4 goldBright = ImVec4(0.980f, 0.750f, 0.300f, 1.00f);
		const ImVec4 goldDeep   = ImVec4(0.480f, 0.350f, 0.140f, 1.00f);
		const ImVec4 btn        = ImVec4(0.200f, 0.170f, 0.125f, 1.00f);
		const ImVec4 btnHov     = ImVec4(0.360f, 0.275f, 0.140f, 1.00f);

		c[ImGuiCol_Text]                  = text;
		c[ImGuiCol_TextDisabled]          = textDim;
		c[ImGuiCol_WindowBg]              = surface;
		c[ImGuiCol_ChildBg]               = surface2;
		c[ImGuiCol_PopupBg]               = ImVec4(0.120f, 0.110f, 0.098f, 0.98f);
		c[ImGuiCol_Border]                = line;
		c[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_FrameBg]               = surface3;
		c[ImGuiCol_FrameBgHovered]        = surfaceHov;
		c[ImGuiCol_FrameBgActive]         = goldDeep;
		c[ImGuiCol_TitleBg]               = ImVec4(0.100f, 0.090f, 0.080f, 1.00f);
		c[ImGuiCol_TitleBgActive]         = goldDeep;
		c[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.080f, 0.072f, 0.065f, 1.00f);
		c[ImGuiCol_MenuBarBg]             = surface2;
		c[ImGuiCol_ScrollbarBg]           = ImVec4(0.060f, 0.055f, 0.050f, 1.00f);
		c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.300f, 0.250f, 0.180f, 1.00f);
		c[ImGuiCol_ScrollbarGrabHovered]  = gold;
		c[ImGuiCol_ScrollbarGrabActive]   = goldBright;
		c[ImGuiCol_CheckMark]             = goldBright;
		c[ImGuiCol_SliderGrab]            = gold;
		c[ImGuiCol_SliderGrabActive]      = goldBright;
		c[ImGuiCol_Button]                = btn;
		c[ImGuiCol_ButtonHovered]         = btnHov;
		c[ImGuiCol_ButtonActive]          = goldDeep;
		c[ImGuiCol_Header]                = btn;
		c[ImGuiCol_HeaderHovered]         = btnHov;
		c[ImGuiCol_HeaderActive]          = goldDeep;
		c[ImGuiCol_Separator]             = line;
		c[ImGuiCol_SeparatorHovered]      = gold;
		c[ImGuiCol_SeparatorActive]       = goldBright;
		c[ImGuiCol_ResizeGrip]            = line;
		c[ImGuiCol_ResizeGripHovered]     = gold;
		c[ImGuiCol_ResizeGripActive]      = goldBright;
		c[ImGuiCol_Tab]                   = btn;
		c[ImGuiCol_TabHovered]            = btnHov;
		c[ImGuiCol_TabActive]             = goldDeep;
		c[ImGuiCol_TabUnfocused]          = surface2;
		c[ImGuiCol_TabUnfocusedActive]    = surface3;
		c[ImGuiCol_PlotLines]             = gold;
		c[ImGuiCol_PlotLinesHovered]      = goldBright;
		c[ImGuiCol_PlotHistogram]         = goldDeep;
		c[ImGuiCol_PlotHistogramHovered]  = gold;
		c[ImGuiCol_TableHeaderBg]         = goldDeep;
		c[ImGuiCol_TableBorderStrong]     = line;
		c[ImGuiCol_TableBorderLight]      = ImVec4(0.220f, 0.190f, 0.140f, 1.00f);
		c[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_TableRowBgAlt]         = ImVec4(0.93f, 0.91f, 0.86f, 0.04f);
		c[ImGuiCol_TextSelectedBg]        = ImVec4(0.480f, 0.350f, 0.140f, 0.60f);
		c[ImGuiCol_DragDropTarget]        = goldBright;
		c[ImGuiCol_NavHighlight]          = gold;
		c[ImGuiCol_NavWindowingHighlight] = goldBright;
		c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.05f, 0.04f, 0.03f, 0.60f);
		c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.05f, 0.04f, 0.03f, 0.60f);

		style.WindowPadding     = ImVec2(12.0f, 10.0f);
		style.FramePadding      = ImVec2(8.0f, 4.0f);
		style.ItemSpacing       = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
		style.WindowRounding    = 6.0f;
		style.ChildRounding     = 4.0f;
		style.FrameRounding     = 4.0f;
		style.PopupRounding     = 4.0f;
		style.ScrollbarRounding = 6.0f;
		style.GrabRounding      = 4.0f;
		style.TabRounding       = 4.0f;
		style.WindowBorderSize  = 1.0f;
		style.FrameBorderSize   = 1.0f;
		style.PopupBorderSize   = 1.0f;
	}
}
