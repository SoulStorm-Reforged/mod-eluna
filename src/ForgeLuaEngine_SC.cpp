/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Chat.h"
#include "ForgeEventMgr.h"
#include "Log.h"
#include "LuaEngine.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

class Forge_AllCreatureScript : public AllCreatureScript
{
public:
    Forge_AllCreatureScript() : AllCreatureScript("Forge_AllCreatureScript") { }

    // Creature
    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (sForge->OnGossipHello(player, creature))
            return true;

        return false;
    }

    bool CanCreatureGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (sForge->OnGossipSelect(player, creature, sender, action))
            return true;

        return false;
    }

    bool CanCreatureGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
    {
        if (sForge->OnGossipSelectCode(player, creature, sender, action, code))
            return true;

        return false;
    }

    void OnCreatureAddWorld(Creature* creature) override
    {
        sForge->OnAddToWorld(creature);

        if (creature->IsGuardian() && creature->ToTempSummon() && creature->ToTempSummon()->GetSummonerGUID().IsPlayer())
            sForge->OnPetAddedToWorld(creature->ToTempSummon()->GetSummonerUnit()->ToPlayer(), creature);
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        sForge->OnRemoveFromWorld(creature);
    }

    bool CanCreatureQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        sForge->OnQuestAccept(player, creature, quest);
        return false;
    }

    bool CanCreatureQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) override
    {
        if (sForge->OnQuestReward(player, creature, quest, opt))
        {
            ClearGossipMenuFor(player);
            return true;
        }

        return false;
    }

    CreatureAI* GetCreatureAI(Creature* creature) const override
    {
        if (CreatureAI* luaAI = sForge->GetAI(creature))
            return luaAI;

        return nullptr;
    }
};

class Forge_AllGameObjectScript : public AllGameObjectScript
{
public:
    Forge_AllGameObjectScript() : AllGameObjectScript("Forge_AllGameObjectScript") { }

    void OnGameObjectAddWorld(GameObject* go) override
    {
        sForge->OnAddToWorld(go);
    }

    void OnGameObjectRemoveWorld(GameObject* go) override
    {
        sForge->OnRemoveFromWorld(go);
    }

    void OnGameObjectUpdate(GameObject* go, uint32 diff) override
    {
        sForge->UpdateAI(go, diff);
    }

    bool CanGameObjectGossipHello(Player* player, GameObject* go) override
    {
        if (sForge->OnGossipHello(player, go))
            return true;

        if (sForge->OnGameObjectUse(player, go))
            return true;

        return false;
    }

    void OnGameObjectDamaged(GameObject* go, Player* player) override
    {
        sForge->OnDamaged(go, player);
    }

    void OnGameObjectDestroyed(GameObject* go, Player* player) override
    {
        sForge->OnDestroyed(go, player);
    }

    void OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* /*unit*/) override
    {
        sForge->OnLootStateChanged(go, state);
    }

    void OnGameObjectStateChanged(GameObject* go, uint32 state) override
    {
        sForge->OnGameObjectStateChanged(go, state);
    }

    bool CanGameObjectQuestAccept(Player* player, GameObject* go, Quest const* quest) override
    {
        sForge->OnQuestAccept(player, go, quest);
        return false;
    }

    bool CanGameObjectGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action) override
    {
        if (sForge->OnGossipSelect(player, go, sender, action))
            return true;

        return false;
    }

    bool CanGameObjectGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code) override
    {
        if (sForge->OnGossipSelectCode(player, go, sender, action, code))
            return true;

        return false;
    }

    bool CanGameObjectQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt) override
    {
        if (sForge->OnQuestAccept(player, go, quest))
            return false;

        if (sForge->OnQuestReward(player, go, quest, opt))
            return false;

        return true;
    }

    GameObjectAI* GetGameObjectAI(GameObject* go) const override
    {
        sForge->OnSpawn(go);
        return nullptr;
    }
};

class Forge_AllItemScript : public AllItemScript
{
public:
    Forge_AllItemScript() : AllItemScript("Forge_AllItemScript") { }

    bool CanItemQuestAccept(Player* player, Item* item, Quest const* quest) override
    {
        if (sForge->OnQuestAccept(player, item, quest))
            return false;

        return true;
    }

    bool CanItemUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (!sForge->OnUse(player, item, targets))
            return true;

        return false;
    }

    bool CanItemExpire(Player* player, ItemTemplate const* proto) override
    {
        if (sForge->OnExpire(player, proto))
            return false;

        return true;
    }

    bool CanItemRemove(Player* player, Item* item) override
    {
        if (sForge->OnRemove(player, item))
            return false;

        return true;
    }

    void OnItemGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        sForge->HandleGossipSelectOption(player, item, sender, action, "");
    }

    void OnItemGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code) override
    {
        sForge->HandleGossipSelectOption(player, item, sender, action, code);
    }
};

class Forge_AllMapScript : public AllMapScript
{
public:
    Forge_AllMapScript() : AllMapScript("Forge_AllMapScript") { }

    void OnBeforeCreateInstanceScript(InstanceMap* instanceMap, InstanceScript** instanceData, bool /*load*/, std::string /*data*/, uint32 /*completedEncounterMask*/) override
    {
        if (instanceData)
            *instanceData = sForge->GetInstanceData(instanceMap);
    }

    void OnDestroyInstance(MapInstanced* /*mapInstanced*/, Map* map) override
    {
        sForge->FreeInstanceId(map->GetInstanceId());
    }

    void OnCreateMap(Map* map) override
    {
        sForge->OnCreate(map);
    }

    void OnDestroyMap(Map* map) override
    {
        sForge->OnDestroy(map);
    }

    void OnPlayerEnterAll(Map* map, Player* player) override
    {
        sForge->OnPlayerEnter(map, player);
    }

    void OnPlayerLeaveAll(Map* map, Player* player) override
    {
        sForge->OnPlayerLeave(map, player);
    }

    void OnMapUpdate(Map* map, uint32 diff) override
    {
        sForge->OnUpdate(map, diff);
    }
};

class Forge_AuctionHouseScript : public AuctionHouseScript
{
public:
    Forge_AuctionHouseScript() : AuctionHouseScript("Forge_AuctionHouseScript") { }

    void OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sForge->OnAdd(ah, entry);
    }

    void OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sForge->OnRemove(ah, entry);
    }

    void OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sForge->OnSuccessful(ah, entry);
    }

    void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sForge->OnExpire(ah, entry);
    }
};

class Forge_BGScript : public BGScript
{
public:
    Forge_BGScript() : BGScript("Forge_BGScript") { }

    void OnBattlegroundStart(Battleground* bg) override
    {
        sForge->OnBGStart(bg, bg->GetBgTypeID(), bg->GetInstanceID());
    }

    void OnBattlegroundEnd(Battleground* bg, TeamId winnerTeam) override
    {
        sForge->OnBGEnd(bg, bg->GetBgTypeID(), bg->GetInstanceID(), winnerTeam);
    }

    void OnBattlegroundDestroy(Battleground* bg) override
    {
        sForge->OnBGDestroy(bg, bg->GetBgTypeID(), bg->GetInstanceID());
    }

    void OnBattlegroundCreate(Battleground* bg) override
    {
        sForge->OnBGCreate(bg, bg->GetBgTypeID(), bg->GetInstanceID());
    }
};

class Forge_CommandSC : public CommandSC
{
public:
    Forge_CommandSC() : CommandSC("Forge_CommandSC") { }

    bool OnTryExecuteCommand(ChatHandler& handler, std::string_view cmdStr) override
    {
        if (!sForge->OnCommand(handler, std::string(cmdStr).c_str()))
        {
            return false;
        }

        return true;
    }
};

class Forge_ForgeScript : public ForgeScript
{
public:
    Forge_ForgeScript() : ForgeScript("Forge_ForgeScript") { }

    // Weather
    void OnWeatherChange(Weather* weather, WeatherState state, float grade) override
    {
        sForge->OnChange(weather, weather->GetZone(), state, grade);
    }

    // AreaTriger
    bool CanAreaTrigger(Player* player, AreaTrigger const* trigger) override
    {
        if (sForge->OnAreaTrigger(player, trigger))
            return true;

        return false;
    }
};

class Forge_GameEventScript : public GameEventScript
{
public:
    Forge_GameEventScript() : GameEventScript("Forge_GameEventScript") { }

    void OnStart(uint16 eventID) override
    {
        sForge->OnGameEventStart(eventID);
    }

    void OnStop(uint16 eventID) override
    {
        sForge->OnGameEventStop(eventID);
    }
};

class Forge_GroupScript : public GroupScript
{
public:
    Forge_GroupScript() : GroupScript("Forge_GroupScript") { }

    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        sForge->OnAddMember(group, guid);
    }

    void OnInviteMember(Group* group, ObjectGuid guid) override
    {
        sForge->OnInviteMember(group, guid);
    }

    void OnRemoveMember(Group* group, ObjectGuid guid, RemoveMethod method, ObjectGuid /* kicker */, const char* /* reason */) override
    {
        sForge->OnRemoveMember(group, guid, method);
    }

    void OnChangeLeader(Group* group, ObjectGuid newLeaderGuid, ObjectGuid oldLeaderGuid) override
    {
        sForge->OnChangeLeader(group, newLeaderGuid, oldLeaderGuid);
    }

    void OnDisband(Group* group) override
    {
        sForge->OnDisband(group);
    }

    void OnCreate(Group* group, Player* leader) override
    {
        sForge->OnCreate(group, leader->GetGUID(), group->GetGroupType());
    }
};

class Forge_GuildScript : public GuildScript
{
public:
    Forge_GuildScript() : GuildScript("Forge_GuildScript") { }

    void OnAddMember(Guild* guild, Player* player, uint8& plRank) override
    {
        sForge->OnAddMember(guild, player, plRank);
    }

    void OnRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool /*isKicked*/) override
    {
        sForge->OnRemoveMember(guild, player, isDisbanding);
    }

    void OnMOTDChanged(Guild* guild, const std::string& newMotd) override
    {
        sForge->OnMOTDChanged(guild, newMotd);
    }

    void OnInfoChanged(Guild* guild, const std::string& newInfo) override
    {
        sForge->OnInfoChanged(guild, newInfo);
    }

    void OnCreate(Guild* guild, Player* leader, const std::string& name) override
    {
        sForge->OnCreate(guild, leader, name);
    }

    void OnDisband(Guild* guild) override
    {
        sForge->OnDisband(guild);
    }

    void OnMemberWitdrawMoney(Guild* guild, Player* player, uint32& amount, bool isRepair) override
    {
        sForge->OnMemberWitdrawMoney(guild, player, amount, isRepair);
    }

    void OnMemberDepositMoney(Guild* guild, Player* player, uint32& amount) override
    {
        sForge->OnMemberDepositMoney(guild, player, amount);
    }

    void OnItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId,
        bool isDestBank, uint8 destContainer, uint8 destSlotId) override
    {
        sForge->OnItemMove(guild, player, pItem, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId);
    }

    void OnEvent(Guild* guild, uint8 eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank) override
    {
        sForge->OnEvent(guild, eventType, playerGuid1, playerGuid2, newRank);
    }

    void OnBankEvent(Guild* guild, uint8 eventType, uint8 tabId, ObjectGuid::LowType playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId) override
    {
        sForge->OnBankEvent(guild, eventType, tabId, playerGuid, itemOrMoney, itemStackCount, destTabId);
    }
};

class Forge_LootScript : public LootScript
{
public:
    Forge_LootScript() : LootScript("Forge_LootScript") { }

    void OnLootMoney(Player* player, uint32 gold) override
    {
        sForge->OnLootMoney(player, gold);
    }
};

class Forge_MiscScript : public MiscScript
{
public:
    Forge_MiscScript() : MiscScript("Forge_MiscScript") { }

    void GetDialogStatus(Player* player, Object* questgiver) override
    {
        if (questgiver->GetTypeId() == TYPEID_GAMEOBJECT)
            sForge->GetDialogStatus(player, questgiver->ToGameObject());
        else if (questgiver->GetTypeId() == TYPEID_UNIT)
            sForge->GetDialogStatus(player, questgiver->ToCreature());
    }
};

class Forge_PetScript : public PetScript
{
public:
    Forge_PetScript() : PetScript("Forge_PetScript") { }

    void OnPetAddToWorld(Pet* pet) override
    {
        sForge->OnPetAddedToWorld(pet->GetOwner(), pet);
    }
};

class Forge_PlayerScript : public PlayerScript
{
public:
    Forge_PlayerScript() : PlayerScript("Forge_PlayerScript") { }

    void OnPlayerResurrect(Player* player, float /*restore_percent*/, bool /*applySickness*/) override
    {
        sForge->OnResurrect(player);
    }

    bool CanPlayerUseChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
    {
        if (type != CHAT_MSG_SAY && type != CHAT_MSG_YELL && type != CHAT_MSG_EMOTE)
            return true;

        if (!sForge->OnChat(player, type, lang, msg))
            return false;

        return true;
    }

    bool CanPlayerUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* target) override
    {
        if (!sForge->OnChat(player, type, lang, msg, target))
            return false;

        return true;
    }

    bool CanPlayerUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group) override
    {
        if (!sForge->OnChat(player, type, lang, msg, group))
            return false;

        return true;
    }

    bool CanPlayerUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild) override
    {
        if (!sForge->OnChat(player, type, lang, msg, guild))
            return false;

        return true;
    }

    bool CanPlayerUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel) override
    {
        if (!sForge->OnChat(player, type, lang, msg, channel))
            return false;

        return true;
    }

    void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override
    {
        sForge->OnLootItem(player, item, count, lootguid);
    }

    void OnPlayerLearnTalents(Player* player, uint32 talentId, uint32 talentRank, uint32 spellid) override
    {
        sForge->OnLearnTalents(player, talentId, talentRank, spellid);
    }

    bool CanUseItem(Player* player, ItemTemplate const* proto, InventoryResult& result) override
    {
        result = sForge->OnCanUseItem(player, proto->ItemId);
        return result != EQUIP_ERR_OK ? false : true;
    }

    void OnEquip(Player* player, Item* it, uint8 bag, uint8 slot, bool /*update*/) override
    {
        sForge->OnEquip(player, it, bag, slot);
    }

    void OnPlayerEnterCombat(Player* player, Unit* enemy) override
    {
        sForge->OnPlayerEnterCombat(player, enemy);
    }

    void OnPlayerLeaveCombat(Player* player) override
    {
        sForge->OnPlayerLeaveCombat(player);
    }

    bool CanRepopAtGraveyard(Player* player) override
    {
        sForge->OnRepop(player);
        return true;
    }

    void OnQuestAbandon(Player* player, uint32 questId) override
    {
        sForge->OnQuestAbandon(player, questId);
    }

    void OnMapChanged(Player* player) override
    {
        sForge->OnMapChanged(player);
    }

    void OnGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action) override
    {
        sForge->HandleGossipSelectOption(player, menu_id, sender, action, "");
    }

    void OnGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, const char* code) override
    {
        sForge->HandleGossipSelectOption(player, menu_id, sender, action, code);
    }

    void OnPVPKill(Player* killer, Player* killed) override
    {
        sForge->OnPVPKill(killer, killed);
    }

    void OnCreatureKill(Player* killer, Creature* killed) override
    {
        sForge->OnCreatureKill(killer, killed);
    }

    void OnPlayerKilledByCreature(Creature* killer, Player* killed) override
    {
        sForge->OnPlayerKilledByCreature(killer, killed);
    }

    void OnLevelChanged(Player* player, uint8 oldLevel) override
    {
        sForge->OnLevelChanged(player, oldLevel);
    }

    void OnFreeTalentPointsChanged(Player* player, uint32 points) override
    {
        sForge->OnFreeTalentPointsChanged(player, points);
    }

    void OnTalentsReset(Player* player, bool noCost) override
    {
        sForge->OnTalentsReset(player, noCost);
    }

    void OnMoneyChanged(Player* player, int32& amount) override
    {
        sForge->OnMoneyChanged(player, amount);
    }

    void OnGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource) override
    {
        sForge->OnGiveXP(player, amount, victim, xpSource);
    }

    bool OnReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental) override
    {
        return sForge->OnReputationChange(player, factionID, standing, incremental);
    }

    void OnDuelRequest(Player* target, Player* challenger) override
    {
        sForge->OnDuelRequest(target, challenger);
    }

    void OnDuelStart(Player* player1, Player* player2) override
    {
        sForge->OnDuelStart(player1, player2);
    }

    void OnDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
    {
        sForge->OnDuelEnd(winner, loser, type);
    }

    void OnEmote(Player* player, uint32 emote) override
    {
        sForge->OnEmote(player, emote);
    }

    void OnTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid) override
    {
        sForge->OnTextEmote(player, textEmote, emoteNum, guid);
    }

    void OnSpellCast(Player* player, Spell* spell, bool skipCheck) override
    {
        sForge->OnSpellCast(player, spell, skipCheck);
    }

    void OnLogin(Player* player) override
    {
        sForge->OnLogin(player);
    }

    void OnLogout(Player* player) override
    {
        sForge->OnLogout(player);
    }

    void OnCreate(Player* player) override
    {
        sForge->OnCreate(player);
    }

    void OnSave(Player* player) override
    {
        sForge->OnSave(player);
    }

    void OnDelete(ObjectGuid guid, uint32 /*accountId*/) override
    {
        sForge->OnDelete(guid.GetCounter());
    }

    void OnBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent) override
    {
        sForge->OnBindToInstance(player, difficulty, mapid, permanent);
    }

    void OnUpdateArea(Player* player, uint32 oldArea, uint32 newArea) override
    {
        sForge->OnUpdateArea(player, oldArea, newArea);
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea) override
    {
        sForge->OnUpdateZone(player, newZone, newArea);
    }

    void OnFirstLogin(Player* player) override
    {
        sForge->OnFirstLogin(player);
    }

    void OnLearnSpell(Player* player, uint32 spellId) override
    {
        sForge->OnLearnSpell(player, spellId);
    }

    void OnAchiComplete(Player* player, AchievementEntry const* achievement) override
    {
        sForge->OnAchiComplete(player, achievement);
    }

    void OnFfaPvpStateUpdate(Player* player, bool IsFlaggedForFfaPvp) override
    {
        sForge->OnFfaPvpStateUpdate(player, IsFlaggedForFfaPvp);
    }

    bool CanInitTrade(Player* player, Player* target) override
    {
        return sForge->OnCanInitTrade(player, target);
    }

    bool CanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint32 money, uint32 cod, Item* item) override
    {
        return sForge->OnCanSendMail(player, receiverGuid, mailbox, subject, body, money, cod, item);
    }

    bool CanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, const std::string& comment) override
    {
        return sForge->OnCanJoinLfg(player, roles, dungeons, comment);
    }

    void OnQuestRewardItem(Player* player, Item* item, uint32 count) override
    {
        sForge->OnQuestRewardItem(player, item, count);
    }

    void OnGroupRollRewardItem(Player* player, Item* item, uint32 count, RollVote voteType, Roll* roll) override
    {
        sForge->OnGroupRollRewardItem(player, item, count, voteType, roll);
    }

    void OnCreateItem(Player* player, Item* item, uint32 count) override
    {
        sForge->OnCreateItem(player, item, count);
    }

    void OnStoreNewItem(Player* player, Item* item, uint32 count) override
    {
        sForge->OnStoreNewItem(player, item, count);
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        sForge->OnPlayerCompleteQuest(player, quest);
    }

    bool CanGroupInvite(Player* player, std::string& memberName) override
    {
        return sForge->OnCanGroupInvite(player, memberName);
    }

    void OnBattlegroundDesertion(Player* player, const BattlegroundDesertionType type) override
    {
        sForge->OnBattlegroundDesertion(player, type);
    }

    void OnCreatureKilledByPet(Player* player, Creature* killed) override
    {
        sForge->OnCreatureKilledByPet(player, killed);
    }
};

class Forge_ServerScript : public ServerScript
{
public:
    Forge_ServerScript() : ServerScript("Forge_ServerScript") { }

    bool CanPacketSend(WorldSession* session, WorldPacket& packet) override
    {
        if (!sForge->OnPacketSend(session, packet))
            return false;

        return true;
    }

    bool CanPacketReceive(WorldSession* session, WorldPacket& packet) override
    {
        if (!sForge->OnPacketReceive(session, packet))
            return false;

        return true;
    }
};

class Forge_SpellSC : public SpellSC
{
public:
    Forge_SpellSC() : SpellSC("Forge_SpellSC") { }

    void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, GameObject* gameObjTarget) override
    {
        sForge->OnDummyEffect(caster, spellID, effIndex, gameObjTarget);
    }

    void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Creature* creatureTarget) override
    {
        sForge->OnDummyEffect(caster, spellID, effIndex, creatureTarget);
    }

    void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Item* itemTarget) override
    {
        sForge->OnDummyEffect(caster, spellID, effIndex, itemTarget);
    }
};

class Forge_UnitScript : public UnitScript
{
public:
    Forge_UnitScript() : UnitScript("Forge_UnitScript") { }

    void OnUnitUpdate(Unit* unit, uint32 diff) override
    {
        unit->forgeEvents->Update(diff);
    }
};

class Forge_VehicleScript : public VehicleScript
{
public:
    Forge_VehicleScript() : VehicleScript("Forge_VehicleScript") { }

    void OnInstall(Vehicle* veh) override
    {
        sForge->OnInstall(veh);
    }

    void OnUninstall(Vehicle* veh) override
    {
        sForge->OnUninstall(veh);
    }

    void OnInstallAccessory(Vehicle* veh, Creature* accessory) override
    {
        sForge->OnInstallAccessory(veh, accessory);
    }

    void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatId) override
    {
        sForge->OnAddPassenger(veh, passenger, seatId);
    }

    void OnRemovePassenger(Vehicle* veh, Unit* passenger) override
    {
        sForge->OnRemovePassenger(veh, passenger);
    }
};

class Forge_WorldObjectScript : public WorldObjectScript
{
public:
    Forge_WorldObjectScript() : WorldObjectScript("Forge_WorldObjectScript") { }

    void OnWorldObjectDestroy(WorldObject* object) override
    {
        delete object->forgeEvents;
        object->forgeEvents = nullptr;
    }

    void OnWorldObjectCreate(WorldObject* object) override
    {
        object->forgeEvents = nullptr;
    }

    void OnWorldObjectSetMap(WorldObject* object, Map* /*map*/) override
    {
        if (!object->forgeEvents)
            object->forgeEvents = new ForgeEventProcessor(&Forge::GForge, object);
    }

    void OnWorldObjectUpdate(WorldObject* object, uint32 diff) override
    {
        object->forgeEvents->Update(diff);
    }
};

class Forge_WorldScript : public WorldScript
{
public:
    Forge_WorldScript() : WorldScript("Forge_WorldScript") { }

    void OnOpenStateChange(bool open) override
    {
        sForge->OnOpenStateChange(open);
    }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload)
        {
            ///- Initialize Lua Engine
            LOG_INFO("forge", "Initialize Forge Lua Engine...");
            Forge::Initialize();
        }

        sForge->OnConfigLoad(reload, true);
    }

    void OnAfterConfigLoad(bool reload) override
    {
        sForge->OnConfigLoad(reload, false);
    }

    void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask) override
    {
        sForge->OnShutdownInitiate(code, mask);
    }

    void OnShutdownCancel() override
    {
        sForge->OnShutdownCancel();
    }

    void OnUpdate(uint32 diff) override
    {
        sForge->OnWorldUpdate(diff);
    }

    void OnStartup() override
    {
        sForge->OnStartup();
    }

    void OnShutdown() override
    {
        sForge->OnShutdown();
    }

    void OnAfterUnloadAllMaps() override
    {
        Forge::Uninitialize();
    }

    void OnBeforeWorldInitialized() override
    {
        ///- Run forge scripts.
        // in multithread foreach: run scripts
        sForge->RunScripts();
        sForge->OnConfigLoad(false, false); // Must be done after Forge is initialized and scripts have run.
    }
};

// Group all custom scripts
void AddSC_ForgeLuaEngine()
{
    new Forge_AllCreatureScript();
    new Forge_AllGameObjectScript();
    new Forge_AllItemScript();
    new Forge_AllMapScript();
    new Forge_AuctionHouseScript();
    new Forge_BGScript();
    new Forge_CommandSC();
    new Forge_ForgeScript();
    new Forge_GameEventScript();
    new Forge_GroupScript();
    new Forge_GuildScript();
    new Forge_LootScript();
    new Forge_MiscScript();
    new Forge_PetScript();
    new Forge_PlayerScript();
    new Forge_ServerScript();
    new Forge_SpellSC();
    new Forge_UnitScript();
    new Forge_VehicleScript();
    new Forge_WorldObjectScript();
    new Forge_WorldScript();
}
