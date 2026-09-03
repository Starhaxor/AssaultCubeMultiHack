#include "pch.h"
#include "imgui.h"
#include "menu_state.h"
#include "menu_visuals.h"

namespace menu
{
	void RenderVisualsSection()
	{
		if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen))
			ImGui::Checkbox("ESP", &bESP);
	}
}
