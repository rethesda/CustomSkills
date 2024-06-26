#include "SkillUse.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void SkillUse::WriteHooks()
	{
		UseSkillPatch();
		ConstructibleObjectBottomBarPatch();
		ConstructibleObjectCreationPatch();
	}

	void SkillUse::UseSkillPatch()
	{
		auto vtbl = REL::Relocation<std::uintptr_t>(RE::Offset::PlayerCharacter::Vtbl);

		_UseSkill = vtbl.write_vfunc(247, &SkillUse::UseSkill);
	}

	[[nodiscard]] static std::shared_ptr<Skill> GetWorkbenchSkill(
		const RE::TESFurniture* a_furniture)
	{
		for (const RE::BGSKeyword* const keyword :
			 std::span(a_furniture->keywords, a_furniture->numKeywords)) {

			if (!keyword)
				continue;

			const auto str = std::string_view(keyword->formEditorID);
			static constexpr auto prefix = "CustomSkillWorkbench_"sv;
			if (str.size() <= prefix.size() ||
				::_strnicmp(str.data(), prefix.data(), prefix.size()) != 0) {
				continue;
			}

			if (const auto skill = CustomSkillsManager::FindSkill(str.substr(prefix.size()))) {
				return skill;
			}
		}

		return nullptr;
	}

	[[nodiscard]] static std::shared_ptr<Skill> GetObjectSkill(const RE::TESForm* a_object)
	{
		const auto keywordForm = skyrim_cast<const RE::BGSKeywordForm*>(a_object);
		if (!keywordForm)
			return nullptr;

		for (const RE::BGSKeyword* const keyword :
			 std::span(keywordForm->keywords, keywordForm->numKeywords)) {

			if (!keyword)
				continue;

			const auto str = std::string_view(keyword->formEditorID);
			static constexpr auto prefix = "CustomSkillAdvance_"sv;
			if (str.size() <= prefix.size() ||
				::_strnicmp(str.data(), prefix.data(), prefix.size()) != 0) {
				continue;
			}

			if (const auto skill = CustomSkillsManager::FindSkill(str.substr(prefix.size()))) {
				return skill;
			}
		}

		return nullptr;
	}

	static bool UpdateBottomBar(RE::CraftingSubMenus::ConstructibleObjectMenu* a_menu)
	{
		const auto skill = GetWorkbenchSkill(a_menu->furniture);
		if (!skill)
			return false;

		std::array<RE::GFxValue, 3> craftingInfo;
		craftingInfo[0].SetString(skill->GetName());
		craftingInfo[1].SetNumber(skill->GetLevel());
		craftingInfo[2].SetNumber(skill->GetProgressPercent());

		a_menu->bottomBarInfo.Invoke("UpdateCraftingInfo", craftingInfo);

		return true;
	}

	void SkillUse::ConstructibleObjectBottomBarPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::CraftingSubMenus::ConstructibleObjectMenu::UpdateBottomBar,
			0x37D);
		REL::make_pattern<"80 F9 11 77 0D">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
			{
				Xbyak::Label customSkill;
				Xbyak::Label noSkill;
				Xbyak::Label funcLbl;

				cmp(cl, 0x11);
				ja(customSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x5);

				L(customSkill);
				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				cmp(al, 0);
				jz(noSkill);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0xD);

				L(noSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x12);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(hook.address(), reinterpret_cast<std::uintptr_t>(&UpdateBottomBar));
		patch->ready();

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(hook.address(), patch->getCode());
	}

	static void UseWorkbench(const RE::TESFurniture* a_furniture, float a_amount)
	{
		if (const auto skill = GetWorkbenchSkill(a_furniture)) {
			skill->Advance(a_amount);
		}
	}

	void SkillUse::ConstructibleObjectCreationPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::CraftingSubMenus::ConstructibleObjectMenu::CreationConfirmed,
			0x83);
		REL::make_pattern<"83 F8 11 77 1E">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
			{
				Xbyak::Label customSkill;
				Xbyak::Label funcLbl;

				cmp(eax, 0x11);
				ja(customSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x5);

				L(customSkill);
				mov(rcx,
					ptr[r15 + offsetof(RE::CraftingSubMenus::ConstructibleObjectMenu, furniture)]);
				movaps(xmm1, xmm0);
				call(ptr[rip + funcLbl]);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x23);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(hook.address(), reinterpret_cast<std::uintptr_t>(&UseWorkbench));
		patch->ready();

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(hook.address(), patch->getCode());
	}

	void SkillUse::UseSkill(
		RE::PlayerCharacter* a_player,
		RE::ActorValue a_skill,
		float a_amount,
		RE::TESForm* a_advanceObject,
		std::uint32_t a_advanceAction)
	{
		if (a_skill == RE::ActorValue::kNone) {
			if (const auto skill = GetObjectSkill(a_advanceObject)) {
				skill->Advance(a_amount);
				return;
			}
		}

		return _UseSkill(a_player, a_skill, a_amount, a_advanceObject, a_advanceAction);
	}
}
