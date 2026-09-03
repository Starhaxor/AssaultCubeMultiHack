#include "pch.h"
#include "imgui.h"
#include "menu_state.h"
#include "menu_misc.h"

namespace menu
{
	void RenderMiscSection()
	{
		if (ImGui::CollapsingHeader("Misc", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("No Recoil", &bNoRecoil);
			ImGui::Checkbox("Rapid Fire", &bRapidFire);
		}
	}
}
