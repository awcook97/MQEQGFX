#pragma once

namespace eqgfx::ui {

void CastGem(int gem);
void ForgetGem(int gem);
void InspectSpell(int spellId);
void InspectItem(int slot, int bagSlot = -1);
void RemoveMyBuff(int slot, int spellId);
void RemovePetBuff(int slot, int spellId);
void TurnHotbarPage(int bar, int delta);

void RunQueuedActions();

} // namespace eqgfx::ui
