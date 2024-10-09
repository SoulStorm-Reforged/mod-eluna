/*
* Copyright (C) 2010 - 2016 Forge Lua Engine <http://emudevs.com/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

extern "C"
{
#include "lua.h"
};

// Forge
#include "LuaEngine.h"
#include "ForgeEventMgr.h"
#include "ForgeIncludes.h"
#include "ForgeTemplate.h"
#include "ForgeUtility.h"

// Method includes
#include "GlobalMethods.h"
#include "ObjectMethods.h"
#include "WorldObjectMethods.h"
#include "UnitMethods.h"
#include "PlayerMethods.h"
#include "CreatureMethods.h"
#include "GroupMethods.h"
#include "GuildMethods.h"
#include "GameObjectMethods.h"
#include "ForgeQueryMethods.h"
#include "AuraMethods.h"
#include "ItemMethods.h"
#include "LootMethods.h"
#include "WorldPacketMethods.h"
#include "SpellMethods.h"
#include "SpellEffectInfoMethods.h"
#include "SpellInfoMethods.h"
#include "QuestMethods.h"
#include "MapMethods.h"
#include "CorpseMethods.h"
#include "VehicleMethods.h"
#include "BattleGroundMethods.h"
#include "ChatHandlerMethods.h"
#include "AchievementMethods.h"
#include "ItemTemplateMethods.h"
#include "RollMethods.h"



ForgeRegister<Object> ObjectMethods[] =
{
    // Getters
    { "GetEntry", &LuaObject::GetEntry },
    { "GetGUID", &LuaObject::GetGUID },
    { "GetGUIDLow", &LuaObject::GetGUIDLow },
    { "GetInt32Value", &LuaObject::GetInt32Value },
    { "GetUInt32Value", &LuaObject::GetUInt32Value },
    { "GetFloatValue", &LuaObject::GetFloatValue },
    { "GetByteValue", &LuaObject::GetByteValue },
    { "GetUInt16Value", &LuaObject::GetUInt16Value },
    { "GetUInt64Value", &LuaObject::GetUInt64Value },
    { "GetScale", &LuaObject::GetScale },
    { "GetTypeId", &LuaObject::GetTypeId },

    // Setters
    { "SetInt32Value", &LuaObject::SetInt32Value },
    { "SetUInt32Value", &LuaObject::SetUInt32Value },
    { "UpdateUInt32Value", &LuaObject::UpdateUInt32Value },
    { "SetFloatValue", &LuaObject::SetFloatValue },
    { "SetByteValue", &LuaObject::SetByteValue },
    { "SetUInt16Value", &LuaObject::SetUInt16Value },
    { "SetInt16Value", &LuaObject::SetInt16Value },
    { "SetUInt64Value", &LuaObject::SetUInt64Value },
    { "SetScale", &LuaObject::SetScale },
    { "SetFlag", &LuaObject::SetFlag },

    // Boolean
    { "IsInWorld", &LuaObject::IsInWorld },
    { "IsPlayer", &LuaObject::IsPlayer },
    { "HasFlag", &LuaObject::HasFlag },

    // Other
    { "ToGameObject", &LuaObject::ToGameObject },
    { "ToUnit", &LuaObject::ToUnit },
    { "ToCreature", &LuaObject::ToCreature },
    { "ToPlayer", &LuaObject::ToPlayer },
    { "ToCorpse", &LuaObject::ToCorpse },
    { "RemoveFlag", &LuaObject::RemoveFlag },

    { NULL, NULL }
};

ForgeRegister<WorldObject> WorldObjectMethods[] =
{
    // Getters
    { "GetName", &LuaWorldObject::GetName },
    { "GetMap", &LuaWorldObject::GetMap },
#if (!defined(TBC) && !defined(CLASSIC))
    { "GetPhaseMask", &LuaWorldObject::GetPhaseMask },
    { "SetPhaseMask", &LuaWorldObject::SetPhaseMask },
#endif
    { "GetInstanceId", &LuaWorldObject::GetInstanceId },
    { "GetAreaId", &LuaWorldObject::GetAreaId },
    { "GetZoneId", &LuaWorldObject::GetZoneId },
    { "GetMapId", &LuaWorldObject::GetMapId },
    { "GetX", &LuaWorldObject::GetX },
    { "GetY", &LuaWorldObject::GetY },
    { "GetZ", &LuaWorldObject::GetZ },
    { "GetO", &LuaWorldObject::GetO },
    { "GetLocation", &LuaWorldObject::GetLocation },
    { "GetPlayersInRange", &LuaWorldObject::GetPlayersInRange },
    { "GetCreaturesInRange", &LuaWorldObject::GetCreaturesInRange },
    { "GetGameObjectsInRange", &LuaWorldObject::GetGameObjectsInRange },
    { "GetNearestPlayer", &LuaWorldObject::GetNearestPlayer },
    { "GetNearestGameObject", &LuaWorldObject::GetNearestGameObject },
    { "GetNearestCreature", &LuaWorldObject::GetNearestCreature },
    { "GetNearObject", &LuaWorldObject::GetNearObject },
    { "GetNearObjects", &LuaWorldObject::GetNearObjects },
    { "GetDistance", &LuaWorldObject::GetDistance },
    { "GetExactDistance", &LuaWorldObject::GetExactDistance },
    { "GetDistance2d", &LuaWorldObject::GetDistance2d },
    { "GetExactDistance2d", &LuaWorldObject::GetExactDistance2d },
    { "GetRelativePoint", &LuaWorldObject::GetRelativePoint },
    { "GetAngle", &LuaWorldObject::GetAngle },

    // Boolean
    { "IsWithinLoS", &LuaWorldObject::IsWithinLoS },
    { "IsInMap", &LuaWorldObject::IsInMap },
    { "IsWithinDist3d", &LuaWorldObject::IsWithinDist3d },
    { "IsWithinDist2d", &LuaWorldObject::IsWithinDist2d },
    { "IsWithinDist", &LuaWorldObject::IsWithinDist },
    { "IsWithinDistInMap", &LuaWorldObject::IsWithinDistInMap },
    { "IsInRange", &LuaWorldObject::IsInRange },
    { "IsInRange2d", &LuaWorldObject::IsInRange2d },
    { "IsInRange3d", &LuaWorldObject::IsInRange3d },
    { "IsInFront", &LuaWorldObject::IsInFront },
    { "IsInBack", &LuaWorldObject::IsInBack },

    // Other
    { "SummonGameObject", &LuaWorldObject::SummonGameObject },
    { "SpawnCreature", &LuaWorldObject::SpawnCreature },
    { "SendPacket", &LuaWorldObject::SendPacket },
    { "RegisterEvent", &LuaWorldObject::RegisterEvent },
    { "RemoveEventById", &LuaWorldObject::RemoveEventById },
    { "RemoveEvents", &LuaWorldObject::RemoveEvents },
    { "PlayMusic", &LuaWorldObject::PlayMusic },
    { "PlayDirectSound", &LuaWorldObject::PlayDirectSound },
    { "PlayDistanceSound", &LuaWorldObject::PlayDistanceSound },

    { NULL, NULL }
};



ForgeRegister<GameObject> GameObjectMethods[] =
{
    // Getters
    { "GetDisplayId", &LuaGameObject::GetDisplayId },
    { "GetGoState", &LuaGameObject::GetGoState },
    { "GetLootState", &LuaGameObject::GetLootState },
    { "GetLootRecipient", &LuaGameObject::GetLootRecipient },
    { "GetLootRecipientGroup", &LuaGameObject::GetLootRecipientGroup },
    { "GetDBTableGUIDLow", &LuaGameObject::GetDBTableGUIDLow },

    // Setters
    { "SetGoState", &LuaGameObject::SetGoState },
    { "SetLootState", &LuaGameObject::SetLootState },
    { "SetRespawnTime", &LuaGameObject::SetRespawnTime },

    // Boolean
    { "IsTransport", &LuaGameObject::IsTransport },
    // {"IsDestructible", &LuaGameObject::IsDestructible},    // :IsDestructible() - UNDOCUMENTED
    { "IsActive", &LuaGameObject::IsActive },
    { "HasQuest", &LuaGameObject::HasQuest },
    { "IsSpawned", &LuaGameObject::IsSpawned },

    // Other
    { "RemoveFromWorld", &LuaGameObject::RemoveFromWorld },
    { "UseDoorOrButton", &LuaGameObject::UseDoorOrButton },
    { "Despawn", &LuaGameObject::Despawn },
    { "Respawn", &LuaGameObject::Respawn },
    { "SaveToDB", &LuaGameObject::SaveToDB },
    { "AddLoot", &LuaGameObject::AddLoot },

    { NULL, NULL }
};

ForgeRegister<Item> ItemMethods[] =
{
    // Getters
    { "GetOwnerGUID", &LuaItem::GetOwnerGUID },
    { "GetOwner", &LuaItem::GetOwner },
    { "GetCount", &LuaItem::GetCount },
    { "GetMaxStackCount", &LuaItem::GetMaxStackCount },
    { "GetSlot", &LuaItem::GetSlot },
    { "GetBagSlot", &LuaItem::GetBagSlot },
    { "GetEnchantmentId", &LuaItem::GetEnchantmentId },
    { "GetSpellId", &LuaItem::GetSpellId },
    { "GetSpellTrigger", &LuaItem::GetSpellTrigger },
    { "GetItemLink", &LuaItem::GetItemLink },
    { "GetClass", &LuaItem::GetClass },
    { "GetSubClass", &LuaItem::GetSubClass },
    { "GetName", &LuaItem::GetName },
    { "GetDisplayId", &LuaItem::GetDisplayId },
    { "GetQuality", &LuaItem::GetQuality },
    { "GetBuyCount", &LuaItem::GetBuyCount },
    { "GetBuyPrice", &LuaItem::GetBuyPrice },
    { "GetSellPrice", &LuaItem::GetSellPrice },
    { "GetInventoryType", &LuaItem::GetInventoryType },
    { "GetAllowableClass", &LuaItem::GetAllowableClass },
    { "GetAllowableRace", &LuaItem::GetAllowableRace },
    { "GetItemLevel", &LuaItem::GetItemLevel },
    { "GetRequiredLevel", &LuaItem::GetRequiredLevel },
#ifdef WOTLK
    { "GetStatsCount", &LuaItem::GetStatsCount },
#endif
    { "GetRandomProperty", &LuaItem::GetRandomProperty },
#ifndef CLASSIC
    { "GetRandomSuffix", &LuaItem::GetRandomSuffix },
#endif
    { "GetItemSet", &LuaItem::GetItemSet },
    { "GetBagSize", &LuaItem::GetBagSize },
    { "GetItemTemplate", &LuaItem::GetItemTemplate },

    // Setters
    { "SetOwner", &LuaItem::SetOwner },
    { "SetBinding", &LuaItem::SetBinding },
    { "SetCount", &LuaItem::SetCount },

    // Boolean
    { "IsSoulBound", &LuaItem::IsSoulBound },
#if (!defined(TBC) && !defined(CLASSIC))
    { "IsBoundAccountWide", &LuaItem::IsBoundAccountWide },
#endif
    { "IsBoundByEnchant", &LuaItem::IsBoundByEnchant },
    { "IsNotBoundToPlayer", &LuaItem::IsNotBoundToPlayer },
    { "IsLocked", &LuaItem::IsLocked },
    { "IsBag", &LuaItem::IsBag },
#ifndef CLASSIC
    { "IsCurrencyToken", &LuaItem::IsCurrencyToken },
#endif
    { "IsNotEmptyBag", &LuaItem::IsNotEmptyBag },
    { "IsBroken", &LuaItem::IsBroken },
    { "CanBeTraded", &LuaItem::CanBeTraded },
    { "IsInTrade", &LuaItem::IsInTrade },
    { "IsInBag", &LuaItem::IsInBag },
    { "IsEquipped", &LuaItem::IsEquipped },
    { "HasQuest", &LuaItem::HasQuest },
    { "IsPotion", &LuaItem::IsPotion },
#if defined(CLASSIC) || defined(TBC) || defined(WOTLK)
    { "IsWeaponVellum", &LuaItem::IsWeaponVellum },
    { "IsArmorVellum", &LuaItem::IsArmorVellum },
#endif
    { "IsConjuredConsumable", &LuaItem::IsConjuredConsumable },
    //{"IsRefundExpired", &LuaItem::IsRefundExpired},               // :IsRefundExpired() - UNDOCUMENTED - Returns true if the item's refund time has expired
    { "SetEnchantment", &LuaItem::SetEnchantment },
    { "ClearEnchantment", &LuaItem::ClearEnchantment },

    // Other
    { "SaveToDB", &LuaItem::SaveToDB },

    { NULL, NULL }
};

ForgeRegister<ItemTemplate> ItemTemplateMethods[] =
{
    { "GetItemId", &LuaItemTemplate::GetItemId },
    { "GetClass", &LuaItemTemplate::GetClass },
    { "GetSubClass", &LuaItemTemplate::GetSubClass },
    { "GetName", &LuaItemTemplate::GetName },
    { "GetDisplayId", &LuaItemTemplate::GetDisplayId },
    { "GetQuality", &LuaItemTemplate::GetQuality },
    { "GetFlags", &LuaItemTemplate::GetFlags },
    { "GetExtraFlags", &LuaItemTemplate::GetExtraFlags },
    { "GetBuyCount", &LuaItemTemplate::GetBuyCount },
    { "GetBuyPrice", &LuaItemTemplate::GetBuyPrice },
    { "GetSellPrice", &LuaItemTemplate::GetSellPrice },
    { "GetInventoryType", &LuaItemTemplate::GetInventoryType },
    { "GetAllowableClass", &LuaItemTemplate::GetAllowableClass },
    { "GetAllowableRace", &LuaItemTemplate::GetAllowableRace },
    { "GetItemLevel", &LuaItemTemplate::GetItemLevel },
    { "GetRequiredLevel", &LuaItemTemplate::GetRequiredLevel },
    { NULL, NULL }
};

ForgeRegister<Aura> AuraMethods[] =
{
    // Getters
    { "GetCaster", &LuaAura::GetCaster },
    { "GetCasterGUID", &LuaAura::GetCasterGUID },
    { "GetCasterLevel", &LuaAura::GetCasterLevel },
    { "GetDuration", &LuaAura::GetDuration },
    { "GetMaxDuration", &LuaAura::GetMaxDuration },
    { "GetAuraId", &LuaAura::GetAuraId },
    { "GetStackAmount", &LuaAura::GetStackAmount },
    { "GetOwner", &LuaAura::GetOwner },

    // Setters
    { "SetDuration", &LuaAura::SetDuration },
    { "SetMaxDuration", &LuaAura::SetMaxDuration },
    { "SetStackAmount", &LuaAura::SetStackAmount },

    // Other
    { "Remove", &LuaAura::Remove },

    { NULL, NULL }
};

ForgeRegister<Quest> QuestMethods[] =
{
    // Getters
    { "GetId", &LuaQuest::GetId },
    { "GetLevel", &LuaQuest::GetLevel },
    // {"GetMaxLevel", &LuaQuest::GetMaxLevel},                   // :GetMaxLevel() - UNDOCUMENTED - Returns the quest's max level
    { "GetMinLevel", &LuaQuest::GetMinLevel },
    { "GetNextQuestId", &LuaQuest::GetNextQuestId },
    { "GetPrevQuestId", &LuaQuest::GetPrevQuestId },
    { "GetNextQuestInChain", &LuaQuest::GetNextQuestInChain },
    { "GetFlags", &LuaQuest::GetFlags },
    { "GetType", &LuaQuest::GetType },

    // Boolean
    { "HasFlag", &LuaQuest::HasFlag },
#ifndef CLASSIC
    { "IsDaily", &LuaQuest::IsDaily },
#endif
    { "IsRepeatable", &LuaQuest::IsRepeatable },

    { NULL, NULL }
};

ForgeRegister<Group> GroupMethods[] =
{
    // Getters
    { "GetMembers", &LuaGroup::GetMembers },
    { "GetLeaderGUID", &LuaGroup::GetLeaderGUID },
    { "GetGUID", &LuaGroup::GetGUID },
    { "GetMemberGroup", &LuaGroup::GetMemberGroup },
    { "GetMemberGUID", &LuaGroup::GetMemberGUID },
    { "GetMembersCount", &LuaGroup::GetMembersCount },
    { "GetGroupType", &LuaGroup::GetGroupType },

    // Setters
    { "SetLeader", &LuaGroup::SetLeader },
    { "SetMembersGroup", &LuaGroup::SetMembersGroup },
    { "SetTargetIcon", &LuaGroup::SetTargetIcon },
    { "SetMemberFlag", &LuaGroup::SetMemberFlag },

    // Boolean
    { "IsLeader", &LuaGroup::IsLeader },
    { "AddMember", &LuaGroup::AddMember },
    { "RemoveMember", &LuaGroup::RemoveMember },
    { "Disband", &LuaGroup::Disband },
    { "IsFull", &LuaGroup::IsFull },
    { "IsLFGGroup", &LuaGroup::IsLFGGroup },
    { "IsRaidGroup", &LuaGroup::IsRaidGroup },
    { "IsBGGroup", &LuaGroup::IsBGGroup },
    // {"IsBFGroup", &LuaGroup::IsBFGroup},                       // :IsBFGroup() - UNDOCUMENTED - Returns true if the group is a battlefield group
    { "IsMember", &LuaGroup::IsMember },
    { "IsAssistant", &LuaGroup::IsAssistant },
    { "SameSubGroup", &LuaGroup::SameSubGroup },
    { "HasFreeSlotSubGroup", &LuaGroup::HasFreeSlotSubGroup },

    // Other
    { "SendPacket", &LuaGroup::SendPacket },
    // {"ConvertToLFG", &LuaGroup::ConvertToLFG},                 // :ConvertToLFG() - UNDOCUMENTED - Converts the group to an LFG group
    { "ConvertToRaid", &LuaGroup::ConvertToRaid },

    { NULL, NULL }
};

ForgeRegister<Guild> GuildMethods[] =
{
    // Getters
    { "GetMembers", &LuaGuild::GetMembers },
    { "GetLeader", &LuaGuild::GetLeader },
    { "GetLeaderGUID", &LuaGuild::GetLeaderGUID },
    { "GetId", &LuaGuild::GetId },
    { "GetName", &LuaGuild::GetName },
    { "GetMOTD", &LuaGuild::GetMOTD },
    { "GetInfo", &LuaGuild::GetInfo },
    { "GetMemberCount", &LuaGuild::GetMemberCount },

    // Setters
#ifndef CLASSIC
    { "SetBankTabText", &LuaGuild::SetBankTabText },
#endif
    { "SetMemberRank", &LuaGuild::SetMemberRank },
#if defined(CLASSIC) || defined(TBC) || defined(WOTLK)
    { "SetLeader", &LuaGuild::SetLeader },
#endif

    // Other
    { "SendPacket", &LuaGuild::SendPacket },
    { "SendPacketToRanked", &LuaGuild::SendPacketToRanked },
    { "Disband", &LuaGuild::Disband },
    { "AddMember", &LuaGuild::AddMember },
    { "DeleteMember", &LuaGuild::DeleteMember },

    { NULL, NULL }
};

#ifndef CLASSIC
#ifndef TBC
ForgeRegister<Vehicle> VehicleMethods[] =
{
    // Getters
    { "GetOwner", &LuaVehicle::GetOwner },
    { "GetEntry", &LuaVehicle::GetEntry },
    { "GetPassenger", &LuaVehicle::GetPassenger },

    // Boolean
    { "IsOnBoard", &LuaVehicle::IsOnBoard },

    // Other
    { "AddPassenger", &LuaVehicle::AddPassenger },
    { "RemovePassenger", &LuaVehicle::RemovePassenger },

    { NULL, NULL }
};
#endif
#endif

ForgeRegister<ForgeQuery> QueryMethods[] =
{
    // Getters
    { "GetColumnCount", &LuaQuery::GetColumnCount },
    { "GetRowCount", &LuaQuery::GetRowCount },
    { "GetRow", &LuaQuery::GetRow },
    { "GetBool", &LuaQuery::GetBool },
    { "GetUInt8", &LuaQuery::GetUInt8 },
    { "GetUInt16", &LuaQuery::GetUInt16 },
    { "GetUInt32", &LuaQuery::GetUInt32 },
    { "GetUInt64", &LuaQuery::GetUInt64 },
    { "GetInt8", &LuaQuery::GetInt8 },
    { "GetInt16", &LuaQuery::GetInt16 },
    { "GetInt32", &LuaQuery::GetInt32 },
    { "GetInt64", &LuaQuery::GetInt64 },
    { "GetFloat", &LuaQuery::GetFloat },
    { "GetDouble", &LuaQuery::GetDouble },
    { "GetString", &LuaQuery::GetString },

    // Boolean
    { "NextRow", &LuaQuery::NextRow },
    { "IsNull", &LuaQuery::IsNull },

    { NULL, NULL }
};

ForgeRegister<WorldPacket> PacketMethods[] =
{
    // Getters
    { "GetOpcode", &LuaPacket::GetOpcode },
    { "GetSize", &LuaPacket::GetSize },

    // Setters
    { "SetOpcode", &LuaPacket::SetOpcode },

    // Readers
    { "ReadByte", &LuaPacket::ReadByte },
    { "ReadUByte", &LuaPacket::ReadUByte },
    { "ReadShort", &LuaPacket::ReadShort },
    { "ReadUShort", &LuaPacket::ReadUShort },
    { "ReadLong", &LuaPacket::ReadLong },
    { "ReadULong", &LuaPacket::ReadULong },
    { "ReadGUID", &LuaPacket::ReadGUID },
    { "ReadString", &LuaPacket::ReadString },
    { "ReadFloat", &LuaPacket::ReadFloat },
    { "ReadDouble", &LuaPacket::ReadDouble },

    // Writers
    { "WriteByte", &LuaPacket::WriteByte },
    { "WriteUByte", &LuaPacket::WriteUByte },
    { "WriteShort", &LuaPacket::WriteShort },
    { "WriteUShort", &LuaPacket::WriteUShort },
    { "WriteLong", &LuaPacket::WriteLong },
    { "WriteULong", &LuaPacket::WriteULong },
    { "WriteGUID", &LuaPacket::WriteGUID },
    { "WriteString", &LuaPacket::WriteString },
    { "WriteFloat", &LuaPacket::WriteFloat },
    { "WriteDouble", &LuaPacket::WriteDouble },

    { NULL, NULL }
};

ForgeRegister<Corpse> CorpseMethods[] =
{
    // Getters
    { "GetOwnerGUID", &LuaCorpse::GetOwnerGUID },
    { "GetGhostTime", &LuaCorpse::GetGhostTime },
    { "GetType", &LuaCorpse::GetType },

    // Other
    { "ResetGhostTime", &LuaCorpse::ResetGhostTime },
    { "SaveToDB", &LuaCorpse::SaveToDB },

    { NULL, NULL }
};

ForgeRegister<AuctionHouseEntry> AuctionMethods[] =
{
    { NULL, NULL }
};

ForgeRegister<BattleGround> BattleGroundMethods[] =
{
    // Getters
    { "GetName", &LuaBattleGround::GetName },
    { "GetAlivePlayersCountByTeam", &LuaBattleGround::GetAlivePlayersCountByTeam },
    { "GetMap", &LuaBattleGround::GetMap },
    { "GetBonusHonorFromKillCount", &LuaBattleGround::GetBonusHonorFromKillCount },
#ifndef AZEROTHCORE
    { "GetBracketId", &LuaBattleGround::GetBracketId },
#endif
    { "GetEndTime", &LuaBattleGround::GetEndTime },
    { "GetFreeSlotsForTeam", &LuaBattleGround::GetFreeSlotsForTeam },
    { "GetInstanceId", &LuaBattleGround::GetInstanceId },
    { "GetMapId", &LuaBattleGround::GetMapId },
    { "GetTypeId", &LuaBattleGround::GetTypeId },
    { "GetMaxLevel", &LuaBattleGround::GetMaxLevel },
    { "GetMinLevel", &LuaBattleGround::GetMinLevel },
    { "GetMaxPlayers", &LuaBattleGround::GetMaxPlayers },
    { "GetMinPlayers", &LuaBattleGround::GetMinPlayers },
    { "GetMaxPlayersPerTeam", &LuaBattleGround::GetMaxPlayersPerTeam },
    { "GetMinPlayersPerTeam", &LuaBattleGround::GetMinPlayersPerTeam },
    { "GetWinner", &LuaBattleGround::GetWinner },
    { "GetStatus", &LuaBattleGround::GetStatus },

    { NULL, NULL }
};

ForgeRegister<ChatHandler> ChatHandlerMethods[] =
{
    { "SendSysMessage", &LuaChatHandler::SendSysMessage },
    { "IsConsole", &LuaChatHandler::IsConsole },
    { "GetPlayer", &LuaChatHandler::GetPlayer },
    { "SendGlobalSysMessage", &LuaChatHandler::SendGlobalSysMessage },
    { "SendGlobalGMSysMessage", &LuaChatHandler::SendGlobalGMSysMessage },
    { "HasLowerSecurity", &LuaChatHandler::HasLowerSecurity },
    { "HasLowerSecurityAccount", &LuaChatHandler::HasLowerSecurityAccount },
    { "GetSelectedPlayer", &LuaChatHandler::GetSelectedPlayer },
    { "GetSelectedCreature", &LuaChatHandler::GetSelectedCreature },
    { "GetSelectedUnit", &LuaChatHandler::GetSelectedUnit },
    { "GetSelectedObject", &LuaChatHandler::GetSelectedObject },
    { "GetSelectedPlayerOrSelf", &LuaChatHandler::GetSelectedPlayerOrSelf },
    { "IsAvailable", &LuaChatHandler::IsAvailable },
    { "HasSentErrorMessage", &LuaChatHandler::HasSentErrorMessage },

    { NULL, NULL }
};

ForgeRegister<AchievementEntry> AchievementMethods[] =
{
    { "GetId", &LuaAchievement::GetId },
    { "GetName", &LuaAchievement::GetName },

    { NULL, NULL }
};

ForgeRegister<Roll> RollMethods[] =
{
    { "GetItemGUID", &LuaRoll::GetItemGUID },
    { "GetItemId", &LuaRoll::GetItemId },
    { "GetItemRandomPropId", &LuaRoll::GetItemRandomPropId },
    { "GetItemRandomSuffix", &LuaRoll::GetItemRandomSuffix },
    { "GetItemCount", &LuaRoll::GetItemCount },
    { "GetPlayerVote", &LuaRoll::GetPlayerVote },
    { "GetPlayerVoteGUIDs", &LuaRoll::GetPlayerVoteGUIDs },
    { "GetTotalPlayersRolling", &LuaRoll::GetTotalPlayersRolling },
    { "GetTotalNeed", &LuaRoll::GetTotalNeed },
    { "GetTotalGreed", &LuaRoll::GetTotalGreed },
    { "GetTotalPass", &LuaRoll::GetTotalPass },
    { "GetItemSlot", &LuaRoll::GetItemSlot },
    { "GetRollVoteMask", &LuaRoll::GetRollVoteMask },

    { NULL, NULL }
};

#if (!defined(TBC) && !defined(CLASSIC))
// fix compile error about accessing vehicle destructor
template<> int ForgeTemplate<Vehicle>::CollectGarbage(lua_State* L)
{
    ASSERT(!manageMemory);

    // Get object pointer (and check type, no error)
    ForgeObject* obj = Forge::CHECKOBJ<ForgeObject>(L, 1, false);
    delete obj;
    return 0;
}
#endif

// Template by Mud from http://stackoverflow.com/questions/4484437/lua-integer-type/4485511#4485511
template<> int ForgeTemplate<unsigned long long>::Add(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) + Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Substract(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) - Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Multiply(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) * Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Divide(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) / Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Mod(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) % Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
// template<> int ForgeTemplate<unsigned long long>::UnaryMinus(lua_State* L) { Forge::Push(L, -Forge::CHECKVAL<unsigned long long>(L, 1)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Equal(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) == Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Less(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) < Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::LessOrEqual(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<unsigned long long>(L, 1) <= Forge::CHECKVAL<unsigned long long>(L, 2)); return 1; }
template<> int ForgeTemplate<unsigned long long>::Pow(lua_State* L)
{
    Forge::Push(L, static_cast<unsigned long long>(powl(static_cast<long double>(Forge::CHECKVAL<unsigned long long>(L, 1)), static_cast<long double>(Forge::CHECKVAL<unsigned long long>(L, 2)))));
    return 1;
}
template<> int ForgeTemplate<unsigned long long>::ToString(lua_State* L)
{
    unsigned long long l = Forge::CHECKVAL<unsigned long long>(L, 1);
    std::ostringstream ss;
    ss << l;
    Forge::Push(L, ss.str());
    return 1;
}

template<> int ForgeTemplate<long long>::Add(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) + Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::Substract(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) - Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::Multiply(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) * Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::Divide(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) / Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::Mod(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) % Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::UnaryMinus(lua_State* L) { Forge::Push(L, -Forge::CHECKVAL<long long>(L, 1)); return 1; }
template<> int ForgeTemplate<long long>::Equal(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) == Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::Less(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) < Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::LessOrEqual(lua_State* L) { Forge::Push(L, Forge::CHECKVAL<long long>(L, 1) <= Forge::CHECKVAL<long long>(L, 2)); return 1; }
template<> int ForgeTemplate<long long>::Pow(lua_State* L)
{
    Forge::Push(L, static_cast<long long>(powl(static_cast<long double>(Forge::CHECKVAL<long long>(L, 1)), static_cast<long double>(Forge::CHECKVAL<long long>(L, 2)))));
    return 1;
}
template<> int ForgeTemplate<long long>::ToString(lua_State* L)
{
    long long l = Forge::CHECKVAL<long long>(L, 1);
    std::ostringstream ss;
    ss << l;
    Forge::Push(L, ss.str());
    return 1;
}

void RegisterFunctions(Forge* E)
{
    ForgeGlobal::SetMethods(E, LuaGlobalFunctions::GlobalMethods);

    ForgeTemplate<Object>::Register(E, "Object");
    ForgeTemplate<Object>::SetMethods(E, ObjectMethods);

    ForgeTemplate<WorldObject>::Register(E, "WorldObject");
    ForgeTemplate<WorldObject>::SetMethods(E, ObjectMethods);
    ForgeTemplate<WorldObject>::SetMethods(E, WorldObjectMethods);

    ForgeTemplate<Unit>::Register(E, "Unit");
    ForgeTemplate<Unit>::SetMethods(E, ObjectMethods);
    ForgeTemplate<Unit>::SetMethods(E, WorldObjectMethods);
    ForgeTemplate<Unit>::SetMethods(E, LuaUnit::UnitMethods);

    ForgeTemplate<Player>::Register(E, "Player");
    ForgeTemplate<Player>::SetMethods(E, ObjectMethods);
    ForgeTemplate<Player>::SetMethods(E, WorldObjectMethods);
    ForgeTemplate<Player>::SetMethods(E, LuaUnit::UnitMethods);
    ForgeTemplate<Player>::SetMethods(E, LuaPlayer::PlayerMethods);

    ForgeTemplate<Creature>::Register(E, "Creature");
    ForgeTemplate<Creature>::SetMethods(E, ObjectMethods);
    ForgeTemplate<Creature>::SetMethods(E, WorldObjectMethods);
    ForgeTemplate<Creature>::SetMethods(E, LuaUnit::UnitMethods);
    ForgeTemplate<Creature>::SetMethods(E, LuaCreature::CreatureMethods);

    ForgeTemplate<GameObject>::Register(E, "GameObject");
    ForgeTemplate<GameObject>::SetMethods(E, ObjectMethods);
    ForgeTemplate<GameObject>::SetMethods(E, WorldObjectMethods);
    ForgeTemplate<GameObject>::SetMethods(E, GameObjectMethods);

    ForgeTemplate<Corpse>::Register(E, "Corpse");
    ForgeTemplate<Corpse>::SetMethods(E, ObjectMethods);
    ForgeTemplate<Corpse>::SetMethods(E, WorldObjectMethods);
    ForgeTemplate<Corpse>::SetMethods(E, CorpseMethods);

    ForgeTemplate<Item>::Register(E, "Item");
    ForgeTemplate<Item>::SetMethods(E, ObjectMethods);
    ForgeTemplate<Item>::SetMethods(E, ItemMethods);

    ForgeTemplate<ItemTemplate>::Register(E, "ItemTemplate");
    ForgeTemplate<ItemTemplate>::SetMethods(E, ItemTemplateMethods);

#ifndef CLASSIC
#ifndef TBC
    ForgeTemplate<Vehicle>::Register(E, "Vehicle");
    ForgeTemplate<Vehicle>::SetMethods(E, VehicleMethods);
#endif
#endif

    ForgeTemplate<Group>::Register(E, "Group");
    ForgeTemplate<Group>::SetMethods(E, GroupMethods);

    ForgeTemplate<Guild>::Register(E, "Guild");
    ForgeTemplate<Guild>::SetMethods(E, GuildMethods);

    ForgeTemplate<Aura>::Register(E, "Aura");
    ForgeTemplate<Aura>::SetMethods(E, AuraMethods);

    ForgeTemplate<Spell>::Register(E, "Spell");
    ForgeTemplate<Spell>::SetMethods(E, LuaSpell::SpellMethods);

    ForgeTemplate<SpellInfo>::Register(E, "SpellInfo");
    ForgeTemplate<SpellInfo>::SetMethods(E, LuaSpellInfo::SpellInfoMethods);

    ForgeTemplate<SpellEffectInfo>::Register(E, "SpellEffectInfo");
    ForgeTemplate<SpellEffectInfo>::SetMethods(E, LuaSpellEffectInfo::SpellEffectInfoMethods);

    ForgeTemplate<Quest>::Register(E, "Quest");
    ForgeTemplate<Quest>::SetMethods(E, QuestMethods);

    ForgeTemplate<Map>::Register(E, "Map");
    ForgeTemplate<Map>::SetMethods(E, LuaMap::MapMethods);

    ForgeTemplate<AuctionHouseEntry>::Register(E, "AuctionHouseEntry");
    ForgeTemplate<AuctionHouseEntry>::SetMethods(E, AuctionMethods);

    ForgeTemplate<BattleGround>::Register(E, "BattleGround");
    ForgeTemplate<BattleGround>::SetMethods(E, BattleGroundMethods);

    ForgeTemplate<ChatHandler>::Register(E, "ChatHandler");
    ForgeTemplate<ChatHandler>::SetMethods(E, ChatHandlerMethods);

    ForgeTemplate<WorldPacket>::Register(E, "WorldPacket", true);
    ForgeTemplate<WorldPacket>::SetMethods(E, PacketMethods);

    ForgeTemplate<ForgeQuery>::Register(E, "ForgeQuery", true);
    ForgeTemplate<ForgeQuery>::SetMethods(E, QueryMethods);

    ForgeTemplate<AchievementEntry>::Register(E, "AchievementEntry");
    ForgeTemplate<AchievementEntry>::SetMethods(E, AchievementMethods);

    ForgeTemplate<Roll>::Register(E, "Roll");
    ForgeTemplate<Roll>::SetMethods(E, RollMethods);

    ForgeTemplate<Loot>::Register(E, "Loot");
    ForgeTemplate<Loot>::SetMethods(E, LuaLoot::LootMethods);

    ForgeTemplate<long long>::Register(E, "long long", true);

    ForgeTemplate<unsigned long long>::Register(E, "unsigned long long", true);
}
