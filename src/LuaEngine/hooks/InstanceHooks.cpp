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
#include "ForgeTemplate.h"
#include "ForgeInstanceAI.h"

using namespace Hooks;

#define START_HOOK(EVENT, AI) \
    if (!IsEnabled())\
        return;\
    auto mapKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetId());\
    auto instanceKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetInstanceId());\
    if (!MapEventBindings->HasBindingsFor(mapKey) && !InstanceEventBindings->HasBindingsFor(instanceKey))\
        return;\
    LOCK_FORGE;\
    PushInstanceData(L, AI);\
    Push(AI->instance)

#define START_HOOK_WITH_RETVAL(EVENT, AI, RETVAL) \
    if (!IsEnabled())\
        return RETVAL;\
    auto mapKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetId());\
    auto instanceKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetInstanceId());\
    if (!MapEventBindings->HasBindingsFor(mapKey) && !InstanceEventBindings->HasBindingsFor(instanceKey))\
        return RETVAL;\
    LOCK_FORGE;\
    PushInstanceData(L, AI);\
    Push(AI->instance)

void Forge::OnInitialize(ForgeInstanceAI* ai)
{
    START_HOOK(INSTANCE_EVENT_ON_INITIALIZE, ai);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void Forge::OnLoad(ForgeInstanceAI* ai)
{
    START_HOOK(INSTANCE_EVENT_ON_LOAD, ai);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void Forge::OnUpdateInstance(ForgeInstanceAI* ai, uint32 diff)
{
    START_HOOK(INSTANCE_EVENT_ON_UPDATE, ai);
    Push(diff);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void Forge::OnPlayerEnterInstance(ForgeInstanceAI* ai, Player* player)
{
    START_HOOK(INSTANCE_EVENT_ON_PLAYER_ENTER, ai);
    Push(player);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void Forge::OnCreatureCreate(ForgeInstanceAI* ai, Creature* creature)
{
    START_HOOK(INSTANCE_EVENT_ON_CREATURE_CREATE, ai);
    Push(creature);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void Forge::OnGameObjectCreate(ForgeInstanceAI* ai, GameObject* gameobject)
{
    START_HOOK(INSTANCE_EVENT_ON_GAMEOBJECT_CREATE, ai);
    Push(gameobject);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

bool Forge::OnCheckEncounterInProgress(ForgeInstanceAI* ai)
{
    START_HOOK_WITH_RETVAL(INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS, ai, false);
    return CallAllFunctionsBool(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}
