#include "Renderer.h"

#include <algorithm>
#include <string>

namespace Pong
{
    namespace
    {
        constexpr int kCenterStripeWidth = 4;
        constexpr int kCenterStripeHeight = 12;
        constexpr int kCenterStripeGap = 10;
        constexpr float kScoreFontHeight = 36.0f;
        constexpr float kVictoryHintFontHeight = 20.0f;
    }

    bool Renderer::IsReady() const
    {
        return renderTarget_ != nullptr;
    }

    HRESULT Renderer::Initialize()
    {
        HRESULT result = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            d2dFactory_.GetAddressOf());
        if (FAILED(result))
        {
            return result;
        }

        result = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(writeFactory_.GetAddressOf()));
        if (FAILED(result))
        {
            return result;
        }

        result = CreateTextFormat(
            DWRITE_FONT_WEIGHT_BOLD,
            kScoreFontHeight,
            &scoreTextFormat_);
        if (FAILED(result))
        {
            return result;
        }
        result = SetCenteredAlignment(scoreTextFormat_.Get());
        if (FAILED(result))
        {
            return result;
        }

        result = CreateTextFormat(
            DWRITE_FONT_WEIGHT_NORMAL,
            kVictoryHintFontHeight,
            &victoryHintTextFormat_);
        if (FAILED(result))
        {
            return result;
        }
        return SetCenteredAlignment(victoryHintTextFormat_.Get());
    }

    HRESULT Renderer::CreateRenderTarget(HWND window)
    {
        RECT clientRect{};
        if (!GetClientRect(window, &clientRect))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        const D2D1_SIZE_U targetSize = D2D1::SizeU(
            static_cast<UINT32>(clientRect.right - clientRect.left),
            static_cast<UINT32>(clientRect.bottom - clientRect.top));
        HRESULT result = d2dFactory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(window, targetSize),
            &renderTarget_);
        if (FAILED(result))
        {
            return result;
        }

        result = renderTarget_->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &whiteBrush_);
        if (FAILED(result))
        {
            ReleaseRenderTarget();
            return result;
        }
        return S_OK;
    }

    HRESULT Renderer::Resize(UINT width, UINT height)
    {
        return renderTarget_->Resize(D2D1::SizeU(width, height));
    }

    HRESULT Renderer::RecreateRenderTarget(HWND window)
    {
        ReleaseRenderTarget();
        return CreateRenderTarget(window);
    }

    HRESULT Renderer::Draw(const Game& game)
    {
        renderTarget_->BeginDraw();
        renderTarget_->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        DrawWalls();
        DrawCenterLine();
        DrawPaddles(game);
        DrawBall(game.GetBall());
        DrawScore(game);
        DrawVictory(game.Winner());
        return renderTarget_->EndDraw();
    }

    HRESULT Renderer::CreateTextFormat(
        DWRITE_FONT_WEIGHT weight,
        float fontSize,
        IDWriteTextFormat** textFormat)
    {
        return writeFactory_->CreateTextFormat(
            L"Arial",
            nullptr,
            weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"",
            textFormat);
    }

    HRESULT Renderer::SetCenteredAlignment(IDWriteTextFormat* textFormat)
    {
        HRESULT result = textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        if (FAILED(result))
        {
            return result;
        }
        return textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    void Renderer::ReleaseRenderTarget()
    {
        whiteBrush_.Reset();
        renderTarget_.Reset();
    }

    void Renderer::FillRectangle(float left, float top, float right, float bottom)
    {
        renderTarget_->FillRectangle(D2D1::RectF(left, top, right, bottom), whiteBrush_.Get());
    }

    void Renderer::DrawWalls()
    {
        FillRectangle(0.0f, 0.0f, static_cast<float>(kCanvasWidth), static_cast<float>(kPlayableTop));
        FillRectangle(
            0.0f,
            static_cast<float>(kPlayableBottom),
            static_cast<float>(kCanvasWidth),
            static_cast<float>(kCanvasHeight));
    }

    void Renderer::DrawCenterLine()
    {
        const float left = (kCanvasWidth - kCenterStripeWidth) / 2.0f;
        const float right = (kCanvasWidth + kCenterStripeWidth) / 2.0f;
        for (int y = kPlayableTop; y < kPlayableBottom; y += kCenterStripeHeight + kCenterStripeGap)
        {
            FillRectangle(
                left,
                static_cast<float>(y),
                right,
                static_cast<float>(std::min(y + kCenterStripeHeight, kPlayableBottom)));
        }
    }

    void Renderer::DrawPaddles(const Game& game)
    {
        for (unsigned int player = 1; player <= 2; ++player)
        {
            const Paddle& paddle = game.PaddleForPlayer(player);
            FillRectangle(
                static_cast<float>(paddle.x),
                static_cast<float>(paddle.y),
                static_cast<float>(paddle.x + kPaddleWidth),
                static_cast<float>(paddle.y + kPaddleHeight));
        }
    }

    void Renderer::DrawBall(const Ball& ball)
    {
        FillRectangle(
            static_cast<float>(ball.x),
            static_cast<float>(ball.y),
            static_cast<float>(ball.x + kBallSize),
            static_cast<float>(ball.y + kBallSize));
    }

    void Renderer::DrawText(
        const wchar_t* text,
        UINT32 length,
        IDWriteTextFormat* format,
        const D2D1_RECT_F& bounds)
    {
        renderTarget_->DrawTextW(text, length, format, bounds, whiteBrush_.Get());
    }

    void Renderer::DrawScore(const Game& game)
    {
        const std::wstring leftScore = std::to_wstring(game.ScoreForPlayer(1));
        const std::wstring rightScore = std::to_wstring(game.ScoreForPlayer(2));
        DrawText(
            leftScore.c_str(),
            static_cast<UINT32>(leftScore.length()),
            scoreTextFormat_.Get(),
            D2D1::RectF(300.0f, 45.0f, 400.0f, 105.0f));
        DrawText(
            rightScore.c_str(),
            static_cast<UINT32>(rightScore.length()),
            scoreTextFormat_.Get(),
            D2D1::RectF(400.0f, 45.0f, 500.0f, 105.0f));
    }

    void Renderer::DrawVictory(unsigned int winner)
    {
        if (winner == 0)
        {
            return;
        }

        const std::wstring message = L"Player " + std::to_wstring(winner) + L" wins!";
        constexpr wchar_t hint[] = L"Press R to restart or Esc to exit.";
        DrawText(
            message.c_str(),
            static_cast<UINT32>(message.length()),
            scoreTextFormat_.Get(),
            D2D1::RectF(100.0f, 245.0f, 700.0f, 315.0f));
        DrawText(
            hint,
            static_cast<UINT32>(sizeof(hint) / sizeof(hint[0]) - 1),
            victoryHintTextFormat_.Get(),
            D2D1::RectF(100.0f, 315.0f, 700.0f, 365.0f));
    }
}
