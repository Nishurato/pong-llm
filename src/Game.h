#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <array>

namespace Pong
{
    inline constexpr int kCanvasWidth = 800;
    inline constexpr int kCanvasHeight = 600;
    inline constexpr int kWallInset = 25;
    inline constexpr int kWallThickness = 10;
    inline constexpr int kPlayableTop = kWallInset + kWallThickness;
    inline constexpr int kPlayableBottom = kCanvasHeight - kWallInset - kWallThickness;
    inline constexpr int kPaddleWidth = 12;
    inline constexpr int kPaddleHeight = 80;
    inline constexpr int kBallSize = 12;

    struct InputState
    {
        int leftPaddleDirection;
        int rightPaddleDirection;
    };

    struct Paddle
    {
        int x;
        int y;
    };

    struct Ball
    {
        double x;
        double y;
        double velocityX;
        double velocityY;
    };

    class Game
    {
    public:
        Game();

        void Update(const InputState& input);
        void Restart();

        unsigned int ScoreForPlayer(unsigned int player) const;
        unsigned int Winner() const;
        const Paddle& PaddleForPlayer(unsigned int player) const;
        const Ball& GetBall() const;

    private:
        void ResetMatch(int horizontalDirection);
        void ResetBall(int horizontalDirection);
        static RECT GetPaddleRect(const Paddle& paddle);
        RECT GetBallRect() const;
        static bool Overlaps(const RECT& first, const RECT& second);
        static void MovePaddle(Paddle& paddle, int direction);
        void BounceOffPaddle(const Paddle& paddle, int horizontalDirection);
        bool UpdateBall();

        std::array<unsigned int, 2> scores_{};
        std::array<Paddle, 2> paddles_{};
        Ball ball_{};
        double ballSpeed_ = 6.0;
        unsigned int winner_ = 0;
    };
}
