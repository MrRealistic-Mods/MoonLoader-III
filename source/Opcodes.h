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
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace ml {
    void RegisterOpcodes(lua_State* L);
    void UpdateCheatBuffer();

    int opcode_wait(lua_State* L);
    int opcode_test_cheat(lua_State* L);
    int opcode_is_key_pressed(lua_State* L);
    int opcode_get_frame_delta_time(lua_State* L);
    int opcode_get_camera_position(lua_State* L);
    int opcode_get_camera_target(lua_State* L);
    int opcode_get_player_ped(lua_State* L);
    int opcode_is_player_playing(lua_State* L);
    int opcode_is_char_in_any_car(lua_State* L);
    int opcode_get_player_car(lua_State* L);
    int opcode_get_player_coords(lua_State* L);
    int opcode_set_player_coords(lua_State* L);
    int opcode_set_car_collision(lua_State* L);
    int opcode_set_ped_collision(lua_State* L);
    int opcode_set_player_can_enter_exit_vehicles(lua_State* L);
    int opcode_does_car_exist(lua_State* L);
    int opcode_create_ped(lua_State* L);
    int opcode_delete_ped(lua_State* L);
    int opcode_set_ped_coords(lua_State* L);
    int opcode_get_ped_coords(lua_State* L);
    int opcode_is_ped_in_car(lua_State* L);
    int opcode_create_car(lua_State* L);
    int opcode_delete_car(lua_State* L);
    int opcode_set_car_coords(lua_State* L);
    int opcode_get_car_coords(lua_State* L);
    int opcode_set_car_health(lua_State* L);
    int opcode_get_car_health(lua_State* L);
    int opcode_get_game_timer(lua_State* L);
    int opcode_load_model(lua_State* L);
    int opcode_has_model_loaded(lua_State* L);
    int opcode_load_all_models_now(lua_State* L);
    int opcode_mark_model_as_no_longer_needed(lua_State* L);
    int opcode_request_model(lua_State* L);
    int opcode_read_memory(lua_State* L);
    int opcode_write_memory(lua_State* L);
    int opcode_call_function(lua_State* L);
    int opcode_print_string(lua_State* L);
    int opcode_print_big(lua_State* L);
}