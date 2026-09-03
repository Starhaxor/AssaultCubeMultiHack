#include "pch.h"

#include "imgui.h"

#include "theme.h"

namespace theme
{
	void ApplyHeritageCream()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* c = style.Colors;

		// Vintage palette: aged paper cream + antique gold on dark walnut.
		const ImVec4 cream      = ImVec4(0.96f, 0.91f, 0.78f, 1.00f);
		const ImVec4 creamDim   = ImVec4(0.62f, 0.56f, 0.44f, 1.00f);
		const ImVec4 walnut     = ImVec4(0.13f, 0.10f, 0.07f, 0.97f);
		const ImVec4 walnut2    = ImVec4(0.18f, 0.14f, 0.09f, 1.00f);
		const ImVec4 walnut3    = ImVec4(0.24f, 0.18f, 0.11f, 1.00f);
		const ImVec4 brass      = ImVec4(0.55f, 0.42f, 0.22f, 1.00f);
		const ImVec4 gold       = ImVec4(0.87f, 0.68f, 0.30f, 1.00f);
		const ImVec4 goldBright = ImVec4(0.98f, 0.80f, 0.42f, 1.00f);
		const ImVec4 bronze     = ImVec4(0.42f, 0.30f, 0.15f, 1.00f);
		const ImVec4 bronzeLt   = ImVec4(0.55f, 0.40f, 0.20f, 1.00f);

		c[ImGuiCol_Text]                  = cream;
		c[ImGuiCol_TextDisabled]          = creamDim;
		c[ImGuiCol_WindowBg]              = walnut;
		c[ImGuiCol_ChildBg]               = walnut2;
		c[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.15f, 0.10f, 0.98f);
		c[ImGuiCol_Border]                = brass;
		c[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_FrameBg]               = walnut3;
		c[ImGuiCol_FrameBgHovered]        = ImVec4(0.32f, 0.24f, 0.14f, 1.00f);
		c[ImGuiCol_FrameBgActive]         = bronzeLt;
		c[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.14f, 0.08f, 1.00f);
		c[ImGuiCol_TitleBgActive]         = bronze;
		c[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.15f, 0.11f, 0.07f, 1.00f);
		c[ImGuiCol_MenuBarBg]             = walnut2;
		c[ImGuiCol_ScrollbarBg]           = ImVec4(0.12f, 0.09f, 0.06f, 1.00f);
		c[ImGuiCol_ScrollbarGrab]         = brass;
		c[ImGuiCol_ScrollbarGrabHovered]  = gold;
		c[ImGuiCol_ScrollbarGrabActive]   = goldBright;
		c[ImGuiCol_CheckMark]             = goldBright;
		c[ImGuiCol_SliderGrab]            = gold;
		c[ImGuiCol_SliderGrabActive]      = goldBright;
		c[ImGuiCol_Button]                = bronze;
		c[ImGuiCol_ButtonHovered]         = bronzeLt;
		c[ImGuiCol_ButtonActive]          = gold;
		c[ImGuiCol_Header]                = bronze;
		c[ImGuiCol_HeaderHovered]         = bronzeLt;
		c[ImGuiCol_HeaderActive]          = gold;
		c[ImGuiCol_Separator]             = brass;
		c[ImGuiCol_SeparatorHovered]      = gold;
		c[ImGuiCol_SeparatorActive]       = goldBright;
		c[ImGuiCol_ResizeGrip]            = brass;
		c[ImGuiCol_ResizeGripHovered]     = gold;
		c[ImGuiCol_ResizeGripActive]      = goldBright;
		c[ImGuiCol_Tab]                   = bronze;
		c[ImGuiCol_TabHovered]            = bronzeLt;
		c[ImGuiCol_TabActive]             = gold;
		c[ImGuiCol_TabUnfocused]          = walnut3;
		c[ImGuiCol_TabUnfocusedActive]    = bronze;
		c[ImGuiCol_PlotLines]             = gold;
		c[ImGuiCol_PlotLinesHovered]      = goldBright;
		c[ImGuiCol_PlotHistogram]         = bronzeLt;
		c[ImGuiCol_PlotHistogramHovered]  = gold;
		c[ImGuiCol_TableHeaderBg]         = bronze;
		c[ImGuiCol_TableBorderStrong]     = brass;
		c[ImGuiCol_TableBorderLight]      = ImVec4(0.42f, 0.32f, 0.18f, 1.00f);
		c[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_TableRowBgAlt]         = ImVec4(0.96f, 0.91f, 0.78f, 0.06f);
		c[ImGuiCol_TextSelectedBg]        = ImVec4(0.65f, 0.50f, 0.26f, 0.60f);
		c[ImGuiCol_DragDropTarget]        = goldBright;
		c[ImGuiCol_NavHighlight]          = gold;
		c[ImGuiCol_NavWindowingHighlight] = goldBright;
		c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.05f, 0.03f, 0.02f, 0.60f);
		c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.05f, 0.03f, 0.02f, 0.60f);

		style.WindowPadding    = ImVec2(12.0f, 10.0f);
		style.FramePadding     = ImVec2(8.0f, 4.0f);
		style.ItemSpacing      = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
		style.WindowRounding   = 8.0f;
		style.ChildRounding    = 6.0f;
		style.FrameRounding    = 6.0f;
		style.PopupRounding    = 6.0f;
		style.ScrollbarRounding = 8.0f;
		style.GrabRounding     = 6.0f;
		style.TabRounding      = 6.0f;
		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize  = 1.0f;
		style.PopupBorderSize  = 1.0f;
	}
}
