/*
 * MIT License
 * MoonLoader III
 *
 * Copyright (c) 2026 MrRealistic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This project uses Plugin-SDK (https://github.com/DK22Pac/plugin-sdk) and
 * LuaJIT (https://luajit.org/), each under their respective licenses.
 */

#include "Script.h"
#include "Log.h"
#include "Opcodes.h"
#include "Utils.h"

int ml::Script::nextScriptId = 1;

static ml::Script* GetScriptFromState(lua_State* L) {
    lua_pushlightuserdata(L, (void*)L);
    lua_gettable(L, LUA_REGISTRYINDEX);
    auto* s = static_cast<ml::Script*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return s;
}

int ml::Script::LuaPrint(lua_State* L) {
    int n = lua_gettop(L);
    std::string line;
    auto* script = GetScriptFromState(L);
    std::string prefix = "[LUA]";
    if (script && !script->GetDisplayName().empty()) {
        prefix = script->GetDisplayName();
    }

    lua_getglobal(L, "tostring");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        ml::Log::System(prefix + ": <tostring missing>");
        return 0;
    }

    for (int i = 1; i <= n; i++) {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        if (lua_pcall(L, 1, 1, 0) != 0) {
            lua_pop(L, 1);
            continue;
        }
        const char* s = lua_tostring(L, -1);
        if (s) {
            if (i > 1) line += "\t";
            line += s;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    ml::Log::System(prefix + ": " + (line.empty() ? "<nil>" : line));
    return 0;
}

static int LuaPanic(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    ml::Log::Error("LUA PANIC: " + std::string(msg ? msg : "unknown error"));
    return 0;
}

ml::Script::Script(const std::string& fp, const std::string& n)
    : filepath(fp), name(n), displayName(n), id(nextScriptId++),
    status(Status::RUNNING), waitTime(0), waitStart(0), thread(nullptr) {
    size_t dot = displayName.find_last_of('.');
    if (dot != std::string::npos) displayName = displayName.substr(0, dot);

    L = luaL_newstate();
    luaL_openlibs(L);
    lua_atpanic(L, LuaPanic);

    // --- NEW: Setup Library Search Paths ---
    std::string libPath = GetMoonLoaderPath() + "\\lib\\";

    lua_getglobal(L, "package");

    // 1. Update package.path to find .lua files in moonloader\lib
    lua_getfield(L, -1, "path");
    std::string currentPath = lua_tostring(L, -1);
    lua_pop(L, 1);
    currentPath += ";" + libPath + "?.lua;" + libPath + "?\\init.lua";
    lua_pushstring(L, currentPath.c_str());
    lua_setfield(L, -2, "path");

    // 2. Update package.cpath to find .dll files in moonloader\lib
    lua_getfield(L, -1, "cpath");
    std::string currentCPath = lua_tostring(L, -1);
    lua_pop(L, 1);
    currentCPath += ";" + libPath + "?.dll";
    lua_pushstring(L, currentCPath.c_str());
    lua_setfield(L, -2, "cpath");

    lua_pop(L, 1); // Pop the "package" table off the stack
    // ---------------------------------------
}

ml::Script::~Script() {
    if (L) {
        if (thread) {
            lua_pushlightuserdata(L, (void*)thread);
            lua_pushnil(L);
            lua_settable(L, LUA_REGISTRYINDEX);
        }
        lua_pushlightuserdata(L, (void*)this);
        lua_pushnil(L);
        lua_settable(L, LUA_REGISTRYINDEX);
        lua_close(L);
    }
}

bool ml::Script::Load() {
    lua_pushstring(L, filepath.c_str());
    lua_setglobal(L, "script_path");

    lua_pushcfunction(L, LuaWait);
    lua_setglobal(L, "wait");
    lua_pushcfunction(L, LuaPrint);
    lua_setglobal(L, "print");

    lua_pushcfunction(L, LuaScriptName);
    lua_setglobal(L, "script_name");
    lua_pushcfunction(L, LuaScriptAuthor);
    lua_setglobal(L, "script_author");
    lua_pushcfunction(L, LuaScriptDescription);
    lua_setglobal(L, "script_description");
    lua_pushcfunction(L, LuaScriptVersion);
    lua_setglobal(L, "script_version");

    RegisterOpcodes(L);

    Log::System("Loading script \"" + filepath + "\"...\t(id:" + std::to_string(id) + ")");

    if (luaL_loadfile(L, filepath.c_str()) != LUA_OK) {
        Log::Error("Failed to load " + name + ": " + lua_tostring(L, -1));
        lua_pop(L, 1);
        status = Status::DEAD;
        return false;
    }

    thread = lua_newthread(L);
    lua_pushvalue(L, -2);
    lua_xmove(L, thread, 1);

    lua_pushlightuserdata(L, (void*)this);
    lua_pushvalue(L, -2);
    lua_settable(L, LUA_REGISTRYINDEX);

    lua_pop(L, 2);

    lua_pushlightuserdata(L, (void*)thread);
    lua_pushlightuserdata(L, (void*)this);
    lua_settable(L, LUA_REGISTRYINDEX);

    int result = lua_resume(thread, 0);
    if (result != LUA_OK && result != LUA_YIELD) {
        Log::Error("Runtime error in " + displayName + ": " + lua_tostring(thread, -1));
        lua_pop(thread, 1);
        status = Status::DEAD;
        return false;
    }
    if (result == LUA_OK) {
        Log::System(displayName + ": Script terminated. (id:" + std::to_string(id) + ")");
        status = Status::DEAD;
    }
    else {
        Log::System(displayName + ": Loaded successfully.");
    }
    return true;
}

void ml::Script::Update() {
    if (status == Status::DEAD) return;

    if (status == Status::WAITING) {
        if ((GetTickCount() - waitStart) >= static_cast<DWORD>(waitTime)) {
            status = Status::RUNNING;
        }
        else {
            return;
        }
    }

    int result = lua_resume(thread, 0);
    if (result == LUA_YIELD) {
        return;
    }
    else if (result != LUA_OK) {
        Log::Error("Runtime error in " + displayName + ": " + lua_tostring(thread, -1));
        lua_pop(thread, 1);
        status = Status::DEAD;
    }
    else {
        Log::System(displayName + ": Script terminated. (id:" + std::to_string(id) + ")");
        status = Status::DEAD;
    }
}

void ml::Script::Kill() {
    status = Status::DEAD;
}

void ml::Script::Wait(int ms) {
    waitTime = ms;
    waitStart = GetTickCount();
    status = Status::WAITING;
}

void ml::Script::SetGlobal(const char* name, const char* value) {
    lua_pushstring(L, value);
    lua_setglobal(L, name);
}

void ml::Script::SetGlobal(const char* name, int value) {
    lua_pushinteger(L, value);
    lua_setglobal(L, name);
}

int ml::Script::LuaWait(lua_State* L) {
    int ms = luaL_checkint(L, 1);
    auto* script = GetScriptFromState(L);
    if (script) script->Wait(ms);
    return lua_yield(L, 0);
}

int ml::Script::LuaScriptName(lua_State* L) {
    auto* script = GetScriptFromState(L);
    if (!script) return 0;
    if (lua_gettop(L) >= 1 && lua_isstring(L, 1)) {
        script->displayName = lua_tostring(L, 1);
        return 0;
    }
    lua_pushstring(L, script->displayName.c_str());
    return 1;
}

int ml::Script::LuaScriptAuthor(lua_State* L) {
    auto* script = GetScriptFromState(L);
    if (script && lua_gettop(L) >= 1 && lua_isstring(L, 1)) {
        script->author = lua_tostring(L, 1);
    }
    return 0;
}

int ml::Script::LuaScriptDescription(lua_State* L) {
    auto* script = GetScriptFromState(L);
    if (script && lua_gettop(L) >= 1 && lua_isstring(L, 1)) {
        script->description = lua_tostring(L, 1);
    }
    return 0;
}

int ml::Script::LuaScriptVersion(lua_State* L) {
    auto* script = GetScriptFromState(L);
    if (script && lua_gettop(L) >= 1 && lua_isstring(L, 1)) {
        script->version = lua_tostring(L, 1);
    }
    return 0;
}