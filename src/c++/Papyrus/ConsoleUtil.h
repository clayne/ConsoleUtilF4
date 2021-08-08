#pragma once

#include "Papyrus/Common.h"

namespace Papyrus::ConsoleUtil
{
	inline void ExecuteCommand(std::monostate, std::string_view a_command)
	{
		RE::Console::ExecuteCommand(a_command.data());
	}

	inline RE::TESObjectREFR* GetSelectedReference(std::monostate)
	{
		const auto ref = RE::Console::GetPickRef();
		return ref.get().get();
	}

	inline void SetSelectedReference(std::monostate, RE::TESObjectREFR* a_reference)
	{
		const auto ui = RE::UI::GetSingleton();
		const auto console = ui ? ui->GetMenu<RE::Console>() : nullptr;
		if (console) {
			RE::ObjectRefHandle handle{ a_reference };
			console->SetCurrentPickREFR(&handle);
		}
	}

	inline std::string_view ReadMessage(std::monostate)
	{
		const auto console = RE::ConsoleLog::GetSingleton();
		return console ? console->buffer : ""sv;
	}

	inline void PrintMessage(std::monostate, std::string_view a_message)
	{
		const auto console = RE::ConsoleLog::GetSingleton();
		console->AddString(
			fmt::format(
				FMT_STRING("{}\n"),
				a_message)
				.data());
	}

	inline void Bind(RE::BSScript::IVirtualMachine& a_vm)
	{
		const auto obj = "ConsoleUtil"sv;

		BIND(ExecuteCommand);
		BIND(GetSelectedReference);
		BIND(SetSelectedReference);
		BIND(ReadMessage);
		BIND(PrintMessage);

		logger::debug("bound {} script"sv, obj);
	}
}
