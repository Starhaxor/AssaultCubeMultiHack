#include "pch.h"
#include "imgui.h"
#include "menu_state.h"
#include "menu_aimbot.h"

namespace menu
{
	void RenderAimbotSection()
	{
		if (ImGui::CollapsingHeader("Aimbot", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Enable Aimbot", &bAimbot);
			ImGui::SliderFloat("FOV", &fovScale, 1.0f, 360.0f, "%.0f");
		}
	}
}
