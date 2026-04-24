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

#pragma once
#include <windows.h>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <string>
#include <cstdint>

namespace ml {
    class Script {
    public:
        enum class Status {
            RUNNING,
            WAITING,
            SUSPENDED,
            DEAD
        };

        Script(const std::string& filepath, const std::string& name);
        ~Script();

        bool Load();
        void Update();
        void Kill();

        Status GetStatus() const { return status; }
        const std::string& GetName() const { return name; }
        lua_State* GetState() const { return L; }

        void Wait(int ms);
        void SetGlobal(const char* name, const char* value);
        void SetGlobal(const char* name, int value);

    private:
        lua_State* L;
        lua_State* thread;
        std::string filepath;
        std::string name;
        Status status;
        int waitTime;
        DWORD waitStart;

        static int LuaWait(lua_State* L);
        static int LuaTerminate(lua_State* L);
    };
}