#pragma once

#include "Game.h"

#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

namespace Pong
{
    class Renderer
    {
    public:
        bool IsReady() const;
        HRESULT Initialize();
        HRESULT CreateRenderTarget(HWND window);
        HRESULT Resize(UINT width, UINT height);
        HRESULT RecreateRenderTarget(HWND window);
        HRESULT Draw(const Game& game);

    private:
        HRESULT CreateTextFormat(
            DWRITE_FONT_WEIGHT weight,
            float fontSize,
            IDWriteTextFormat** textFormat);
        static HRESULT SetCenteredAlignment(IDWriteTextFormat* textFormat);
        void ReleaseRenderTarget();
        void FillRectangle(float left, float top, float right, float bottom);
        void DrawWalls();
        void DrawCenterLine();
        void DrawPaddles(const Game& game);
        void DrawBall(const Ball& ball);
        void DrawText(
            const wchar_t* text,
            UINT32 length,
            IDWriteTextFormat* format,
            const D2D1_RECT_F& bounds);
        void DrawScore(const Game& game);
        void DrawVictory(unsigned int winner);

        Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory_;
        Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> renderTarget_;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteBrush_;
        Microsoft::WRL::ComPtr<IDWriteFactory> writeFactory_;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> scoreTextFormat_;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> victoryHintTextFormat_;
    };
}
