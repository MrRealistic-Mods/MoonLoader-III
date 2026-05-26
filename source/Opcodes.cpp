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

#include "Opcodes.h"
#include "Script.h"
#include "Log.h"
#include "Utils.h"
#include <plugin_III.h>
#include <CCivilianPed.h>
#include <CAutomobile.h>
#include <CStreaming.h>
#include <CWorld.h>
#include <CPed.h>
#include <CVehicle.h>
#include <CPlayerInfo.h>
#include <common.h>
#include <CCamera.h>
#include <CTimer.h>
#include <CFont.h>
#include <CWeaponInfo.h>
#include <eWeaponType.h>
#include <CPad.h>
#include <CPools.h>
#include <CPool.h>

using namespace plugin;

static std::string g_CheatBuffer;

void ml::UpdateCheatBuffer() {
    static bool keyStates[256] = { false };

    for (int vk = 'A'; vk <= 'Z'; vk++) {
        bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
        if (down && !keyStates[vk]) {
            g_CheatBuffer += static_cast<char>(vk);
        }
        keyStates[vk] = down;
    }
    for (int vk = '0'; vk <= '9'; vk++) {
        bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
        if (down && !keyStates[vk]) {
            g_CheatBuffer += static_cast<char>(vk);
        }
        keyStates[vk] = down;
    }
    {
        bool down = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        if (down && !keyStates[VK_SPACE]) {
            g_CheatBuffer += ' ';
        }
        keyStates[VK_SPACE] = down;
    }

    if (g_CheatBuffer.length() > 32) {
        g_CheatBuffer = g_CheatBuffer.substr(g_CheatBuffer.length() - 32);
    }
}

static ml::Script* GetScript(lua_State* L) {
    lua_pushlightuserdata(L, (void*)L);
    lua_gettable(L, LUA_REGISTRYINDEX);
    auto* s = static_cast<ml::Script*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return s;
}

void ml::RegisterOpcodes(lua_State* L) {
    lua_pushcfunction(L, opcode_wait);                  lua_setglobal(L, "wait");
    lua_pushcfunction(L, opcode_test_cheat);            lua_setglobal(L, "test_cheat");
    lua_pushcfunction(L, opcode_is_key_pressed);        lua_setglobal(L, "is_key_pressed");
    lua_pushcfunction(L, opcode_get_frame_delta_time);  lua_setglobal(L, "get_frame_delta_time");
    lua_pushcfunction(L, opcode_get_camera_position);   lua_setglobal(L, "get_camera_position");
    lua_pushcfunction(L, opcode_get_camera_target);     lua_setglobal(L, "get_camera_target");
    lua_pushcfunction(L, opcode_get_player_ped);        lua_setglobal(L, "get_player_ped");
    lua_pushcfunction(L, opcode_is_player_playing);     lua_setglobal(L, "is_player_playing");
    lua_pushcfunction(L, opcode_is_char_in_any_car);    lua_setglobal(L, "is_char_in_any_car");
    lua_pushcfunction(L, opcode_get_player_car);        lua_setglobal(L, "get_player_car");
    lua_pushcfunction(L, opcode_get_player_coords);     lua_setglobal(L, "get_player_coords");
    lua_pushcfunction(L, opcode_set_player_coords);     lua_setglobal(L, "set_player_coords");
    lua_pushcfunction(L, opcode_set_car_collision);     lua_setglobal(L, "set_car_collision");
    lua_pushcfunction(L, opcode_set_ped_collision);     lua_setglobal(L, "set_ped_collision");
    lua_pushcfunction(L, opcode_set_player_can_enter_exit_vehicles);
    lua_setglobal(L, "set_player_can_enter_exit_vehicles");
    lua_pushcfunction(L, opcode_does_car_exist);        lua_setglobal(L, "does_car_exist");
    lua_pushcfunction(L, opcode_create_ped);            lua_setglobal(L, "create_ped");
    lua_pushcfunction(L, opcode_delete_ped);            lua_setglobal(L, "delete_ped");
    lua_pushcfunction(L, opcode_set_ped_coords);        lua_setglobal(L, "set_ped_coords");
    lua_pushcfunction(L, opcode_get_ped_coords);        lua_setglobal(L, "get_ped_coords");
    lua_pushcfunction(L, opcode_is_ped_in_car);         lua_setglobal(L, "is_ped_in_car");
    lua_pushcfunction(L, opcode_create_car);            lua_setglobal(L, "create_car");
    lua_pushcfunction(L, opcode_delete_car);            lua_setglobal(L, "delete_car");
    lua_pushcfunction(L, opcode_set_car_coords);        lua_setglobal(L, "set_car_coords");
    lua_pushcfunction(L, opcode_get_car_coords);        lua_setglobal(L, "get_car_coords");
    lua_pushcfunction(L, opcode_set_car_health);        lua_setglobal(L, "set_car_health");
    lua_pushcfunction(L, opcode_get_car_health);        lua_setglobal(L, "get_car_health");
    lua_pushcfunction(L, opcode_get_game_timer);        lua_setglobal(L, "get_game_timer");
    lua_pushcfunction(L, opcode_load_model);            lua_setglobal(L, "load_model");
    lua_pushcfunction(L, opcode_has_model_loaded);      lua_setglobal(L, "has_model_loaded");
    lua_pushcfunction(L, opcode_load_all_models_now);   lua_setglobal(L, "load_all_models_now");
    lua_pushcfunction(L, opcode_mark_model_as_no_longer_needed);
    lua_setglobal(L, "mark_model_as_no_longer_needed");
    lua_pushcfunction(L, opcode_request_model);         lua_setglobal(L, "request_model");
    lua_pushcfunction(L, opcode_read_memory);           lua_setglobal(L, "read_memory");
    lua_pushcfunction(L, opcode_write_memory);          lua_setglobal(L, "write_memory");
    lua_pushcfunction(L, opcode_call_function);         lua_setglobal(L, "call_function");
    lua_pushcfunction(L, opcode_print_string);          lua_setglobal(L, "print_string");
    lua_pushcfunction(L, opcode_print_big);             lua_setglobal(L, "print_big");

    // Camera
    lua_pushcfunction(L, opcode_set_camera_mode);       lua_setglobal(L, "set_camera_mode");
    lua_pushcfunction(L, opcode_get_camera_mode);       lua_setglobal(L, "get_camera_mode");
    lua_pushcfunction(L, opcode_set_camera_position);     lua_setglobal(L, "set_camera_position");
    lua_pushcfunction(L, opcode_set_camera_target);     lua_setglobal(L, "set_camera_target");
    lua_pushcfunction(L, opcode_set_camera_fov);        lua_setglobal(L, "set_camera_fov");
    lua_pushcfunction(L, opcode_get_camera_fov);        lua_setglobal(L, "get_camera_fov");
    lua_pushcfunction(L, opcode_set_camera_in_front_of_player);
    lua_setglobal(L, "set_camera_in_front_of_player");
    lua_pushcfunction(L, opcode_restore_camera);        lua_setglobal(L, "restore_camera");

    // Input
    lua_pushcfunction(L, opcode_is_mouse_button_pressed); lua_setglobal(L, "is_mouse_button_pressed");

    // Weapon
    lua_pushcfunction(L, opcode_get_player_weapon);     lua_setglobal(L, "get_player_weapon");
    lua_pushcfunction(L, opcode_is_weapon_sniper);      lua_setglobal(L, "is_weapon_sniper");
    lua_pushcfunction(L, opcode_is_player_aiming);      lua_setglobal(L, "is_player_aiming");
    lua_pushcfunction(L, opcode_set_player_aiming);     lua_setglobal(L, "set_player_aiming");

    // Char
    lua_pushcfunction(L, opcode_get_ped_heading);       lua_setglobal(L, "get_ped_heading");
    lua_pushcfunction(L, opcode_set_ped_heading);       lua_setglobal(L, "set_ped_heading");
    lua_pushcfunction(L, opcode_is_ped_on_foot);        lua_setglobal(L, "is_ped_on_foot");
    lua_pushcfunction(L, opcode_get_all_chars);         lua_setglobal(L, "get_all_chars");
    lua_pushcfunction(L, opcode_does_char_exist);       lua_setglobal(L, "does_char_exist");
    lua_pushcfunction(L, opcode_get_char_health);       lua_setglobal(L, "get_char_health");
    lua_pushcfunction(L, opcode_remove_all_char_weapons); lua_setglobal(L, "remove_all_char_weapons");
    lua_pushcfunction(L, opcode_remove_all_char_weapons); lua_setglobal(L, "removeAllCharWeapons");

    // Vehicle
    lua_pushcfunction(L, opcode_get_car_speed);         lua_setglobal(L, "get_car_speed");
    lua_pushcfunction(L, opcode_get_car_heading);       lua_setglobal(L, "get_car_heading");
    lua_pushcfunction(L, opcode_is_car_in_air);         lua_setglobal(L, "is_car_in_air");
}

int ml::opcode_wait(lua_State* L) {
    int ms = luaL_checkint(L, 1);
    auto* script = GetScript(L);
    if (script) script->Wait(ms);
    return lua_yield(L, 0);
}

int ml::opcode_test_cheat(lua_State* L) {
    const char* cheat = luaL_checkstring(L, 1);
    if (!cheat || !*cheat) {
        lua_pushboolean(L, false);
        return 1;
    }

    size_t len = strlen(cheat);
    bool found = false;

    if (g_CheatBuffer.length() >= len) {
        const char* tail = g_CheatBuffer.c_str() + g_CheatBuffer.length() - len;
        if (_stricmp(tail, cheat) == 0) {
            found = true;
            g_CheatBuffer.clear();
        }
    }

    lua_pushboolean(L, found);
    return 1;
}

int ml::opcode_is_player_playing(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    bool playing = false;
    if (playerId == 0 && CWorld::Players[0].m_pPed) {
        playing = true;
    }
    lua_pushboolean(L, playing);
    return 1;
}

int ml::opcode_get_player_coords(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    CPed* ped = CWorld::Players[playerId].m_pPed;
    if (ped) {
        lua_pushnumber(L, ped->GetPosition().x);
        lua_pushnumber(L, ped->GetPosition().y);
        lua_pushnumber(L, ped->GetPosition().z);
        return 3;
    }
    lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0);
    return 3;
}

int ml::opcode_set_player_coords(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    CPed* ped = CWorld::Players[playerId].m_pPed;
    if (ped) {
        ped->SetPosition(x, y, z);
    }
    return 0;
}

int ml::opcode_create_ped(lua_State* L) {
    int pedType = luaL_checkint(L, 1);
    int modelId = luaL_checkint(L, 2);
    float x = (float)luaL_checknumber(L, 3);
    float y = (float)luaL_checknumber(L, 4);
    float z = (float)luaL_checknumber(L, 5);

    CStreaming::RequestModel(modelId, 0);
    CStreaming::LoadAllRequestedModels(false);

    void* mem = ::operator new(sizeof(CCivilianPed));
    CCivilianPed* civ = reinterpret_cast<CCivilianPed*>(mem);

    plugin::CallMethodDynGlobal(
        GLOBAL_ADDRESS_BY_VERSION(0x4BFF30, 0x4C0020, 0x4BFFB0),
        civ,
        static_cast<ePedType>(pedType),
        static_cast<unsigned int>(modelId)
    );

    CPed* ped = reinterpret_cast<CPed*>(civ);
    ped->Teleport(CVector(x, y, z));
    CWorld::Add(reinterpret_cast<CEntity*>(ped));

    lua_pushinteger(L, reinterpret_cast<lua_Integer>(ped));
    return 1;
}

int ml::opcode_delete_ped(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    if (ped) {
        CWorld::Remove(ped);
        delete ped;
    }
    return 0;
}

int ml::opcode_set_ped_coords(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    if (ped) ped->Teleport(CVector(x, y, z));
    return 0;
}

int ml::opcode_get_ped_coords(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    if (ped) {
        lua_pushnumber(L, ped->GetPosition().x);
        lua_pushnumber(L, ped->GetPosition().y);
        lua_pushnumber(L, ped->GetPosition().z);
    }
    else {
        lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0);
    }
    return 3;
}

int ml::opcode_is_ped_in_car(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    lua_pushboolean(L, ped && ped->m_pVehicle != nullptr);
    return 1;
}

int ml::opcode_create_car(lua_State* L) {
    int modelId = luaL_checkint(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);

    CStreaming::RequestModel(modelId, 0);
    CStreaming::LoadAllRequestedModels(false);

    void* mem = ::operator new(sizeof(CAutomobile));
    CAutomobile* car = reinterpret_cast<CAutomobile*>(mem);

    plugin::CallMethodDynGlobal(
        GLOBAL_ADDRESS_BY_VERSION(0x52C6B0, 0x52C8F0, 0x52C880),
        car,
        modelId,
        static_cast<unsigned char>(1)
    );

    car->SetPosition(x, y, z);
    car->m_nStatus = STATUS_PHYSICS;
    CWorld::Add(car);

    lua_pushinteger(L, reinterpret_cast<lua_Integer>(car));
    return 1;
}

int ml::opcode_delete_car(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    if (car) {
        CWorld::Remove(car);
        delete car;
    }
    return 0;
}

int ml::opcode_set_car_coords(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    if (car) car->SetPosition(x, y, z);
    return 0;
}

int ml::opcode_get_car_coords(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    if (car) {
        lua_pushnumber(L, car->GetPosition().x);
        lua_pushnumber(L, car->GetPosition().y);
        lua_pushnumber(L, car->GetPosition().z);
    }
    else {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
    }
    return 3;
}

int ml::opcode_set_car_health(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    int health = luaL_checkint(L, 2);
    if (car) car->m_fHealth = static_cast<float>(health);
    return 0;
}

int ml::opcode_get_car_health(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    lua_pushinteger(L, car ? static_cast<int>(car->m_fHealth) : 0);
    return 1;
}


int ml::opcode_get_game_timer(lua_State* L) {
    lua_pushinteger(L, (lua_Integer)CTimer::m_snTimeInMilliseconds);
    return 1;
}

int ml::opcode_load_model(lua_State* L) {
    int modelId = luaL_checkint(L, 1);
    CStreaming::RequestModel(modelId, 0);
    return 0;
}


int ml::opcode_has_model_loaded(lua_State* L) {
    int modelId = luaL_checkint(L, 1);
    lua_pushboolean(L, CStreaming::ms_aInfoForModel[modelId].m_nLoadState != 0);
    return 1;
}

int ml::opcode_load_all_models_now(lua_State* L) {
    CStreaming::LoadAllRequestedModels(false);
    return 0;
}

int ml::opcode_mark_model_as_no_longer_needed(lua_State* L) {
    int modelId = luaL_checkint(L, 1);
    CStreaming::SetModelIsDeletable(modelId);
    return 0;
}

int ml::opcode_request_model(lua_State* L) {
    int modelId = luaL_checkint(L, 1);
    CStreaming::RequestModel(modelId, 0);
    return 0;
}

int ml::opcode_read_memory(lua_State* L) {
    uintptr_t addr = (uintptr_t)lua_tointeger(L, 1);
    int size = luaL_checkint(L, 2);

    switch (size) {
    case 1: lua_pushinteger(L, *(uint8_t*)addr); break;
    case 2: lua_pushinteger(L, *(uint16_t*)addr); break;
    case 4: lua_pushinteger(L, *(uint32_t*)addr); break;
    default: lua_pushinteger(L, 0); break;
    }
    return 1;
}

int ml::opcode_write_memory(lua_State* L) {
    uintptr_t addr = (uintptr_t)lua_tointeger(L, 1);
    int size = luaL_checkint(L, 2);
    uint32_t value = (uint32_t)lua_tointeger(L, 3);

    DWORD oldProtect;
    VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    switch (size) {
    case 1: *(uint8_t*)addr = (uint8_t)value; break;
    case 2: *(uint16_t*)addr = (uint16_t)value; break;
    case 4: *(uint32_t*)addr = value; break;
    }
    VirtualProtect((void*)addr, size, oldProtect, &oldProtect);
    return 0;
}

int ml::opcode_print_string(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    CFont::InitPerFrame();
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetBackgroundOff();
    CFont::SetCentreOff();
    CFont::SetJustifyOn();
    CFont::SetScale(0.4f, 0.8f);
    CFont::SetColor(CRGBA(255, 255, 255, 255));
    CFont::SetFontStyle(FONT_BANK);
    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
    CFont::SetWrapx(640.0f);
    CFont::PrintString(10.0f, 10.0f, text);
    return 0;
}

int ml::opcode_print_big(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    CFont::InitPerFrame();
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetBackgroundOff();
    CFont::SetCentreOff();
    CFont::SetJustifyOn();
    CFont::SetScale(0.8f, 1.6f);
    CFont::SetColor(CRGBA(255, 200, 0, 255));
    CFont::SetFontStyle(FONT_HEADING);
    CFont::SetDropShadowPosition(2);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
    CFont::SetWrapx(640.0f);
    CFont::PrintString(10.0f, 80.0f, text);
    return 0;
}

int ml::opcode_call_function(lua_State* L) {
    uintptr_t addr = (uintptr_t)lua_tointeger(L, 1);
    Log::System("call_function called - implement calling convention handler");
    lua_pushinteger(L, 0);
    return 1;
}

int ml::opcode_is_key_pressed(lua_State* L) {
    int vk = luaL_checkint(L, 1);
    lua_pushboolean(L, (GetAsyncKeyState(vk) & 0x8000) != 0);
    return 1;
}

int ml::opcode_get_frame_delta_time(lua_State* L) {
    lua_pushnumber(L, CTimer::ms_fTimeStep);
    return 1;
}

int ml::opcode_get_camera_position(lua_State* L) {
    CVector pos = TheCamera.GetPosition();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

int ml::opcode_get_camera_target(lua_State* L) {
    CMatrix* matrix = TheCamera.GetCameraMatrix();
    CVector pos = matrix->pos;
    CVector forward = matrix->at;
    lua_pushnumber(L, pos.x + forward.x);
    lua_pushnumber(L, pos.y + forward.y);
    lua_pushnumber(L, pos.z + forward.z);
    return 3;
}

int ml::opcode_get_player_ped(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    CPed* ped = CWorld::Players[playerId].m_pPed;
    lua_pushinteger(L, reinterpret_cast<lua_Integer>(ped));
    return 1;
}

int ml::opcode_is_char_in_any_car(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    lua_pushboolean(L, ped && ped->m_pVehicle != nullptr);
    return 1;
}

int ml::opcode_get_player_car(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    CPed* ped = CWorld::Players[playerId].m_pPed;
    if (ped && ped->m_pVehicle) {
        lua_pushinteger(L, reinterpret_cast<lua_Integer>(ped->m_pVehicle));
    }
    else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

int ml::opcode_set_ped_collision(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    bool enable = lua_toboolean(L, 2) != 0;
    if (ped) {
        ped->bUsesCollision = enable;
    }
    return 0;
}

int ml::opcode_set_car_collision(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    bool enable = lua_toboolean(L, 2) != 0;
    if (car) {
        car->bUsesCollision = enable;
    }
    return 0;
}

int ml::opcode_does_car_exist(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    lua_pushboolean(L, car != nullptr);
    return 1;
}

int ml::opcode_set_player_can_enter_exit_vehicles(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    bool enable = lua_toboolean(L, 2) != 0;
    CPed* ped = CWorld::Players[playerId].m_pPed;
    if (ped) {
        ped->bCanPedEnterSeekedCar = enable;
    }
    return 0;
}

// ==================== CAMERA ====================

int ml::opcode_set_camera_mode(lua_State* L) {
    short mode = (short)luaL_checkint(L, 1);
    TheCamera.TakeControlNoEntity(CVector(0, 0, 0), 1, 1);
    return 0;
}

int ml::opcode_get_camera_mode(lua_State* L) {
    lua_pushinteger(L, TheCamera.m_nWhoIsInControlOfTheCamera);
    return 1;
}

int ml::opcode_set_camera_position(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);

    TheCamera.TakeControlNoEntity(CVector(x, y, z), 1, 1);
    return 0;
}

int ml::opcode_set_camera_target(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);

    TheCamera.m_vecFixedModeVector = CVector(x, y, z);
    TheCamera.m_bLookingAtVector = true;
    return 0;
}

int ml::opcode_restore_camera(lua_State* L) {
    TheCamera.RestoreWithJumpCut();
    TheCamera.m_bLookingAtVector = false;
    return 0;
}

int ml::opcode_set_camera_fov(lua_State* L) {
    float fov = (float)luaL_checknumber(L, 1);
    TheCamera.m_fFOVWhenInterPol = fov;
    return 0;
}

int ml::opcode_get_camera_fov(lua_State* L) {
    lua_pushnumber(L, TheCamera.m_fFOVWhenInterPol);
    return 1;
}

int ml::opcode_set_camera_in_front_of_player(lua_State* L) {
    float dist = (float)luaL_checknumber(L, 1);
    float height = (float)luaL_checknumber(L, 2);

    CPed* ped = CWorld::Players[0].m_pPed;
    if (!ped) {
        lua_pushboolean(L, false);
        return 1;
    }

    CVector pos = ped->GetPosition();
    float heading = ped->GetHeading();

    float camX = pos.x - sin(heading) * dist;
    float camY = pos.y + cos(heading) * dist;
    float camZ = pos.z + height;

    TheCamera.m_vecFixedModeSource = CVector(camX, camY, camZ);
    TheCamera.m_vecFixedModeVector = pos;
    TheCamera.m_bLookingAtVector = true;

    lua_pushboolean(L, true);
    return 1;
}

// ==================== INPUT ====================

int ml::opcode_is_mouse_button_pressed(lua_State* L) {
    int button = luaL_checkint(L, 1);
    bool pressed = false;
    if (button == 2) {
        pressed = (GetAsyncKeyState(0x02) & 0x8000) != 0;
    }
    else if (button == 1) {
        pressed = (GetAsyncKeyState(0x01) & 0x8000) != 0;
    }
    lua_pushboolean(L, pressed);
    return 1;
}

// ==================== WEAPON ====================

int ml::opcode_get_player_weapon(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    CPed* ped = CWorld::Players[playerId].m_pPed;
    if (ped) {
        CWeapon* weapon = ped->GetWeapon();
        if (weapon) {
            lua_pushinteger(L, static_cast<int>(weapon->m_eWeaponType));
        }
        else {
            lua_pushinteger(L, 0);
        }
    }
    else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

int ml::opcode_is_weapon_sniper(lua_State* L) {
    int weaponId = luaL_checkint(L, 1);
    bool isSniper = (weaponId == 28 || weaponId == 29);
    lua_pushboolean(L, isSniper);
    return 1;
}

int ml::opcode_is_player_aiming(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    CPed* ped = CWorld::Players[playerId].m_pPed;
    bool aiming = false;
    if (ped) {
        aiming = ped->bIsAimingGun;
    }
    lua_pushboolean(L, aiming);
    return 1;
}

int ml::opcode_set_player_aiming(lua_State* L) {
    int playerId = luaL_checkint(L, 1);
    bool aim = lua_toboolean(L, 2) != 0;
    CPed* ped = CWorld::Players[playerId].m_pPed;
    if (ped) {
        ped->bIsAimingGun = aim;
        ped->bIsPointingGunAt = aim;
    }
    return 0;
}

// ==================== CHAR ====================

int ml::opcode_get_ped_heading(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    if (ped) {
        lua_pushnumber(L, ped->GetHeading());
    }
    else {
        lua_pushnumber(L, 0);
    }
    return 1;
}

int ml::opcode_set_ped_heading(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    float heading = (float)luaL_checknumber(L, 2);
    if (ped) {
        ped->SetHeading(heading);
    }
    return 0;
}

int ml::opcode_is_ped_on_foot(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    lua_pushboolean(L, ped && ped->m_pVehicle == nullptr);
    return 1;
}

int ml::opcode_get_all_chars(lua_State* L) {
    lua_newtable(L); // Create the array table to return
    int index = 1;

    // Using the pointer directly to avoid undefined type name errors
    if (CPools::ms_pPedPool) {
        int poolSize = CPools::ms_pPedPool->m_nSize;
        for (int i = 0; i < poolSize; i++) {
            CPed* ped = CPools::ms_pPedPool->GetAt(i);
            if (ped) {
                lua_pushinteger(L, reinterpret_cast<lua_Integer>(ped));
                lua_rawseti(L, -2, index++);
            }
        }
    }
    return 1; // Return the table
}

int ml::opcode_does_char_exist(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    bool exists = false;
    if (CPools::ms_pPedPool && ped) {
        exists = CPools::ms_pPedPool->IsObjectValid(ped);
    }
    lua_pushboolean(L, exists);
    return 1;
}

int ml::opcode_get_char_health(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    if (ped) {
        // Cast to int to match traditional CLEO/Moonloader logic (health is a float under the hood)
        lua_pushinteger(L, static_cast<int>(ped->m_fHealth));
    }
    else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

int ml::opcode_remove_all_char_weapons(lua_State* L) {
    CPed* ped = reinterpret_cast<CPed*>(lua_tointeger(L, 1));
    if (ped) {
        ped->ClearWeapons();
    }
    return 0;
}

// ==================== VEHICLE ====================

int ml::opcode_get_car_speed(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    if (car) {
        CVector vel = car->m_vecMoveSpeed;
        float speed = sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z) * 50.0f;
        lua_pushnumber(L, speed);
    }
    else {
        lua_pushnumber(L, 0);
    }
    return 1;
}

int ml::opcode_get_car_heading(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    if (car) {
        lua_pushnumber(L, car->GetHeading());
    }
    else {
        lua_pushnumber(L, 0);
    }
    return 1;
}

int ml::opcode_is_car_in_air(lua_State* L) {
    CVehicle* car = reinterpret_cast<CVehicle*>(lua_tointeger(L, 1));
    bool inAir = false;
    if (car) {
        float height = car->GetHeightAboveRoad();
        inAir = (height > 0.5f);
    }
    lua_pushboolean(L, inAir);
    return 1;
}
