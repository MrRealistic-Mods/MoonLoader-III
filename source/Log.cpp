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

#include "Log.h"
#include "Utils.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <windows.h>

std::ofstream ml::Log::file;
std::mutex ml::Log::mtx;

void ml::Log::Init() {
    if (file.is_open()) return;

    std::string dir = ml::GetMoonLoaderPath();
    std::string path = dir + "\\moonloader.log";

    CreateDirectoryA(dir.c_str(), NULL);
    file.open(path, std::ios::out | std::ios::trunc);
}

void ml::Log::Write(const std::string& level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!file.is_open()) return;

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto epoch_us = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
    long us = static_cast<long>(epoch_us % 1000000);
    if (us < 0) us += 1000000;

    std::tm tm;
    localtime_s(&tm, &time_t_now);

    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%H:%M:%S") << "."
        << std::setw(6) << std::setfill('0') << us << "] "
        << "(" << level << ")\t" << msg;

    file << oss.str() << std::endl;
}

void ml::Log::System(const std::string& msg) {
    Write("system", msg);
}

void ml::Log::Debug(const std::string& msg) {
    Write("debug", msg);
}

void ml::Log::Error(const std::string& msg) {
    Write("error", msg);
}