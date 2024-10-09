/*
* Copyright (C) 2010 - 2016 Forge Lua Engine <http://emudevs.com/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef BATTLEGROUNDMETHODS_H
#define BATTLEGROUNDMETHODS_H

/***
 * Contains the state of a battleground, e.g. Warsong Gulch, Arathi Basin, etc.
 *
 * Inherits all methods from: none
 */
namespace LuaBattleGround
{
    /**
     * Returns the name of the [BattleGround].
     *
     * @return string name
     */
    int GetName(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetName());
        return 1;
    }

    /**
     * Returns the amount of alive players in the [BattleGround] by the team ID.
     *
     * @param [Team] team : team ID
     * @return uint32 count
     */
    int GetAlivePlayersCountByTeam(lua_State* L, BattleGround* bg)
    {
        uint32 team = Forge::CHECKVAL<uint32>(L, 2);

#ifndef AZEROTHCORE
        Forge::Push(L, bg->GetAlivePlayersCountByTeam((Team)team));
#else
        Forge::Push(L, bg->GetAlivePlayersCountByTeam((TeamId)team));
#endif
        return 1;
    }

    /**
     * Returns the [Map] of the [BattleGround].
     *
     * @return [Map] map
     */
    int GetMap(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetBgMap());
        return 1;
    }

    /**
     * Returns the bonus honor given by amount of kills in the specific [BattleGround].
     *
     * @param uint32 kills : amount of kills
     * @return uint32 bonusHonor
     */
    int GetBonusHonorFromKillCount(lua_State* L, BattleGround* bg)
    {
        uint32 kills = Forge::CHECKVAL<uint32>(L, 2);

        Forge::Push(L, bg->GetBonusHonorFromKill(kills));
        return 1;
    }

#ifndef AZEROTHCORE
    /**
     * Returns the bracket ID of the specific [BattleGround].
     *
     * @return [BattleGroundBracketId] bracketId
     */
    int GetBracketId(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetBracketId());
        return 1;
    }
#endif

    /**
     * Returns the end time of the [BattleGround].
     *
     * @return uint32 endTime
     */
    int GetEndTime(lua_State* L, BattleGround* bg)
    {
#ifdef CATA
        Forge::Push(L, bg->GetRemainingTime());
#else
        Forge::Push(L, bg->GetEndTime());
#endif
        return 1;
    }

    /**
     * Returns the amount of free slots for the selected team in the specific [BattleGround].
     *
     * @param [Team] team : team ID
     * @return uint32 freeSlots
     */
    int GetFreeSlotsForTeam(lua_State* L, BattleGround* bg)
    {
        uint32 team = Forge::CHECKVAL<uint32>(L, 2);

#ifndef AZEROTHCORE
        Forge::Push(L, bg->GetFreeSlotsForTeam((Team)team));
#else
        Forge::Push(L, bg->GetFreeSlotsForTeam((TeamId)team));
#endif
        return 1;
    }

    /**
     * Returns the instance ID of the [BattleGround].
     *
     * @return uint32 instanceId
     */
    int GetInstanceId(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetInstanceID());
        return 1;
    }

    /**
     * Returns the map ID of the [BattleGround].
     *
     * @return uint32 mapId
     */
    int GetMapId(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetMapId());
        return 1;
    }

    /**
     * Returns the type ID of the [BattleGround].
     *
     * @return [BattleGroundTypeId] typeId
     */
    int GetTypeId(lua_State* L, BattleGround* bg)
    {
#ifndef AZEROTHCORE
        Forge::Push(L, bg->GetTypeID());
#else
        Forge::Push(L, bg->GetBgTypeID());
#endif
        return 1;
    }

    /**
     * Returns the max allowed [Player] level of the specific [BattleGround].
     *
     * @return uint32 maxLevel
     */
    int GetMaxLevel(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetMaxLevel());
        return 1;
    }

    /**
     * Returns the minimum allowed [Player] level of the specific [BattleGround].
     *
     * @return uint32 minLevel
     */
    int GetMinLevel(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetMinLevel());
        return 1;
    }

    /**
     * Returns the maximum allowed [Player] count of the specific [BattleGround].
     *
     * @return uint32 maxPlayerCount
     */
    int GetMaxPlayers(lua_State* L, BattleGround* bg)
    {
#ifndef AZEROTHCORE
        Forge::Push(L, bg->GetMaxPlayers());
#else
        Forge::Push(L, bg->GetMaxPlayersPerTeam() * 2);
#endif
        return 1;
    }

    /**
     * Returns the minimum allowed [Player] count of the specific [BattleGround].
     *
     * @return uint32 minPlayerCount
     */
    int GetMinPlayers(lua_State* L, BattleGround* bg)
    {
#ifndef AZEROTHCORE
        Forge::Push(L, bg->GetMinPlayers());
#else
        Forge::Push(L, bg->GetMaxPlayersPerTeam() * 2);
#endif
        return 1;
    }

    /**
     * Returns the maximum allowed [Player] count per team of the specific [BattleGround].
     *
     * @return uint32 maxTeamPlayerCount
     */
    int GetMaxPlayersPerTeam(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetMaxPlayersPerTeam());
        return 1;
    }

    /**
     * Returns the minimum allowed [Player] count per team of the specific [BattleGround].
     *
     * @return uint32 minTeamPlayerCount
     */
    int GetMinPlayersPerTeam(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetMinPlayersPerTeam());
        return 1;
    }

    /**
     * Returns the winning team of the specific [BattleGround].
     *
     * @return [Team] team
     */
    int GetWinner(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetWinner());
        return 1;
    }

    /**
     * Returns the status of the specific [BattleGround].
     *
     * @return [BattleGroundStatus] status
     */
    int GetStatus(lua_State* L, BattleGround* bg)
    {
        Forge::Push(L, bg->GetStatus());
        return 1;
    }
};
#endif
