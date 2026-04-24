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

static int LuaPrint(lua_State* L) {
    int n = lua_gettop(L);
    std::string line;

    if (n == 0) {
        ml::Log::Info("[LUA] ");
        return 0;
    }

    lua_getglobal(L, "tostring");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        ml::Log::Info("[LUA] <tostring missing>");
        return 0;
    }

    for (int i = 1; i <= n; i++) {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        if (lua_pcall(L, 1, 1, 0) != 0) {
            ml::Log::Error("[LUA] print pcall error: " + std::string(lua_tostring(L, -1)));
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

    if (line.empty()) {
        ml::Log::Info("[LUA] <nil>");
    }
    else {
        ml::Log::Info("[LUA] " + line);
    }
    return 0;
}

static int LuaPanic(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    ml::Log::Error("LUA PANIC: " + std::string(msg ? msg : "unknown error"));
    return 0;
}

ml::Script::Script(const std::string& fp, const std::string& n)
    : filepath(fp), name(n), status(Status::RUNNING), waitTime(0), waitStart(0), thread(nullptr) {
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_atpanic(L, LuaPanic);
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
    lua_pushstring(L, name.c_str());
    lua_setglobal(L, "script_name");
    lua_pushstring(L, filepath.c_str());
    lua_setglobal(L, "script_path");

    lua_pushcfunction(L, LuaWait);
    lua_setglobal(L, "wait");
    lua_pushcfunction(L, LuaPrint);
    lua_setglobal(L, "print");

    RegisterOpcodes(L);

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
        Log::Error("Runtime error in " + name + ": " + lua_tostring(thread, -1));
        lua_pop(thread, 1);
        status = Status::DEAD;
        return false;
    }
    if (result == LUA_OK) status = Status::DEAD;
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
        Log::Error("Runtime error in " + name + ": " + lua_tostring(thread, -1));
        lua_pop(thread, 1);
        status = Status::DEAD;
    }
    else {
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
    lua_pushlightuserdata(L, (void*)L);
    lua_gettable(L, LUA_REGISTRYINDEX);
    Script* script = static_cast<Script*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (script) script->Wait(ms);
    return lua_yield(L, 0);
}