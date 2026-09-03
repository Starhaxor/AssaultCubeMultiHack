#pragma once

// ============================================================================
// MENU OGELERI BURADA (menu_sections.cpp)
// ----------------------------------------------------------------------------
// Ekrandaki tum checkbox / slider / buton bu dosyadadir.
// Yeni menu ogesi eklemek: ilgili Render*Section fonksiyonuna bir
// ImGui satiri ekle (degiskeni once menu_state.h/.cpp'ye ekle).
// ============================================================================
namespace menu
{
	void RenderPlayerSection();
	void RenderAimbotSection();
	void RenderVisualsSection();
	void RenderMovementSection();
	void RenderMiscSection();
}
