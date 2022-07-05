#include "gui.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "proc/proc.h"
#include "mem/mem.h"
#include <thread>

int __stdcall wWinMain(HINSTANCE instance,HINSTANCE previousInstance,PWSTR arguments, int commandShow)
{
	// hack bools
	bool bHealth = false;
	bool bArmor = false;
	bool bRecoil = false;
	bool bAmmo = false;

	// hack inital value
	int newHealth = 100;
	int newArmor = 100;
	std::uint16_t fireRate = 120;
	std::uint16_t damage = 12;
	float xPos = 0;
	float yPos = 0;
	float zPos = 0;

	// initalize variables
	HANDLE hProcess = 0;

	uintptr_t moduleBaseAddr = 0;
	uintptr_t healthAddr = 0;
	uintptr_t armorAddr = 0;
	uintptr_t fireRateAddr = 0;
	uintptr_t damageAddr = 0;
	uintptr_t xPositionAddr = 0;
	uintptr_t yPositionAddr = 0;
	uintptr_t zPositionAddr = 0;

	// get handle, procId and base addy
	DWORD processId = GetProcId("ac_client.exe");

	if (processId)
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

		moduleBaseAddr = GetModuleBaseAdress(processId, "ac_client.exe");

		// localPlayer Addys
		healthAddr = mem::FindDMAAddy(hProcess,moduleBaseAddr + 0x10f4f4, {0xf8});
		armorAddr = healthAddr + 0x4;
		xPositionAddr = mem::FindDMAAddy(hProcess, moduleBaseAddr + 0x10f4f4, { 0x34 });
		yPositionAddr = xPositionAddr + 0x4;
		zPositionAddr = yPositionAddr + 0x4;

		// currentWeapon Addys
		fireRateAddr = mem::FindDMAAddy(hProcess, moduleBaseAddr + 0x10f4f4, { 0x374, 0xC,0x10A });
		damageAddr = fireRateAddr + 0x2;
	}
	

	//create gui
	gui::CreateHWindow("Cheat Menu", "Cheat Menu Class");
	gui::CreateDevice();
	gui::CreateImGui();

	while (gui::exit)
	{
		gui::BeginRender();
		gui::Render();
		
		if (ImGui::BeginTabBar("Cheat Tabs", ImGuiTabBarFlags_None))
		{
			//Player Hacks
			if (ImGui::BeginTabItem("Player Hacks"))
			{
				ImGui::Checkbox("Enable Health Hack", &bHealth);
				ImGui::SliderInt("Health", &newHealth, 1, 1000);

				ImGui::Checkbox("Enable Armor Hack", &bArmor);
				ImGui::SliderInt("Armor", &newArmor, 1, 1000);

				ImGui::EndTabItem();
			}
			//Weapon Hacks
			if (ImGui::BeginTabItem("Weapon Hacks"))
			{
				// Recoil Hack 
				ImGui::Checkbox("Enable No Recoil", &bRecoil);
				if (bRecoil)
				{
					// ret 08 = C2 0800
					mem::PatchEx((BYTE*)(moduleBaseAddr + 0x62020), (BYTE*)"\xC2\x08\x00", 3, hProcess);
				}
				else
				{
					mem::PatchEx((BYTE*)(moduleBaseAddr + 0x62020), (BYTE*)"\x55\x8B\xEC", 3, hProcess);
				}

				// Infinite Ammo
				ImGui::Checkbox("Infinite Ammo", &bAmmo);
				if (bAmmo)
				{
					mem::NopEx((BYTE*)(moduleBaseAddr + 0x637E9), 2, hProcess);
				}
				else
				{
					// FF 0E = dec [esi]
					mem::PatchEx((BYTE*)(moduleBaseAddr + 0x637E9), (BYTE*)"\xFF\x0E", 2, hProcess);
				}

				// Change Fire Rate, this will call only if the value is changed. This will only need to be changed once.
				if (ImGui::SliderInt("Fire Rate", (int*)&fireRate, 1, 500))
				{
					// I have to find the address again since the weapon could of been swapped.
					fireRateAddr = mem::FindDMAAddy(hProcess, moduleBaseAddr + 0x10f4f4, { 0x374, 0xC,0x10A });
					mem::PatchEx((BYTE*)fireRateAddr, (BYTE*)&fireRate, sizeof(fireRate), hProcess);
				}
				// Change Fire Rate, this will call only if the value is changed. This will only need to be changed once.
				if (ImGui::SliderInt("Weapon Damage", (int*)&damage, 1, 500))
				{
					damageAddr = mem::FindDMAAddy(hProcess, moduleBaseAddr + 0x10f4f4, { 0x374, 0xC,0x10C });
					mem::PatchEx((BYTE*)damageAddr, (BYTE*)&damage, sizeof(damage), hProcess);
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Teleport"))
			{
				if (ImGui::Button("Save Position"))
				{
					ReadProcessMemory(hProcess, (BYTE*)xPositionAddr, &xPos, sizeof(xPos), 0);
					ReadProcessMemory(hProcess, (BYTE*)yPositionAddr, &yPos, sizeof(yPos), 0);
					ReadProcessMemory(hProcess, (BYTE*)zPositionAddr, &zPos, sizeof(zPos), 0);
				}
				if (ImGui::Button("Teleport to Saved Pos"))
				{
					mem::PatchEx((BYTE*)xPositionAddr, (BYTE*)&xPos, sizeof(xPos), hProcess);
					mem::PatchEx((BYTE*)yPositionAddr, (BYTE*)&yPos, sizeof(yPos), hProcess);
					mem::PatchEx((BYTE*)zPositionAddr, (BYTE*)&zPos, sizeof(zPos), hProcess);
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar;
		}

		// Hack Code for continuous writes/reads
		if (bHealth)
		{
			mem::PatchEx((BYTE*)healthAddr, (BYTE*)&newHealth, sizeof(newHealth), hProcess);
		}
		if (bArmor)
		{
			mem::PatchEx((BYTE*)armorAddr, (BYTE*)&newArmor, sizeof(newArmor), hProcess);
		}

		ImGui::End();
		gui::EndRender();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// destroy gui
	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();


	return EXIT_SUCCESS;
}