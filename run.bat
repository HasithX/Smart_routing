@echo off
title Smart City Transit Engine
echo ========================================================
echo   Smart City Transit Engine - Windows Terminal Launcher
echo ========================================================
echo.

where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] g++ compiler was not found in your PATH!
    echo.
    echo To run this project on Windows, you need a C++ compiler:
    echo 1. Install MinGW-w64 (via MSYS2, w64devkit, or Code::Blocks)
    echo 2. Or use WSL (Windows Subsystem for Linux) and run 'make run'
    echo.
    pause
    exit /b 1
)

echo [*] Compiling C++17 source files...
g++ -std=c++17 -O3 -Iinclude src\Location.cpp src\Edge.cpp src\Graph.cpp src\Dijkstra.cpp src\RoutePlanner.cpp src\DemandSimulator.cpp src\Profiler.cpp src\Visualizer.cpp src\main.cpp -o smart_transit.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

echo [*] Compilation successful!
echo [*] Launching interactive transit terminal...
echo.
smart_transit.exe
echo.
pause
