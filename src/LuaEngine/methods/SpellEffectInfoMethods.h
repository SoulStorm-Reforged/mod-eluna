/*
* Copyright (C) 2010 - 2024 Forge Lua Engine <https://forgeluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef SPELLEFFECTINFOMETHODS_H
#define SPELLEFFECTINFOMETHODS_H

/***
 *
 *
 * Inherits all methods from: none
 */
namespace LuaSpellEffectInfo
{
    int GetMiscValue(lua_State* L, SpellEffectInfo* spell_effect_info)
    {
        Forge::Push(L, spell_effect_info->MiscValue);
        return 1;
    }

    int GetMiscValueB(lua_State* L, SpellEffectInfo* spell_effect_info)
    {
        Forge::Push(L, spell_effect_info->MiscValueB);
        return 1;
    }

    int GetEffect(lua_State* L, SpellEffectInfo* spell_effect_info)
    {
        Forge::Push(L, spell_effect_info->Effect);
        return 1;
    }

    ForgeRegister<SpellEffectInfo> SpellEffectInfoMethods[] =
    {
        // Getters
        { "GetMiscValue", &LuaSpellEffectInfo::GetMiscValue },
        { "GetMiscValueB", &LuaSpellEffectInfo::GetMiscValueB },
        { "GetEffect", &LuaSpellEffectInfo::GetEffect },

        // Setters

        // Boolean


        // Other

        { NULL, NULL }
    };
}
#endif
