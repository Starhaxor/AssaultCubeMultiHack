#include "pch.h"

#include "imgui.h"

#include "menu_sections.h"
#include "menu_state.h"
#include "game.h"

namespace menu
{
	// ----- Player: dogrulanmis kod yamalari (godmode/onehit/ammo) -----
	void RenderPlayerSection()
	{
		if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("God Mode", &bGodMode);
			ImGui::Checkbox("One Hit Kill", &bOneHit);
			ImGui::Checkbox("Unlimited Ammo", &bUnlimitedAmmo);
		}
	}

	// ----- Aimbot: RMB basiliyken en yakin dusmana kitlenir -----
	void RenderAimbotSection()
	{
		if (ImGui::CollapsingHeader("Aimbot", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!game::EntityConfigured())
			{
				ImGui::BeginDisabled();
				ImGui::Checkbox("Enable Aimbot (hold RMB)", &bAimbot);
				ImGui::EndDisabled();
				ImGui::TextDisabled("bu surum icin adres gerekli");
				return;
			}
			ImGui::Checkbox("Enable Aimbot (hold RMB)", &bAimbot);
			ImGui::Checkbox("Team Check", &bTeamCheck);
			ImGui::SliderFloat("FOV", &fovScale, 5.0f, 180.0f, "%.0f");
			ImGui::SliderFloat("Smooth", &aimSmooth, 1.0f, 20.0f, "%.1f");
		}
	}

	// ----- Visuals: ESP + crosshair -----
	void RenderVisualsSection()
	{
		if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!game::EntityConfigured())
			{
				ImGui::BeginDisabled();
				ImGui::Checkbox("ESP", &bESP);
				ImGui::EndDisabled();
				ImGui::TextDisabled("ESP icin adres gerekli");
			}
			else
			{
				ImGui::Checkbox("ESP", &bESP);
				ImGui::Checkbox("ESP Box", &bEspBox);
				ImGui::Checkbox("ESP Health Bar", &bEspHealth);
				ImGui::Checkbox("ESP Names", &bEspName);
				ImGui::Checkbox("ESP Snaplines", &bEspLines);
			}

			ImGui::Separator();
			ImGui::Checkbox("Crosshair", &bCrosshair);
			ImGui::SliderFloat("Size", &crosshairSize, 2.0f, 30.0f, "%.0f");
			ImGui::ColorEdit3("Color", crosshairColor);
		}
	}

	// ----- Movement: fly + teleport -----
	void RenderMovementSection()
	{
		if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!game::EntityConfigured())
			{
				ImGui::BeginDisabled();
				ImGui::Checkbox("Fly (SPACE up / CTRL down)", &bFly);
				ImGui::Checkbox("Speed Hack", &bSpeedHack);
				if (ImGui::Button("Konumu Kaydet")) {}
				ImGui::SameLine();
				if (ImGui::Button("Isinlan")) {}
				ImGui::EndDisabled();
				ImGui::TextDisabled("bu surum icin adres gerekli");
				return;
			}

			ImGui::Checkbox("Fly (SPACE up / CTRL down)", &bFly);
			ImGui::SliderFloat("Fly Speed", &flySpeed, 1.0f, 20.0f, "%.1f");

			if (ImGui::Button("Konumu Kaydet"))
			{
				uintptr_t local = game::LocalPlayer();
				if (local && game::ReadHead(local, savedPos) &&
					game::ReadPos(local, savedFeet))
					bHasTeleport = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Isinlan") && bHasTeleport)
			{
				uintptr_t local = game::LocalPlayer();
				if (local)
				{
					game::WritePos(local, savedFeet);
					game::WriteHead(local, savedPos);
				}
			}
			if (!bHasTeleport)
				ImGui::TextDisabled("kayitli konum yok");
		}
	}

	// ----- Misc: rapid fire + tani satirlari -----
	void RenderMiscSection()
	{
		if (ImGui::CollapsingHeader("Misc", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Rapid Fire (semi-auto)", &bRapidFire);
			ImGui::SliderInt("Click Rate ms", &rapidRate, 30, 200);
			ImGui::TextDisabled("INSERT: menuyu ac/kapa");
		}
	}
}
