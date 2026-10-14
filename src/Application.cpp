#include "Application.h"

namespace Pong
{
    namespace
    {
        constexpr LONGLONG kFrameInterval100ns = 166667;
        constexpr double kTargetFramesPerSecond = 60.0;
        constexpr wchar_t kWindowClassName[] = L"PongWindowClass";
        constexpr wchar_t kWindowTitle[] = L"Pong";
    }

    int Application::Run(HINSTANCE instance, int showCommand)
    {
        HRESULT result = renderer_.Initialize();
        if (FAILED(result))
        {
            ShowError(L"Could not initialize Direct2D and DirectWrite.");
            return 1;
        }

        result = CreateGameWindow(instance);
        if (FAILED(result))
        {
            ShowError(L"Could not create the game window.");
            return 1;
        }

        result = renderer_.CreateRenderTarget(window_);
        if (FAILED(result))
        {
            ShowError(L"Could not create the Direct2D render target.");
            DestroyWindow(window_);
            return 1;
        }

        if (!QueryPerformanceFrequency(&frameFrequency_)
            || !QueryPerformanceCounter(&lastFrameCounter_))
        {
            ShowError(L"Could not initialize the performance counter.");
            DestroyWindow(window_);
            return 1;
        }

        HANDLE frameTimer = CreateWaitableTimerExW(
            nullptr,
            nullptr,
            CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
            TIMER_ALL_ACCESS);
        if (frameTimer == nullptr)
        {
            ShowError(L"Could not create the game update timer.");
            DestroyWindow(window_);
            return 1;
        }

        const int exitCode = RunGameLoop(frameTimer, showCommand);
        CloseHandle(frameTimer);
        return exitCode;
    }

    void Application::ShowError(const wchar_t* message)
    {
        MessageBoxW(nullptr, message, kWindowTitle, MB_ICONERROR | MB_OK);
    }

    HRESULT Application::CreateGameWindow(HINSTANCE instance)
    {
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.lpszClassName = kWindowClassName;
        if (!RegisterClassW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        RECT windowRect{ 0, 0, kCanvasWidth, kCanvasHeight };
        if (!AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        window_ = CreateWindowExW(
            0,
            kWindowClassName,
            kWindowTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            instance,
            this);
        return window_ == nullptr ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    }

    int Application::RunGameLoop(HANDLE frameTimer, int showCommand)
    {
        LARGE_INTEGER dueTime{};
        dueTime.QuadPart = -kFrameInterval100ns;
        if (!SetWaitableTimer(frameTimer, &dueTime, 0, nullptr, nullptr, FALSE))
        {
            ShowError(L"Could not start the game update timer.");
            DestroyWindow(window_);
            return 1;
        }

        ShowWindow(window_, showCommand);
        while (true)
        {
            MSG message{};
            if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
            {
                if (message.message == WM_QUIT)
                {
                    return static_cast<int>(message.wParam);
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
                continue;
            }

            const DWORD waitResult = MsgWaitForMultipleObjectsEx(
                1,
                &frameTimer,
                INFINITE,
                QS_ALLINPUT,
                MWMO_INPUTAVAILABLE);
            if (waitResult == WAIT_OBJECT_0)
            {
                LARGE_INTEGER currentCounter{};
                if (!QueryPerformanceCounter(&currentCounter))
                {
                    ShowError(L"Could not read the performance counter.");
                    DestroyWindow(window_);
                    return 1;
                }

                Update(currentCounter);
                if (!SetWaitableTimer(frameTimer, &dueTime, 0, nullptr, nullptr, FALSE))
                {
                    ShowError(L"Could not restart the game update timer.");
                    DestroyWindow(window_);
                    return 1;
                }
            }
            else if (waitResult != WAIT_OBJECT_0 + 1)
            {
                ShowError(L"Could not wait for the next game update.");
                DestroyWindow(window_);
                return 1;
            }
        }
    }

    void Application::Update(const LARGE_INTEGER& currentCounter)
    {
        if (game_.Winner() != 0 || GetForegroundWindow() != window_)
        {
            lastFrameCounter_ = currentCounter;
            frameAccumulator_ = 0;
            return;
        }

        const LONGLONG elapsedTicks = currentCounter.QuadPart - lastFrameCounter_.QuadPart;
        lastFrameCounter_ = currentCounter;
        frameAccumulator_ += elapsedTicks * kTargetFramesPerSecond;
        if (frameAccumulator_ < frameFrequency_.QuadPart)
        {
            return;
        }

        frameAccumulator_ -= frameFrequency_.QuadPart;
        if (frameAccumulator_ >= frameFrequency_.QuadPart)
        {
            frameAccumulator_ = 0;
        }

        game_.Update({
            static_cast<int>(IsKeyDown('S')) - static_cast<int>(IsKeyDown('W')),
            static_cast<int>(IsKeyDown(VK_DOWN)) - static_cast<int>(IsKeyDown(VK_UP))
        });
        InvalidateRect(window_, nullptr, FALSE);
    }

    bool Application::IsKeyDown(int key)
    {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    }

    LRESULT CALLBACK Application::WindowProc(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        Application* application = reinterpret_cast<Application*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE)
        {
            const auto* createInfo = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            application = static_cast<Application*>(createInfo->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
        }
        return application == nullptr
            ? DefWindowProcW(window, message, wParam, lParam)
            : application->HandleMessage(window, message, wParam, lParam);
    }

    LRESULT Application::HandleMessage(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(window);
                return 0;
            }
            if (game_.Winner() != 0 && wParam == 'R')
            {
                game_.Restart();
                if (!QueryPerformanceCounter(&lastFrameCounter_))
                {
                    ShowError(L"Could not restart the game timer.");
                    DestroyWindow(window);
                    return 0;
                }
                frameAccumulator_ = 0;
                InvalidateRect(window, nullptr, FALSE);
                return 0;
            }
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint{};
            BeginPaint(window, &paint);
            HRESULT result = renderer_.Draw(game_);
            EndPaint(window, &paint);

            if (result == D2DERR_RECREATE_TARGET)
            {
                result = renderer_.RecreateRenderTarget(window);
                if (SUCCEEDED(result))
                {
                    InvalidateRect(window, nullptr, FALSE);
                }
            }
            if (FAILED(result))
            {
                ShowError(L"Could not render the game frame with Direct2D.");
                DestroyWindow(window);
            }
            return 0;
        }
        case WM_SIZE:
            if (renderer_.IsReady() && LOWORD(lParam) != 0 && HIWORD(lParam) != 0)
            {
                const HRESULT result = renderer_.Resize(LOWORD(lParam), HIWORD(lParam));
                if (result == D2DERR_RECREATE_TARGET)
                {
                    const HRESULT recreateResult = renderer_.RecreateRenderTarget(window);
                    if (FAILED(recreateResult))
                    {
                        ShowError(L"Could not recreate the Direct2D render target.");
                        DestroyWindow(window);
                    }
                }
                else if (FAILED(result))
                {
                    ShowError(L"Could not resize the Direct2D render target.");
                    DestroyWindow(window);
                }
                InvalidateRect(window, nullptr, FALSE);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wParam, lParam);
        }
        return DefWindowProcW(window, message, wParam, lParam);
    }
}
