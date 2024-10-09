/*
* Copyright (C) 2010 - 2016 Forge Lua Engine <http://emudevs.com/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef VEHICLEMETHODS_H
#define VEHICLEMETHODS_H
#ifndef CLASSIC
#ifndef TBC

/***
 * Inherits all methods from: none
 */
namespace LuaVehicle
{
    /**
     * Returns true if the [Unit] passenger is on board
     *
     * @param [Unit] passenger
     * @return bool isOnBoard
     */
    int IsOnBoard(lua_State* L, Vehicle* vehicle)
    {
        Unit* passenger = Forge::CHECKOBJ<Unit>(L, 2);
#if defined TRINITY || AZEROTHCORE
        Forge::Push(L, passenger->IsOnVehicle(vehicle->GetBase()));
#else
        Forge::Push(L, vehicle->HasOnBoard(passenger));
#endif
        return 1;
    }

    /**
     * Returns the [Vehicle]'s owner
     *
     * @return [Unit] owner
     */
    int GetOwner(lua_State* L, Vehicle* vehicle)
    {
#if defined TRINITY || AZEROTHCORE
        Forge::Push(L, vehicle->GetBase());
#else
        Forge::Push(L, vehicle->GetOwner());
#endif
        return 1;
    }

    /**
     * Returns the [Vehicle]'s entry
     *
     * @return uint32 entry
     */
    int GetEntry(lua_State* L, Vehicle* vehicle)
    {
#ifdef TRINITY
        Forge::Push(L, vehicle->GetVehicleInfo()->ID);
#elif AZEROTHCORE
        Forge::Push(L, vehicle->GetVehicleInfo()->m_ID);
#else
        Forge::Push(L, vehicle->GetVehicleEntry()->m_ID);
#endif
        return 1;
    }

    /**
     * Returns the [Vehicle]'s passenger in the specified seat
     *
     * @param int8 seat
     * @return [Unit] passenger
     */
    int GetPassenger(lua_State* L, Vehicle* vehicle)
    {
        int8 seatId = Forge::CHECKVAL<int8>(L, 2);
        Forge::Push(L, vehicle->GetPassenger(seatId));
        return 1;
    }

    /**
     * Adds [Unit] passenger to a specified seat in the [Vehicle]
     *
     * @param [Unit] passenger
     * @param int8 seat
     */
    int AddPassenger(lua_State* L, Vehicle* vehicle)
    {
        Unit* passenger = Forge::CHECKOBJ<Unit>(L, 2);
        int8 seatId = Forge::CHECKVAL<int8>(L, 3);
#if defined TRINITY || AZEROTHCORE
        vehicle->AddPassenger(passenger, seatId);
#else
        if (vehicle->CanBoard(passenger))
            vehicle->Board(passenger, seatId);
#endif
        return 0;
    }

    /**
     * Removes [Unit] passenger from the [Vehicle]
     *
     * @param [Unit] passenger
     */
    int RemovePassenger(lua_State* L, Vehicle* vehicle)
    {
        Unit* passenger = Forge::CHECKOBJ<Unit>(L, 2);
#if defined TRINITY || AZEROTHCORE
        vehicle->RemovePassenger(passenger);
#else
        vehicle->UnBoard(passenger, false);
#endif
        return 0;
    }
}

#endif // CLASSIC
#endif // TBC
#endif // VEHICLEMETHODS_H
