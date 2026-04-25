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

#include "LuaManager.h"
#include "Log.h"
#include "Utils.h"
#include "Opcodes.h"
#include <windows.h>
#include <algorithm>

ml::LuaManager& ml::LuaManager::Get() {
    static LuaManager instance;
    return instance;
}

void ml::LuaManager::Init() {
    ScanDirectory();
    Log::System("LuaManager initialized with " + std::to_string(scripts.size()) + " scripts");
}

void ml::LuaManager::Shutdown() {
    scripts.clear();
}

void ml::LuaManager::Update() {
    UpdateCheatBuffer();

    for (auto& script : scripts) {
        if (script->GetStatus() != Script::Status::DEAD) {
            script->Update();
        }
    }
    scripts.erase(
        std::remove_if(scripts.begin(), scripts.end(),
            [](const auto& s) { return s->GetStatus() == Script::Status::DEAD; }),
        scripts.end()
    );
}

void ml::LuaManager::LoadScript(const std::string& path) {
    std::string name = path.substr(path.find_last_of("\\/") + 1);
    auto script = std::make_unique<Script>(path, name);
    if (script->Load()) {
        scripts.push_back(std::move(script));
    }
}

void ml::LuaManager::ReloadAll() {
    scripts.clear();
    ScanDirectory();
}

void ml::LuaManager::ScanDirectory() {
    std::string dir = GetMoonLoaderPath() + "\\*.lua";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(dir.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        CreateDirectoryA(GetMoonLoaderPath().c_str(), NULL);
        return;
    }
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            LoadScript(GetMoonLoaderPath() + "\\" + fd.cFileName);
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}