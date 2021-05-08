#include "Papyrus/Papyrus.h"

#include "Papyrus/ConsoleUtil.h"

#undef BIND

namespace Papyrus
{
	bool F4SEAPI Bind(RE::BSScript::IVirtualMachine* a_vm)
	{
		if (!a_vm) {
			return false;
		}

		ConsoleUtil::Bind(*a_vm);

		logger::debug("bound all scripts"sv);
		return true;
	}
}
