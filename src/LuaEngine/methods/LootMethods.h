/*
* Copyright (C) 2010 - 2024 Forge Lua Engine <https://forgeluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef LOOTMETHODS_H
#define LOOTMETHODS_H

/***
 * Inherits all methods from: none
 */
namespace LuaLoot
{
    /**
     * Returns `true` if the [Loot] is already looted, `false` otherwise.
     *
     * @return bool isLooted
     */
    int IsLooted(lua_State* L, Loot* loot)
    {
        Forge::Push(L, loot->isLooted());
        return 1;
    }

    /**
     * Add an item to the [Loot] object
     *
     * @param uint32 itemid : The entry ID of the item to add.
     * @param float chance : The drop chance of the item, represented as a percentage (0-100%).
     * @param bool needs_quest : Specifies if the item is required for a quest (true if required, false otherwise).
     * @param uint16 lootmode : The mode of loot in which the item should be added. It determines under what circumstances the item can be looted.
     * @param uint8 groupid : The group ID for the item, used for grouping related items in the loot table.
     * @param int32 mincount : The minimum quantity of the item that can be dropped.
     * @param uint8 maxcount : The maximum quantity of the item that can be dropped.
     */
    int AddItem(lua_State* L, Loot* loot)
    {
        uint32 itemid = Forge::CHECKVAL<uint32>(L, 2);
        float chance = Forge::CHECKVAL<float>(L, 3);
        bool needs_quest = Forge::CHECKVAL<bool>(L, 4);
        uint16 lootmode = Forge::CHECKVAL<uint16>(L, 5);
        uint8 groupid = Forge::CHECKVAL<uint8>(L, 6);
        int32 mincount = Forge::CHECKVAL<int32>(L, 7);
        uint8 maxcount = Forge::CHECKVAL<uint8>(L, 8);

        for (LootItem& lootitem : loot->items)
        {
            if (lootitem.itemid == itemid && lootitem.count < 255)
            {
                lootitem.count += mincount; // Todo: Update to get rand between mincount and maxcount
                break;
            }
        }

        LootStoreItem newLootStoreItem(itemid, 0, chance, needs_quest, lootmode, groupid, mincount, maxcount);
        loot->AddItem(newLootStoreItem);

        return 0;
    }

    /**
     * Returns `true` if the [Loot] have item, `false` otherwise.
     *
     * @param uint32 itemid : The entry ID of the item to check (optionnal).
     * @param uint32 count : The count of item to check (optionnal).
     * @return bool hasItem : if not itemid return if [Loot] have any item.
     */
    int HasItem(lua_State* L, Loot* loot)
    {
        uint32 itemid = Forge::CHECKVAL<uint32>(L, 2, false);
        uint32 count = Forge::CHECKVAL<uint32>(L, 3, 0);

        bool hasItem = false;

        if (itemid)
        {
            for (const LootItem& lootitem : loot->items)
            {
                if (lootitem.itemid == itemid && (count == 0 || lootitem.count == count))
                {
                    hasItem = true;
                    break;
                }
            }
        }
        else
        {
            for (const LootItem& lootitem : loot->items)
            {
                if (lootitem.itemid && lootitem.itemid != 0)
                {
                    hasItem = true;
                    break;
                }
            }
        }

        Forge::Push(L, hasItem);
        return 1;
    }

    /**
     * Remove Item from the [Loot]. If there is no count there is delete all the specified itemid.
     *
     * @param uint32 itemid : The entry ID of the item to check (optionnal).
     * @param uint32 count : The count of item to check (optionnal).
     * @param bool force_remove : Force if the item is removed, true remove the item and false set the item is looted (not removed from object)
     */
    int RemoveItem(lua_State* L, Loot* loot)
    {
        uint32 itemid = Forge::CHECKVAL<uint32>(L, 2);
        bool isCountSpecified = Forge::CHECKVAL<uint32>(L, 3, false);
        uint32 count = isCountSpecified ? Forge::CHECKVAL<uint32>(L, 3) : 0;

        for (auto it = loot->items.begin(); it != loot->items.end();)
        {
            if (it->itemid == itemid)
            {
                if (isCountSpecified)
                {
                    if (it->count > count)
                    {
                        it->count -= count;
                        break;
                    }
                    else
                    {
                        count -= it->count;
                    }
                }

                it = loot->items.erase(it);
            }
            else
                ++it;
        }

        return 0;
    }

    /**
     * Returns the copper amount on the [Loot].
     *
     * @return uint32 copper
     */
    int GetMoney(lua_State* L, Loot* loot)
    {
        Forge::Push(L, loot->gold);
        return 1;
    }

    /**
     * Set the specified copper amount to the [Loot].
     *
     * @param uint32 copper_amount : copper amount needed to add to the [Loot].
     */
    int SetMoney(lua_State* L, Loot* loot)
    {
        uint32 copper_amount = Forge::CHECKVAL<uint32>(L, 2);
        loot->gold = copper_amount;

        return 0;
    }

    /**
     * Generate random copper amount to the [Loot].
     *
     * @param uint32 min_amount.
     * @param uint32 max_amount.
     */
    int GenerateMoney(lua_State* L, Loot* loot)
    {
        uint32 min_amount = Forge::CHECKVAL<uint32>(L, 2);
        uint32 max_amount = Forge::CHECKVAL<uint32>(L, 3);
        loot->generateMoneyLoot(min_amount, max_amount);

        return 0;
    }

    /**
     * Clear the [Loot].
     */
    int Clear(lua_State* /*L*/, Loot* loot)
    {
        loot->clear();
        return 0;
    }

    /**
     * Set the unlooted count in [Loot].
     *
     * @param uint32 unlooted_count : the unlooted item count in [Loot]
     */
    int SetUnlootedCount(lua_State* L, Loot* loot)
    {
        uint32 unlootedcount = Forge::CHECKVAL<uint32>(L, 2);

        loot->unlootedCount = unlootedcount;
        return 0;
    }

    /**
     * Returns the amount of unlooted count item of the [Loot].
     *
     * @return uint32 unlooted_count
     */
    int GetUnlootedCount(lua_State* L, Loot* loot)
    {
        Forge::Push(L, loot->unlootedCount);
        return 1;
    }

    /**
     * Returns a table with all data from the items in the [Loot]
     *
     * @return table lootItems
     */
    int GetItems(lua_State* L, Loot* loot)
    {
        lua_createtable(L, loot->items.size(), 0);
        int tbl = lua_gettop(L);

        for (unsigned int i = 0; i < loot->items.size(); i++)
        {
            lua_newtable(L);

            Forge::Push(L, loot->items[i].itemid);
            lua_setfield(L, -2, "id");

            Forge::Push(L, loot->items[i].itemIndex);
            lua_setfield(L, -2, "index");

            Forge::Push(L, loot->items[i].count);
            lua_setfield(L, -2, "count");

            Forge::Push(L, loot->items[i].needs_quest);
            lua_setfield(L, -2, "needs_quest");

            Forge::Push(L, loot->items[i].is_looted);
            lua_setfield(L, -2, "is_looted");

            Forge::Push(L, loot->items[i].rollWinnerGUID);
            lua_setfield(L, -2, "rollWinnerGUID");

            lua_rawseti(L, tbl, i + 1);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Update the item_index for the [Loot].
     */
    int UpdateItemIndex(lua_State* /*L*/, Loot* loot)
    {
        for (unsigned int i = 0; i < loot->items.size(); i++)
            loot->items[i].itemIndex = i;

        return 0;
    }

    /**
     * Set the specified item with specified count is already looter for the [Loot].
     *
     * This doesn't work for quest, ffa and conditionnal items (thanks @Shauren).
     *
     * @param uint32 itemid
     * @param uint32 count : specified count is already looter (if 0 or nil all specified item is tag to already looted)
     */
    int SetItemLooted(lua_State* L, Loot* loot)
    {
        uint32 itemid = Forge::CHECKVAL<uint32>(L, 2);
        uint32 count = Forge::CHECKVAL<uint32>(L, 3, 0);

        for (auto& lootItem : loot->items)
        {
            if (lootItem.itemid == itemid && (count == 0 || lootItem.count == count))
            {
                lootItem.is_looted = true;
                break;
            }
        }

        return 0;
    }

    ForgeRegister<Loot> LootMethods[] =
    {
        // Getters
        { "GetMoney", &LuaLoot::GetMoney },
        { "GetUnlootedCount", &LuaLoot::GetUnlootedCount },
        { "GetItems", &LuaLoot::GetItems },

        // Setters
        { "SetMoney", &LuaLoot::SetMoney },
        { "SetUnlootedCount", &LuaLoot::SetUnlootedCount },
        { "SetItemLooted", &LuaLoot::SetItemLooted },

        // Boolean
        { "IsLooted", &LuaLoot::IsLooted },
        { "HasItem", &LuaLoot::HasItem },

        // Other
        { "AddItem", &LuaLoot::AddItem },
        { "RemoveItem", &LuaLoot::RemoveItem },
        { "GenerateMoney", &LuaLoot::GenerateMoney },
        { "Clear", &LuaLoot::Clear },
        { "UpdateItemIndex", &LuaLoot::UpdateItemIndex },
    };
}

#endif
