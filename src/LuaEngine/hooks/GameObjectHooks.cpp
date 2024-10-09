/*
 * Copyright (C) 2010 - 2016 Forge Lua Engine <http://emudevs.com/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ForgeIncludes.h"
#include "ForgeEventMgr.h"
#include "ForgeTemplate.h"

using namespace Hooks;

#define START_HOOK(EVENT, ENTRY) \
    if (!IsEnabled())\
        return;\
    auto key = EntryKey<GameObjectEvents>(EVENT, ENTRY);\
    if (!GameObjectEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_FORGE

#define START_HOOK_WITH_RETVAL(EVENT, ENTRY, RETVAL) \
    if (!IsEnabled())\
        return RETVAL;\
    auto key = EntryKey<GameObjectEvents>(EVENT, ENTRY);\
    if (!GameObjectEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_FORGE

void Forge::OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, GameObject* pTarget)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DUMMY_EFFECT, pTarget->GetEntry());
    Push(pCaster);
    Push(spellId);
    Push(effIndex);
    Push(pTarget);
    CallAllFunctions(GameObjectEventBindings, key);
}

void Forge::UpdateAI(GameObject* pGameObject, uint32 diff)
{
    pGameObject->forgeEvents->Update(diff);
    START_HOOK(GAMEOBJECT_EVENT_ON_AIUPDATE, pGameObject->GetEntry());
    Push(pGameObject);
    Push(diff);
    CallAllFunctions(GameObjectEventBindings, key);
}

bool Forge::OnQuestAccept(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest)
{
    START_HOOK_WITH_RETVAL(GAMEOBJECT_EVENT_ON_QUEST_ACCEPT, pGameObject->GetEntry(), false);
    Push(pPlayer);
    Push(pGameObject);
    Push(pQuest);
    return CallAllFunctionsBool(GameObjectEventBindings, key);
}

bool Forge::OnQuestReward(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest, uint32 opt)
{
    START_HOOK_WITH_RETVAL(GAMEOBJECT_EVENT_ON_QUEST_REWARD, pGameObject->GetEntry(), false);
    Push(pPlayer);
    Push(pGameObject);
    Push(pQuest);
    Push(opt);
    return CallAllFunctionsBool(GameObjectEventBindings, key);
}

void Forge::GetDialogStatus(const Player* pPlayer, const GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DIALOG_STATUS, pGameObject->GetEntry());
    Push(pPlayer);
    Push(pGameObject);
    CallAllFunctions(GameObjectEventBindings, key);
}

#ifndef CLASSIC
#ifndef TBC
void Forge::OnDestroyed(GameObject* pGameObject, WorldObject* attacker)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DESTROYED, pGameObject->GetEntry());
    Push(pGameObject);
    Push(attacker);
    CallAllFunctions(GameObjectEventBindings, key);
}

void Forge::OnDamaged(GameObject* pGameObject, WorldObject* attacker)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DAMAGED, pGameObject->GetEntry());
    Push(pGameObject);
    Push(attacker);
    CallAllFunctions(GameObjectEventBindings, key);
}
#endif
#endif

void Forge::OnLootStateChanged(GameObject* pGameObject, uint32 state)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_LOOT_STATE_CHANGE, pGameObject->GetEntry());
    Push(pGameObject);
    Push(state);
    CallAllFunctions(GameObjectEventBindings, key);
}

void Forge::OnGameObjectStateChanged(GameObject* pGameObject, uint32 state)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_GO_STATE_CHANGED, pGameObject->GetEntry());
    Push(pGameObject);
    Push(state);
    CallAllFunctions(GameObjectEventBindings, key);
}

void Forge::OnSpawn(GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_SPAWN, pGameObject->GetEntry());
    Push(pGameObject);
    CallAllFunctions(GameObjectEventBindings, key);
}

void Forge::OnAddToWorld(GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_ADD, pGameObject->GetEntry());
    Push(pGameObject);
    CallAllFunctions(GameObjectEventBindings, key);
}

void Forge::OnRemoveFromWorld(GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_REMOVE, pGameObject->GetEntry());
    Push(pGameObject);
    CallAllFunctions(GameObjectEventBindings, key);
}

bool Forge::OnGameObjectUse(Player* pPlayer, GameObject* pGameObject)
{
    START_HOOK_WITH_RETVAL(GAMEOBJECT_EVENT_ON_USE, pGameObject->GetEntry(), false);
    Push(pGameObject);
    Push(pPlayer);
    return CallAllFunctionsBool(GameObjectEventBindings, key);
}
