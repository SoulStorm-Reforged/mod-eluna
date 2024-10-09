/*
* Copyright (C) 2010 - 2016 Forge Lua Engine <http://emudevs.com/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "Chat.h"
#include "ForgeCompat.h"
#include "ForgeEventMgr.h"
#include "ForgeIncludes.h"
#include "ForgeTemplate.h"
#include "ForgeUtility.h"
#include "ForgeCreatureAI.h"
#include "ForgeInstanceAI.h"

#if defined(TRINITY_PLATFORM) && defined(TRINITY_PLATFORM_WINDOWS)
#if TRINITY_PLATFORM == TRINITY_PLATFORM_WINDOWS
#define FORGE_WINDOWS
#endif
#elif defined(AC_PLATFORM) && defined(AC_PLATFORM_WINDOWS)
#if AC_PLATFORM == AC_PLATFORM_WINDOWS
#define FORGE_WINDOWS
#endif
#elif defined(PLATFORM) && defined(PLATFORM_WINDOWS)
#if PLATFORM == PLATFORM_WINDOWS
#define FORGE_WINDOWS
#endif
#else
#error Forge could not determine platform
#endif

// Some dummy includes containing BOOST_VERSION:
// ObjectAccessor.h Config.h Log.h
#if !defined MANGOS
#define USING_BOOST
#endif

#ifdef USING_BOOST
#include <boost/filesystem.hpp>
#else
#include <ace/ACE.h>
#include <ace/Dirent.h>
#include <ace/OS_NS_sys_stat.h>
#endif

extern "C"
{
// Base lua libraries
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// Additional lua libraries
};

Forge::ScriptList Forge::lua_scripts;
Forge::ScriptList Forge::lua_extensions;
std::string Forge::lua_folderpath;
std::string Forge::m_requirePath;
std::string Forge::m_requirecPath;
Forge* Forge::GForge = NULL;
bool Forge::reload = false;
bool Forge::initialized = false;
Forge::LockType Forge::lock;

extern void RegisterFunctions(Forge* E);

void Forge::Initialize()
{
    LOCK_FORGE;
    ASSERT(!IsInitialized());

#if defined TRINITY || AZEROTHCORE
    // For instance data the data column needs to be able to hold more than 255 characters (tinytext)
    // so we change it to TEXT automatically on startup
    CharacterDatabase.DirectExecute("ALTER TABLE `instance` CHANGE COLUMN `data` `data` TEXT NOT NULL");
#endif

    LoadScriptPaths();

    // Must be before creating GForge
    // This is checked on Forge creation
    initialized = true;

    // Create global forge
    GForge = new Forge();
}

void Forge::Uninitialize()
{
    LOCK_FORGE;
    ASSERT(IsInitialized());

    delete GForge;
    GForge = NULL;

    lua_scripts.clear();
    lua_extensions.clear();

    initialized = false;
}

void Forge::LoadScriptPaths()
{
    uint32 oldMSTime = ForgeUtil::GetCurrTime();

    lua_scripts.clear();
    lua_extensions.clear();
    lua_folderpath = eConfigMgr->GetOption<std::string>("Forge.ScriptPath", "lua_scripts");
    const std::string& lua_path_extra = eConfigMgr->GetOption<std::string>("Forge.RequirePaths", "");
    const std::string& lua_cpath_extra = eConfigMgr->GetOption<std::string>("Forge.RequireCPaths", "");

#ifndef FORGE_WINDOWS
    if (lua_folderpath[0] == '~')
        if (const char* home = getenv("HOME"))
            lua_folderpath.replace(0, 1, home);
#endif

    FORGE_LOG_INFO("[Forge]: Searching scripts from `{}`", lua_folderpath);

    // clear all cache variables
    m_requirePath.clear();
    m_requirecPath.clear();

    GetScripts(lua_folderpath);

    // append our custom require paths and cpaths if the config variables are not empty
    if (!lua_path_extra.empty())
        m_requirePath += lua_path_extra;

    if (!lua_cpath_extra.empty())
        m_requirecPath += lua_cpath_extra;

    // Erase last ;
    if (!m_requirePath.empty())
        m_requirePath.erase(m_requirePath.end() - 1);

    if (!m_requirecPath.empty())
        m_requirecPath.erase(m_requirecPath.end() - 1);

    FORGE_LOG_DEBUG("[Forge]: Loaded {} scripts in {} ms", lua_scripts.size() + lua_extensions.size(), ForgeUtil::GetTimeDiff(oldMSTime));
}

void Forge::_ReloadForge()
{
    LOCK_FORGE;
    ASSERT(IsInitialized());

    if (eConfigMgr->GetOption<bool>("Forge.PlayerAnnounceReload", false))
        eWorld->SendServerMessage(SERVER_MSG_STRING, "Reloading Forge...");
    else
        ChatHandler(nullptr).SendGMText(SERVER_MSG_STRING, "Reloading Forge...");

    // Remove all timed events
    sForge->eventMgr->SetStates(LUAEVENT_STATE_ERASE);

    // Close lua
    sForge->CloseLua();

    // Reload script paths
    LoadScriptPaths();

    // Open new lua and libaraies
    sForge->OpenLua();

    // Run scripts from laoded paths
    sForge->RunScripts();

    reload = false;
}

Forge::Forge() :
event_level(0),
push_counter(0),
enabled(false),

L(NULL),
eventMgr(NULL),
httpManager(),
queryProcessor(),

ServerEventBindings(NULL),
PlayerEventBindings(NULL),
GuildEventBindings(NULL),
GroupEventBindings(NULL),
VehicleEventBindings(NULL),
BGEventBindings(NULL),

PacketEventBindings(NULL),
CreatureEventBindings(NULL),
CreatureGossipBindings(NULL),
GameObjectEventBindings(NULL),
GameObjectGossipBindings(NULL),
ItemEventBindings(NULL),
ItemGossipBindings(NULL),
PlayerGossipBindings(NULL),
MapEventBindings(NULL),
InstanceEventBindings(NULL),

CreatureUniqueBindings(NULL)
{
    ASSERT(IsInitialized());

    OpenLua();

    // Replace this with map insert if making multithread version

    // Set event manager. Must be after setting sForge
    // on multithread have a map of state pointers and here insert this pointer to the map and then save a pointer of that pointer to the EventMgr
    eventMgr = new EventMgr(&Forge::GForge);
}

Forge::~Forge()
{
    ASSERT(IsInitialized());

    CloseLua();

    delete eventMgr;
    eventMgr = NULL;
}

void Forge::CloseLua()
{
    OnLuaStateClose();

    DestroyBindStores();

    // Must close lua state after deleting stores and mgr
    if (L)
        lua_close(L);
    L = NULL;

    instanceDataRefs.clear();
    continentDataRefs.clear();
}

void Forge::OpenLua()
{
#if defined(AZEROTHCORE)
    enabled = eConfigMgr->GetOption<bool>("Forge.Enabled", true);
#else
    enabled = eConfigMgr->GetBoolDefault("Forge.Enabled", true);
#endif

    if (!IsEnabled())
    {
        FORGE_LOG_INFO("[Forge]: Forge is disabled in config");
        return;
    }

    L = luaL_newstate();

    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, FORGE_STATE_PTR);

    CreateBindStores();

    // open base lua libraries
    luaL_openlibs(L);

    // open additional lua libraries

    // Register methods and functions
    RegisterFunctions(this);

    // get require paths
    const std::string& requirepath = GetRequirePath();
    const std::string& requirecpath = GetRequireCPath();

    // Set lua require folder paths (scripts folder structure)
    lua_getglobal(L, "package");
    lua_pushstring(L, requirepath.c_str());
    lua_setfield(L, -2, "path");
    lua_pushstring(L, requirecpath.c_str());
    lua_setfield(L, -2, "cpath");

    // Set package.loaders loader for precompiled scripts
    lua_getfield(L, -1, "loaders");
    if (lua_isnil(L, -1)) {
        // Lua 5.2+ uses searchers instead of loaders
        lua_pop(L, 1);
        lua_getfield(L, -1, "searchers");
    }

    lua_pop(L, 1);
}

void Forge::CreateBindStores()
{
    DestroyBindStores();

    ServerEventBindings      = new BindingMap< EventKey<Hooks::ServerEvents> >(L);
    PlayerEventBindings      = new BindingMap< EventKey<Hooks::PlayerEvents> >(L);
    GuildEventBindings       = new BindingMap< EventKey<Hooks::GuildEvents> >(L);
    GroupEventBindings       = new BindingMap< EventKey<Hooks::GroupEvents> >(L);
    VehicleEventBindings     = new BindingMap< EventKey<Hooks::VehicleEvents> >(L);
    BGEventBindings          = new BindingMap< EventKey<Hooks::BGEvents> >(L);

    PacketEventBindings      = new BindingMap< EntryKey<Hooks::PacketEvents> >(L);
    CreatureEventBindings    = new BindingMap< EntryKey<Hooks::CreatureEvents> >(L);
    CreatureGossipBindings   = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    GameObjectEventBindings  = new BindingMap< EntryKey<Hooks::GameObjectEvents> >(L);
    GameObjectGossipBindings = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    ItemEventBindings        = new BindingMap< EntryKey<Hooks::ItemEvents> >(L);
    ItemGossipBindings       = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    PlayerGossipBindings     = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    MapEventBindings         = new BindingMap< EntryKey<Hooks::InstanceEvents> >(L);
    InstanceEventBindings    = new BindingMap< EntryKey<Hooks::InstanceEvents> >(L);

    CreatureUniqueBindings   = new BindingMap< UniqueObjectKey<Hooks::CreatureEvents> >(L);
}

void Forge::DestroyBindStores()
{
    delete ServerEventBindings;
    delete PlayerEventBindings;
    delete GuildEventBindings;
    delete GroupEventBindings;
    delete VehicleEventBindings;

    delete PacketEventBindings;
    delete CreatureEventBindings;
    delete CreatureGossipBindings;
    delete GameObjectEventBindings;
    delete GameObjectGossipBindings;
    delete ItemEventBindings;
    delete ItemGossipBindings;
    delete PlayerGossipBindings;
    delete BGEventBindings;
    delete MapEventBindings;
    delete InstanceEventBindings;

    delete CreatureUniqueBindings;

    ServerEventBindings = NULL;
    PlayerEventBindings = NULL;
    GuildEventBindings = NULL;
    GroupEventBindings = NULL;
    VehicleEventBindings = NULL;

    PacketEventBindings = NULL;
    CreatureEventBindings = NULL;
    CreatureGossipBindings = NULL;
    GameObjectEventBindings = NULL;
    GameObjectGossipBindings = NULL;
    ItemEventBindings = NULL;
    ItemGossipBindings = NULL;
    PlayerGossipBindings = NULL;
    BGEventBindings = NULL;
    MapEventBindings = NULL;
    InstanceEventBindings = NULL;

    CreatureUniqueBindings = NULL;
}

void Forge::AddScriptPath(std::string filename, const std::string& fullpath)
{
    FORGE_LOG_DEBUG("[Forge]: AddScriptPath Checking file `{}`", fullpath);

    // split file name
    std::size_t extDot = filename.find_last_of('.');
    if (extDot == std::string::npos)
        return;
    std::string ext = filename.substr(extDot);
    filename = filename.substr(0, extDot);

    // check extension and add path to scripts to load
    if (ext != ".lua" && ext != ".dll" && ext != ".so" && ext != ".ext" && ext != ".moon")
        return;
    bool extension = ext == ".ext";

    LuaScript script;
    script.fileext = ext;
    script.filename = filename;
    script.filepath = fullpath;
    script.modulepath = fullpath.substr(0, fullpath.length() - filename.length() - ext.length());
    if (extension)
        lua_extensions.push_back(script);
    else
        lua_scripts.push_back(script);
    FORGE_LOG_DEBUG("[Forge]: AddScriptPath add path `{}`", fullpath);
}

// Finds lua script files from given path (including subdirectories) and pushes them to scripts
void Forge::GetScripts(std::string path)
{
    FORGE_LOG_DEBUG("[Forge]: GetScripts from path `{}`", path);

#ifdef USING_BOOST
    boost::filesystem::path someDir(path);
    boost::filesystem::directory_iterator end_iter;

    if (boost::filesystem::exists(someDir) && boost::filesystem::is_directory(someDir))
    {
        m_requirePath +=
            path + "/?.lua;" +
            path + "/?.moon;" +
            path + "/?.ext;";

        m_requirecPath +=
            path + "/?.dll;" +
            path + "/?.so;";

        for (boost::filesystem::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
        {
            std::string fullpath = dir_iter->path().generic_string();

            // Check if file is hidden
#ifdef FORGE_WINDOWS
            DWORD dwAttrib = GetFileAttributes(fullpath.c_str());
            if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_HIDDEN))
                continue;
#else
            std::string name = dir_iter->path().filename().generic_string().c_str();
            if (name[0] == '.')
                continue;
#endif

            // load subfolder
            if (boost::filesystem::is_directory(dir_iter->status()))
            {
                GetScripts(fullpath);
                continue;
            }

            if (boost::filesystem::is_regular_file(dir_iter->status()))
            {
                // was file, try add
                std::string filename = dir_iter->path().filename().generic_string();
                AddScriptPath(filename, fullpath);
            }
        }
    }
#else
    ACE_Dirent dir;
    if (dir.open(path.c_str()) == -1) // Error opening directory, return
        return;

    m_requirePath +=
        path + "/?.lua;" +
        path + "/?.moon;" +
        path + "/?.ext;";

    m_requirecPath +=
        path + "/?.dll;" +
        path + "/?.so;";

    ACE_DIRENT *directory = 0;
    while ((directory = dir.read()))
    {
        // Skip the ".." and "." files.
        if (ACE::isdotdir(directory->d_name))
            continue;

        std::string fullpath = path + "/" + directory->d_name;

        // Check if file is hidden
#ifdef FORGE_WINDOWS
        DWORD dwAttrib = GetFileAttributes(fullpath.c_str());
        if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_HIDDEN))
            continue;
#else
        std::string name = directory->d_name;
        if (name[0] == '.')
            continue;
#endif

        ACE_stat stat_buf;
        if (ACE_OS::lstat(fullpath.c_str(), &stat_buf) == -1)
            continue;

        // load subfolder
        if ((stat_buf.st_mode & S_IFMT) == (S_IFDIR))
        {
            GetScripts(fullpath);
            continue;
        }

        // was file, try add
        std::string filename = directory->d_name;
        AddScriptPath(filename, fullpath);
    }
#endif
}

static bool ScriptPathComparator(const LuaScript& first, const LuaScript& second)
{
    return first.filepath < second.filepath;
}

void Forge::RunScripts()
{
    LOCK_FORGE;
    if (!IsEnabled())
        return;

    uint32 oldMSTime = ForgeUtil::GetCurrTime();
    uint32 count = 0;

    ScriptList scripts;
    lua_extensions.sort(ScriptPathComparator);
    lua_scripts.sort(ScriptPathComparator);
    scripts.insert(scripts.end(), lua_extensions.begin(), lua_extensions.end());
    scripts.insert(scripts.end(), lua_scripts.begin(), lua_scripts.end());

    std::unordered_map<std::string, std::string> loaded; // filename, path

    lua_getglobal(L, "package");
    // Stack: package
    luaL_getsubtable(L, -1, "loaded");
    // Stack: package, modules
    int modules = lua_gettop(L);
    for (ScriptList::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
    {
        // Check that no duplicate names exist
        if (loaded.find(it->filename) != loaded.end())
        {
            FORGE_LOG_ERROR("[Forge]: Error loading `{}`. File with same name already loaded from `{}`, rename either file", it->filepath, loaded[it->filename]);
            continue;
        }
        loaded[it->filename] = it->filepath;

        lua_getfield(L, modules, it->filename.c_str());
        // Stack: package, modules, module
        if (!lua_isnoneornil(L, -1))
        {
            lua_pop(L, 1);
            FORGE_LOG_DEBUG("[Forge]: `{}` was already loaded or required", it->filepath);
            continue;
        }
        lua_pop(L, 1);
        // Stack: package, modules

        if (it->fileext == ".moon")
        {
            std::string str = "return require('moonscript').loadfile([[" + it->filepath + "]])";
            if (luaL_loadstring(L, str.c_str()) || lua_pcall(L, 0, LUA_MULTRET, 0))
            {
                // Stack: package, modules, errmsg
                FORGE_LOG_ERROR("[Forge]: Error loading MoonScript `{}`", it->filepath);
                Report(L);
                // Stack: package, modules
                continue;
            }
        }
        else
        {
           if (luaL_loadfile(L, it->filepath.c_str()))
           {
               // Stack: package, modules, errmsg
               FORGE_LOG_ERROR("[Forge]: Error loading `{}`", it->filepath);
               Report(L);
               // Stack: package, modules
               continue;
           }
        }

        // Stack: package, modules, filefunc
        if (ExecuteCall(0, 1))
        {
            // Stack: package, modules, result
            if (lua_isnoneornil(L, -1) || (lua_isboolean(L, -1) && !lua_toboolean(L, -1)))
            {
                // if result evaluates to false, change it to true
                lua_pop(L, 1);
                Push(L, true);
            }
            lua_setfield(L, modules, it->filename.c_str());
            // Stack: package, modules

            // successfully loaded and ran file
            FORGE_LOG_DEBUG("[Forge]: Successfully loaded `{}`", it->filepath);
            ++count;
            continue;
        }
    }
    // Stack: package, modules
    lua_pop(L, 2);
    FORGE_LOG_INFO("[Forge]: Executed {} Lua scripts in {} ms", count, ForgeUtil::GetTimeDiff(oldMSTime));

    OnLuaStateOpen();
}

void Forge::InvalidateObjects()
{
    ++callstackid;
#ifdef TRINITY
    ASSERT(callstackid, "Callstackid overflow");
#else
    ASSERT(callstackid && "Callstackid overflow");
#endif
}

void Forge::Report(lua_State* _L)
{
    const char* msg = lua_tostring(_L, -1);
    FORGE_LOG_ERROR("{}", msg);
    lua_pop(_L, 1);
}

// Borrowed from http://stackoverflow.com/questions/12256455/print-stacktrace-from-c-code-with-embedded-lua
int Forge::StackTrace(lua_State *_L)
{
    // Stack: errmsg
    if (!lua_isstring(_L, -1))  /* 'message' not a string? */
        return 1;  /* keep it intact */
    // Stack: errmsg, debug
    lua_getglobal(_L, "debug");
    if (!lua_istable(_L, -1))
    {
        lua_pop(_L, 1);
        return 1;
    }
    // Stack: errmsg, debug, traceback
    lua_getfield(_L, -1, "traceback");
    if (!lua_isfunction(_L, -1))
    {
        lua_pop(_L, 2);
        return 1;
    }
    lua_pushvalue(_L, -3);  /* pass error message */
    lua_pushinteger(_L, 1);  /* skip this function and traceback */
    // Stack: errmsg, debug, traceback, errmsg, 2
    lua_call(_L, 2, 1);  /* call debug.traceback */

    // dirty stack?
    // Stack: errmsg, debug, tracemsg
    sForge->OnError(std::string(lua_tostring(_L, -1)));
    return 1;
}

bool Forge::ExecuteCall(int params, int res)
{
    int top = lua_gettop(L);
    int base = top - params;

    // Expected: function, [parameters]
    ASSERT(base > 0);

    // Check function type
    if (!lua_isfunction(L, base))
    {
        FORGE_LOG_ERROR("[Forge]: Cannot execute call: registered value is {}, not a function.", luaL_tolstring(L, base, NULL));
        ASSERT(false); // stack probably corrupt
    }

#if defined(AZEROTHCORE)
    bool usetrace = eConfigMgr->GetOption<bool>("Forge.TraceBack", false);
#else
    bool usetrace = eConfigMgr->GetBoolDefault("Forge.TraceBack", false);
#endif

    if (usetrace)
    {
        lua_pushcfunction(L, &StackTrace);
        // Stack: function, [parameters], traceback
        lua_insert(L, base);
        // Stack: traceback, function, [parameters]
    }

    // Objects are invalidated when event_level hits 0
    ++event_level;
    int result = lua_pcall(L, params, res, usetrace ? base : 0);
    --event_level;

    if (usetrace)
    {
        // Stack: traceback, [results or errmsg]
        lua_remove(L, base);
    }
    // Stack: [results or errmsg]

    // lua_pcall returns 0 on success.
    // On error print the error and push nils for expected amount of returned values
    if (result)
    {
        // Stack: errmsg
        Report(L);

        // Force garbage collect
        lua_gc(L, LUA_GCCOLLECT, 0);

        // Push nils for expected amount of results
        for (int i = 0; i < res; ++i)
            lua_pushnil(L);
        // Stack: [nils]
        return false;
    }

    // Stack: [results]
    return true;
}

void Forge::Push(lua_State* luastate)
{
    lua_pushnil(luastate);
}
void Forge::Push(lua_State* luastate, const long long l)
{
    ForgeTemplate<long long>::Push(luastate, new long long(l));
}
void Forge::Push(lua_State* luastate, const unsigned long long l)
{
    ForgeTemplate<unsigned long long>::Push(luastate, new unsigned long long(l));
}
void Forge::Push(lua_State* luastate, const long l)
{
    Push(luastate, static_cast<long long>(l));
}
void Forge::Push(lua_State* luastate, const unsigned long l)
{
    Push(luastate, static_cast<unsigned long long>(l));
}
void Forge::Push(lua_State* luastate, const int i)
{
    lua_pushinteger(luastate, i);
}
void Forge::Push(lua_State* luastate, const unsigned int u)
{
    lua_pushunsigned(luastate, u);
}
void Forge::Push(lua_State* luastate, const double d)
{
    lua_pushnumber(luastate, d);
}
void Forge::Push(lua_State* luastate, const float f)
{
    lua_pushnumber(luastate, f);
}
void Forge::Push(lua_State* luastate, const bool b)
{
    lua_pushboolean(luastate, b);
}
void Forge::Push(lua_State* luastate, const std::string& str)
{
    lua_pushstring(luastate, str.c_str());
}
void Forge::Push(lua_State* luastate, const char* str)
{
    lua_pushstring(luastate, str);
}
void Forge::Push(lua_State* luastate, Pet const* pet)
{
    Push<Creature>(luastate, pet);
}
void Forge::Push(lua_State* luastate, TempSummon const* summon)
{
    Push<Creature>(luastate, summon);
}
void Forge::Push(lua_State* luastate, Unit const* unit)
{
    if (!unit)
    {
        Push(luastate);
        return;
    }
    switch (unit->GetTypeId())
    {
        case TYPEID_UNIT:
            Push(luastate, unit->ToCreature());
            break;
        case TYPEID_PLAYER:
            Push(luastate, unit->ToPlayer());
            break;
        default:
            ForgeTemplate<Unit>::Push(luastate, unit);
    }
}
void Forge::Push(lua_State* luastate, WorldObject const* obj)
{
    if (!obj)
    {
        Push(luastate);
        return;
    }
    switch (obj->GetTypeId())
    {
        case TYPEID_UNIT:
            Push(luastate, obj->ToCreature());
            break;
        case TYPEID_PLAYER:
            Push(luastate, obj->ToPlayer());
            break;
        case TYPEID_GAMEOBJECT:
            Push(luastate, obj->ToGameObject());
            break;
        case TYPEID_CORPSE:
            Push(luastate, obj->ToCorpse());
            break;
        default:
            ForgeTemplate<WorldObject>::Push(luastate, obj);
    }
}
void Forge::Push(lua_State* luastate, Object const* obj)
{
    if (!obj)
    {
        Push(luastate);
        return;
    }
    switch (obj->GetTypeId())
    {
        case TYPEID_UNIT:
            Push(luastate, obj->ToCreature());
            break;
        case TYPEID_PLAYER:
            Push(luastate, obj->ToPlayer());
            break;
        case TYPEID_GAMEOBJECT:
            Push(luastate, obj->ToGameObject());
            break;
        case TYPEID_CORPSE:
            Push(luastate, obj->ToCorpse());
            break;
        default:
            ForgeTemplate<Object>::Push(luastate, obj);
    }
}
void Forge::Push(lua_State* luastate, ObjectGuid const guid)
{
    ForgeTemplate<unsigned long long>::Push(luastate, new unsigned long long(guid.GetRawValue()));
}

void Forge::Push(lua_State* luastate, SpellEffectInfo const& spellEffectInfo)
{
    Push(luastate, &spellEffectInfo);
}

static int CheckIntegerRange(lua_State* luastate, int narg, int min, int max)
{
    double value = luaL_checknumber(luastate, narg);
    char error_buffer[64];

    if (value > max)
    {
        snprintf(error_buffer, 64, "value must be less than or equal to %i", max);
        return luaL_argerror(luastate, narg, error_buffer);
    }

    if (value < min)
    {
        snprintf(error_buffer, 64, "value must be greater than or equal to %i", min);
        return luaL_argerror(luastate, narg, error_buffer);
    }

    return static_cast<int>(value);
}

static unsigned int CheckUnsignedRange(lua_State* luastate, int narg, unsigned int max)
{
    double value = luaL_checknumber(luastate, narg);

    if (value < 0)
        return luaL_argerror(luastate, narg, "value must be greater than or equal to 0");

    if (value > max)
    {
        char error_buffer[64];
        snprintf(error_buffer, 64, "value must be less than or equal to %u", max);
        return luaL_argerror(luastate, narg, error_buffer);
    }

    return static_cast<unsigned int>(value);
}

template<> bool Forge::CHECKVAL<bool>(lua_State* luastate, int narg)
{
    return lua_toboolean(luastate, narg) != 0;
}
template<> float Forge::CHECKVAL<float>(lua_State* luastate, int narg)
{
    return static_cast<float>(luaL_checknumber(luastate, narg));
}
template<> double Forge::CHECKVAL<double>(lua_State* luastate, int narg)
{
    return luaL_checknumber(luastate, narg);
}
template<> signed char Forge::CHECKVAL<signed char>(lua_State* luastate, int narg)
{
    return CheckIntegerRange(luastate, narg, SCHAR_MIN, SCHAR_MAX);
}
template<> unsigned char Forge::CHECKVAL<unsigned char>(lua_State* luastate, int narg)
{
    return CheckUnsignedRange(luastate, narg, UCHAR_MAX);
}
template<> short Forge::CHECKVAL<short>(lua_State* luastate, int narg)
{
    return CheckIntegerRange(luastate, narg, SHRT_MIN, SHRT_MAX);
}
template<> unsigned short Forge::CHECKVAL<unsigned short>(lua_State* luastate, int narg)
{
    return CheckUnsignedRange(luastate, narg, USHRT_MAX);
}
template<> int Forge::CHECKVAL<int>(lua_State* luastate, int narg)
{
    return CheckIntegerRange(luastate, narg, INT_MIN, INT_MAX);
}
template<> unsigned int Forge::CHECKVAL<unsigned int>(lua_State* luastate, int narg)
{
    return CheckUnsignedRange(luastate, narg, UINT_MAX);
}
template<> const char* Forge::CHECKVAL<const char*>(lua_State* luastate, int narg)
{
    return luaL_checkstring(luastate, narg);
}
template<> std::string Forge::CHECKVAL<std::string>(lua_State* luastate, int narg)
{
    return luaL_checkstring(luastate, narg);
}
template<> long long Forge::CHECKVAL<long long>(lua_State* luastate, int narg)
{
    if (lua_isnumber(luastate, narg))
        return static_cast<long long>(CHECKVAL<double>(luastate, narg));
    return *(Forge::CHECKOBJ<long long>(luastate, narg, true));
}
template<> unsigned long long Forge::CHECKVAL<unsigned long long>(lua_State* luastate, int narg)
{
    if (lua_isnumber(luastate, narg))
        return static_cast<unsigned long long>(CHECKVAL<uint32>(luastate, narg));
    return *(Forge::CHECKOBJ<unsigned long long>(luastate, narg, true));
}
template<> long Forge::CHECKVAL<long>(lua_State* luastate, int narg)
{
    return static_cast<long>(CHECKVAL<long long>(luastate, narg));
}
template<> unsigned long Forge::CHECKVAL<unsigned long>(lua_State* luastate, int narg)
{
    return static_cast<unsigned long>(CHECKVAL<unsigned long long>(luastate, narg));
}
template<> ObjectGuid Forge::CHECKVAL<ObjectGuid>(lua_State* luastate, int narg)
{
    return ObjectGuid(uint64((CHECKVAL<unsigned long long>(luastate, narg))));
}

template<> Object* Forge::CHECKOBJ<Object>(lua_State* luastate, int narg, bool error)
{
    Object* obj = CHECKOBJ<WorldObject>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<Item>(luastate, narg, false);
    if (!obj)
        obj = ForgeTemplate<Object>::Check(luastate, narg, error);
    return obj;
}
template<> WorldObject* Forge::CHECKOBJ<WorldObject>(lua_State* luastate, int narg, bool error)
{
    WorldObject* obj = CHECKOBJ<Unit>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<GameObject>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<Corpse>(luastate, narg, false);
    if (!obj)
        obj = ForgeTemplate<WorldObject>::Check(luastate, narg, error);
    return obj;
}
template<> Unit* Forge::CHECKOBJ<Unit>(lua_State* luastate, int narg, bool error)
{
    Unit* obj = CHECKOBJ<Player>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<Creature>(luastate, narg, false);
    if (!obj)
        obj = ForgeTemplate<Unit>::Check(luastate, narg, error);
    return obj;
}

template<> ForgeObject* Forge::CHECKOBJ<ForgeObject>(lua_State* luastate, int narg, bool error)
{
    return CHECKTYPE(luastate, narg, NULL, error);
}

ForgeObject* Forge::CHECKTYPE(lua_State* luastate, int narg, const char* tname, bool error)
{
    if (lua_islightuserdata(luastate, narg))
    {
        if (error)
            luaL_argerror(luastate, narg, "bad argument : userdata expected, got lightuserdata");
        return NULL;
    }

    ForgeObject** ptrHold = static_cast<ForgeObject**>(lua_touserdata(luastate, narg));

    if (!ptrHold || (tname && (*ptrHold)->GetTypeName() != tname))
    {
        if (error)
        {
            char buff[256];
            snprintf(buff, 256, "bad argument : %s expected, got %s", tname ? tname : "ForgeObject", ptrHold ? (*ptrHold)->GetTypeName() : luaL_typename(luastate, narg));
            luaL_argerror(luastate, narg, buff);
        }
        return NULL;
    }
    return *ptrHold;
}

template<typename K>
static int cancelBinding(lua_State *L)
{
    uint64 bindingID = Forge::CHECKVAL<uint64>(L, lua_upvalueindex(1));

    BindingMap<K>* bindings = (BindingMap<K>*)lua_touserdata(L, lua_upvalueindex(2));
    ASSERT(bindings != NULL);

    bindings->Remove(bindingID);

    return 0;
}

template<typename K>
static void createCancelCallback(lua_State* L, uint64 bindingID, BindingMap<K>* bindings)
{
    Forge::Push(L, bindingID);
    lua_pushlightuserdata(L, bindings);
    // Stack: bindingID, bindings

    lua_pushcclosure(L, &cancelBinding<K>, 2);
    // Stack: cancel_callback
}

// Saves the function reference ID given to the register type's store for given entry under the given event
int Forge::Register(lua_State* L, uint8 regtype, uint32 entry, ObjectGuid guid, uint32 instanceId, uint32 event_id, int functionRef, uint32 shots)
{
    uint64 bindingID;

    switch (regtype)
    {
        case Hooks::REGTYPE_SERVER:
            if (event_id < Hooks::SERVER_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::ServerEvents>((Hooks::ServerEvents)event_id);
                bindingID = ServerEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, ServerEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_PLAYER:
            if (event_id < Hooks::PLAYER_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::PlayerEvents>((Hooks::PlayerEvents)event_id);
                bindingID = PlayerEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, PlayerEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GUILD:
            if (event_id < Hooks::GUILD_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::GuildEvents>((Hooks::GuildEvents)event_id);
                bindingID = GuildEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GuildEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GROUP:
            if (event_id < Hooks::GROUP_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::GroupEvents>((Hooks::GroupEvents)event_id);
                bindingID = GroupEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GroupEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_VEHICLE:
            if (event_id < Hooks::VEHICLE_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::VehicleEvents>((Hooks::VehicleEvents)event_id);
                bindingID = VehicleEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, VehicleEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_BG:
            if (event_id < Hooks::BG_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::BGEvents>((Hooks::BGEvents)event_id);
                bindingID = BGEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, BGEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_PACKET:
            if (event_id < Hooks::PACKET_EVENT_COUNT)
            {
                if (entry >= NUM_MSG_TYPES)
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a creature with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::PacketEvents>((Hooks::PacketEvents)event_id, entry);
                bindingID = PacketEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, PacketEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_CREATURE:
            if (event_id < Hooks::CREATURE_EVENT_COUNT)
            {
                if (entry != 0)
                {
                    if (!eObjectMgr->GetCreatureTemplate(entry))
                    {
                        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                        luaL_error(L, "Couldn't find a creature with (ID: %d)!", entry);
                        return 0; // Stack: (empty)
                    }

                    auto key = EntryKey<Hooks::CreatureEvents>((Hooks::CreatureEvents)event_id, entry);
                    bindingID = CreatureEventBindings->Insert(key, functionRef, shots);
                    createCancelCallback(L, bindingID, CreatureEventBindings);
                }
                else
                {
                    if (guid.IsEmpty())
                    {
                        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                        luaL_error(L, "guid was 0!");
                        return 0; // Stack: (empty)
                    }

                    auto key = UniqueObjectKey<Hooks::CreatureEvents>((Hooks::CreatureEvents)event_id, guid, instanceId);
                    bindingID = CreatureUniqueBindings->Insert(key, functionRef, shots);
                    createCancelCallback(L, bindingID, CreatureUniqueBindings);
                }
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_CREATURE_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!eObjectMgr->GetCreatureTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a creature with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = CreatureGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, CreatureGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GAMEOBJECT:
            if (event_id < Hooks::GAMEOBJECT_EVENT_COUNT)
            {
                if (!eObjectMgr->GetGameObjectTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a gameobject with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GameObjectEvents>((Hooks::GameObjectEvents)event_id, entry);
                bindingID = GameObjectEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GameObjectEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GAMEOBJECT_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!eObjectMgr->GetGameObjectTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a gameobject with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = GameObjectGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GameObjectGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_ITEM:
            if (event_id < Hooks::ITEM_EVENT_COUNT)
            {
                if (!eObjectMgr->GetItemTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a item with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::ItemEvents>((Hooks::ItemEvents)event_id, entry);
                bindingID = ItemEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, ItemEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_ITEM_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!eObjectMgr->GetItemTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a item with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = ItemGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, ItemGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_PLAYER_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = PlayerGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, PlayerGossipBindings);
                return 1; // Stack: callback
            }
            break;
        case Hooks::REGTYPE_MAP:
            if (event_id < Hooks::INSTANCE_EVENT_COUNT)
            {
                auto key = EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)event_id, entry);
                bindingID = MapEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, MapEventBindings);
                return 1; // Stack: callback
            }
            break;
        case Hooks::REGTYPE_INSTANCE:
            if (event_id < Hooks::INSTANCE_EVENT_COUNT)
            {
                auto key = EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)event_id, entry);
                bindingID = InstanceEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, InstanceEventBindings);
                return 1; // Stack: callback
            }
            break;
    }
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    std::ostringstream oss;
    oss << "regtype " << static_cast<uint32>(regtype) << ", event " << event_id << ", entry " << entry << ", guid " << guid.GetRawValue() << ", instance " << instanceId;
    luaL_error(L, "Unknown event type (%s)", oss.str().c_str());
    return 0;
}

/*
 * Cleans up the stack, effectively undoing all Push calls and the Setup call.
 */
void Forge::CleanUpStack(int number_of_arguments)
{
    // Stack: event_id, [arguments]

    lua_pop(L, number_of_arguments + 1); // Add 1 because the caller doesn't know about `event_id`.
    // Stack: (empty)

    if (event_level == 0)
        InvalidateObjects();
}

/*
 * Call a single event handler that was put on the stack with `Setup` and removes it from the stack.
 *
 * The caller is responsible for keeping track of how many times this should be called.
 */
int Forge::CallOneFunction(int number_of_functions, int number_of_arguments, int number_of_results)
{
    ++number_of_arguments; // Caller doesn't know about `event_id`.
    ASSERT(number_of_functions > 0 && number_of_arguments > 0 && number_of_results >= 0);
    // Stack: event_id, [arguments], [functions]

    int functions_top        = lua_gettop(L);
    int first_function_index = functions_top - number_of_functions + 1;
    int arguments_top        = first_function_index - 1;
    int first_argument_index = arguments_top - number_of_arguments + 1;

    // Copy the arguments from the bottom of the stack to the top.
    for (int argument_index = first_argument_index; argument_index <= arguments_top; ++argument_index)
    {
        lua_pushvalue(L, argument_index);
    }
    // Stack: event_id, [arguments], [functions], event_id, [arguments]

    ExecuteCall(number_of_arguments, number_of_results);
    --functions_top;
    // Stack: event_id, [arguments], [functions - 1], [results]

    return functions_top + 1; // Return the location of the first result (if any exist).
}

CreatureAI* Forge::GetAI(Creature* creature)
{
    if (!IsEnabled())
        return NULL;

    for (int i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
    {
        Hooks::CreatureEvents event_id = (Hooks::CreatureEvents)i;

        auto entryKey = EntryKey<Hooks::CreatureEvents>(event_id, creature->GetEntry());
        auto uniqueKey = UniqueObjectKey<Hooks::CreatureEvents>(event_id, creature->GET_GUID(), creature->GetInstanceId());

        if (CreatureEventBindings->HasBindingsFor(entryKey) ||
            CreatureUniqueBindings->HasBindingsFor(uniqueKey))
            return new ForgeCreatureAI(creature);
    }

    return NULL;
}

InstanceData* Forge::GetInstanceData(Map* map)
{
    if (!IsEnabled())
        return NULL;

    for (int i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
    {
        Hooks::InstanceEvents event_id = (Hooks::InstanceEvents)i;

        auto key = EntryKey<Hooks::InstanceEvents>(event_id, map->GetId());

        if (MapEventBindings->HasBindingsFor(key) ||
            InstanceEventBindings->HasBindingsFor(key))
            return new ForgeInstanceAI(map);
    }

    return NULL;
}

bool Forge::HasInstanceData(Map const* map)
{
    if (!map->Instanceable())
        return continentDataRefs.find(map->GetId()) != continentDataRefs.end();
    else
        return instanceDataRefs.find(map->GetInstanceId()) != instanceDataRefs.end();
}

void Forge::CreateInstanceData(Map const* map)
{
    ASSERT(lua_istable(L, -1));
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (!map->Instanceable())
    {
        uint32 mapId = map->GetId();

        // If there's another table that was already stored for the map, unref it.
        auto mapRef = continentDataRefs.find(mapId);
        if (mapRef != continentDataRefs.end())
        {
            luaL_unref(L, LUA_REGISTRYINDEX, mapRef->second);
        }

        continentDataRefs[mapId] = ref;
    }
    else
    {
        uint32 instanceId = map->GetInstanceId();

        // If there's another table that was already stored for the instance, unref it.
        auto instRef = instanceDataRefs.find(instanceId);
        if (instRef != instanceDataRefs.end())
        {
            luaL_unref(L, LUA_REGISTRYINDEX, instRef->second);
        }

        instanceDataRefs[instanceId] = ref;
    }
}

/*
 * Unrefs the instanceId related events and data
 * Does all required actions for when an instance is freed.
 */
void Forge::FreeInstanceId(uint32 instanceId)
{
    LOCK_FORGE;

    if (!IsEnabled())
        return;

    for (int i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
    {
        auto key = EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)i, instanceId);

        if (MapEventBindings->HasBindingsFor(key))
            MapEventBindings->Clear(key);

        if (InstanceEventBindings->HasBindingsFor(key))
            InstanceEventBindings->Clear(key);

        if (instanceDataRefs.find(instanceId) != instanceDataRefs.end())
        {
            luaL_unref(L, LUA_REGISTRYINDEX, instanceDataRefs[instanceId]);
            instanceDataRefs.erase(instanceId);
        }
    }
}

void Forge::PushInstanceData(lua_State* L, ForgeInstanceAI* ai, bool incrementCounter)
{
    // Check if the instance data is missing (i.e. someone reloaded Forge).
    if (!HasInstanceData(ai->instance))
        ai->Reload();

    // Get the instance data table from the registry.
    if (!ai->instance->Instanceable())
        lua_rawgeti(L, LUA_REGISTRYINDEX, continentDataRefs[ai->instance->GetId()]);
    else
        lua_rawgeti(L, LUA_REGISTRYINDEX, instanceDataRefs[ai->instance->GetInstanceId()]);

    ASSERT(lua_istable(L, -1));

    if (incrementCounter)
        ++push_counter;
}
