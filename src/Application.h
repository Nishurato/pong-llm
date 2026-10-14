#pragma once

#include "Game.h"
#include "Renderer.h"

namespace Pong
{
    class Application
    {
    public:
        int Run(HINSTANCE instance, int showCommand);

    private:
        static void ShowError(const wchar_t* message);
        HRESULT CreateGameWindow(HINSTANCE instance);
        int RunGameLoop(HANDLE frameTimer, int showCommand);
        void Update(const LARGE_INTEGER& currentCounter);
        static bool IsKeyDown(int key);
        static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

        Game game_;
        Renderer renderer_;
        HWND window_ = nullptr;
        LARGE_INTEGER frameFrequency_{};
        LARGE_INTEGER lastFrameCounter_{};
        double frameAccumulator_ = 0;
    };
}
