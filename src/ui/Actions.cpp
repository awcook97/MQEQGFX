#include "Actions.h"

#include <mq/Plugin.h>

#include <mutex>
#include <vector>

namespace eqgfx::ui {

namespace {

enum class Kind { CastGem, ForgetGem, InspectSpell, InspectItem, RemoveMyBuff, RemovePetBuff, TurnPage };

struct Action
{
	Kind kind;
	int  a;
	int  b;
};

std::mutex          s_queueMutex;
std::vector<Action> s_queue;

void Queue(Kind kind, int a, int b = 0)
{
	std::scoped_lock lock(s_queueMutex);
	s_queue.push_back(Action{ kind, a, b });
}

CHotButtonWnd* Hotbar(int bar)
{
	return bar >= 0 && bar < MAX_HOTBUTTON_WNDS ? pHotButtonWnds[bar].get() : nullptr;
}

void Run(const Action& act)
{
	PlayerClient* me = pLocalPlayer.get();
	if (!me || !pLocalPC)
		return;

	switch (act.kind)
	{
	case Kind::CastGem:
		if (cmdCast && GetMemorizedSpell(act.a) > 0)
		{
			char arg[8];
			sprintf_s(arg, "%d", act.a + 1);
			cmdCast(me, arg);
		}
		break;

	case Kind::ForgetGem:
		if (pCastSpellWnd && GetMemorizedSpell(act.a) > 0)
			pCastSpellWnd->ForgetMemorizedSpell(act.a);
		break;

	case Kind::InspectSpell:
		if (pSpellDisplayManager && GetSpellByID(act.a))
			pSpellDisplayManager->ShowSpell(act.a, true, true, SpellDisplayType_SpellBookWnd);
		break;

	case Kind::InspectItem:
		if (pItemDisplayManager)
		{
			ItemPtr item = pLocalPC->GetInventorySlot(act.a);
			if (item && act.b >= 0)
				item = item->GetHeldItem(act.b);
			if (item)
				pItemDisplayManager->ShowItem(item);
		}
		break;

	case Kind::RemoveMyBuff:
		if (PcProfile* profile = GetPcProfile())
			if (act.a >= 0 && act.a < profile->GetMaxEffects() && profile->GetEffect(act.a).SpellID == act.b)
				pLocalPC->RemoveBuffEffect(act.a, me->SpawnID);
		break;

	case Kind::RemovePetBuff:
		if (pPetInfoWnd && act.a >= 0 && act.a < pPetInfoWnd->GetMaxBuffs() && pPetInfoWnd->GetBuff(act.a) == act.b)
			pLocalPC->RemovePetEffect(act.a);
		break;

	case Kind::TurnPage:
		if (CHotButtonWnd* wnd = Hotbar(act.a))
		{
			CXWnd* button = act.b < 0
				? (wnd->PageUpButton ? wnd->PageUpButton : wnd->PageLeftButton)
				: (wnd->PageDownButton ? wnd->PageDownButton : wnd->PageRightButton);
			if (button)
				wnd->WndNotification(button, XWM_LCLICK, nullptr);
		}
		break;
	}
}

} // namespace

void CastGem(int gem)                         { Queue(Kind::CastGem, gem); }
void ForgetGem(int gem)                       { Queue(Kind::ForgetGem, gem); }
void InspectSpell(int spellId)                { Queue(Kind::InspectSpell, spellId); }
void InspectItem(int slot, int bagSlot)       { Queue(Kind::InspectItem, slot, bagSlot); }
void RemoveMyBuff(int slot, int spellId)      { Queue(Kind::RemoveMyBuff, slot, spellId); }
void RemovePetBuff(int slot, int spellId)     { Queue(Kind::RemovePetBuff, slot, spellId); }
void TurnHotbarPage(int bar, int delta)       { Queue(Kind::TurnPage, bar, delta); }

void RunQueuedActions()
{
	std::vector<Action> run;
	{
		std::scoped_lock lock(s_queueMutex);
		if (s_queue.empty())
			return;
		run.swap(s_queue);
	}
	for (const Action& act : run)
		Run(act);
}

} // namespace eqgfx::ui
