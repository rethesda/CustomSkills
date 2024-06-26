#pragma once

#include "CImageController.h"
#include "Skill.h"

namespace CustomSkills
{
	class CustomSkillsManager final
	{
	public:
		static void Initialize();
		static void LoadSkills();

	public:
		enum class MenuState : uint8_t
		{
			None = 0,
			WaitingToOpen = 1,
			Open = 2,
			WaitingToClose = 3,
		};

		static void CloseStatsMenu();

		static void OpenStatsMenu(std::shared_ptr<SkillGroup> a_group);

		static void ShowTrainingMenu(
			std::shared_ptr<Skill> a_skill,
			std::uint32_t a_maxLevel,
			RE::Actor* a_trainer);

		static void NotifyOpeningSkills();

		static bool IsMenuControlsEnabled();

		static bool IsStatsMenuOpen();

		static std::uint32_t GetCurrentSkillCount();

		static std::uint32_t GetCurrentPerkPoints();

		static void SetCurrentPerkPoints(std::uint8_t a_value);

		static void SetMenuState(MenuState a_state);

		static void SetTrainingState(MenuState a_state);

		static void SetBeastMode(bool a_beastMode);

		static bool IsOurMenuMode();

		static bool IsBeastMode();

		static bool IsOurTrainingMode();

		static float GetSkillLevel(RE::ActorValue a_skill);

		static float GetSkillProgressPercent(RE::ActorValue a_skill);

		static float GetBaseSkillLevel(RE::ActorValue a_skill);

		static std::shared_ptr<SkillGroup> FindSkillMenu(std::string_view a_key);

		static std::shared_ptr<Skill> FindSkill(std::string_view a_key);

		static auto FindSkillOrigin(std::string_view a_key)
			-> std::pair<std::shared_ptr<SkillGroup>, std::size_t>;

		static std::shared_ptr<Skill> FindSkillFromGlobalLevel(RE::TESGlobal* a_global);

		static std::shared_ptr<Skill> GetCurrentSkill(RE::ActorValue a_value);

		static void UpdateSkills();

		static void UpdateMenu();

		static void UpdateVars();

		static void UpdateTraining();

		inline static REL::Relocation<bool*> IsSingleSkillMode;
		inline static REL::Relocation<bool*> UseBeastSkillInfo;
		inline static REL::Relocation<std::uint32_t*> CameraRightPoint;

		using SkillLocation = std::pair<std::shared_ptr<SkillGroup>, std::size_t>;
		inline static util::istring_map<std::shared_ptr<SkillGroup>> _groupIds;
		inline static util::istring_map<SkillLocation> _skillIds;
		inline static std::map<RE::TESGlobal*, std::shared_ptr<Skill>> _requirementSkills;

		inline static std::shared_ptr<SkillGroup> _menuSkills = nullptr;
		inline static std::vector<CImageController> _cImageControllers;
		inline static MenuState _menuState = MenuState::None;
		inline static RE::BSFixedString _colorOfSkillNormal = "#FFFFFF";

		inline static std::shared_ptr<Skill> _trainingSkill = nullptr;
		inline static std::uint32_t _trainingMax = 0;
		inline static MenuState _trainingState = MenuState::None;
	};
}
