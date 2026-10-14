#include "Game.h"

#include <algorithm>
#include <cmath>

namespace Pong
{
    namespace
    {
        constexpr int kPaddleSpeed = 5;
        constexpr double kInitialBallSpeed = 6.0;
        constexpr double kBallSpeedIncrease = 1.05;
        constexpr double kMaximumBallSpeed = 12.0;
        constexpr double kMaximumBounceAngle = 1.0471975511965976;
        constexpr unsigned int kWinningScore = 10;
    }

    Game::Game()
    {
        ResetMatch(1);
    }

    void Game::Update(const InputState& input)
    {
        if (winner_ != 0)
        {
            return;
        }

        MovePaddle(paddles_[0], input.leftPaddleDirection);
        MovePaddle(paddles_[1], input.rightPaddleDirection);

        if (UpdateBall())
        {
            if (scores_[0] >= kWinningScore)
            {
                winner_ = 1;
            }
            else if (scores_[1] >= kWinningScore)
            {
                winner_ = 2;
            }
        }
    }

    void Game::Restart()
    {
        ResetMatch(winner_ == 1 ? 1 : -1);
    }

    unsigned int Game::ScoreForPlayer(unsigned int player) const
    {
        return scores_[player - 1];
    }

    unsigned int Game::Winner() const
    {
        return winner_;
    }

    const Paddle& Game::PaddleForPlayer(unsigned int player) const
    {
        return paddles_[player - 1];
    }

    const Ball& Game::GetBall() const
    {
        return ball_;
    }

    void Game::ResetMatch(int horizontalDirection)
    {
        scores_ = {};
        paddles_[0] = { 30, (kPlayableTop + kPlayableBottom - kPaddleHeight) / 2 };
        paddles_[1] = {
            kCanvasWidth - 30 - kPaddleWidth,
            (kPlayableTop + kPlayableBottom - kPaddleHeight) / 2
        };
        winner_ = 0;
        ResetBall(horizontalDirection);
    }

    void Game::ResetBall(int horizontalDirection)
    {
        ball_ = {
            (kCanvasWidth - kBallSize) / 2.0,
            (kCanvasHeight - kBallSize) / 2.0,
            horizontalDirection * kInitialBallSpeed,
            0.0
        };
        ballSpeed_ = kInitialBallSpeed;
    }

    RECT Game::GetPaddleRect(const Paddle& paddle)
    {
        return { paddle.x, paddle.y, paddle.x + kPaddleWidth, paddle.y + kPaddleHeight };
    }

    RECT Game::GetBallRect() const
    {
        return {
            static_cast<LONG>(ball_.x),
            static_cast<LONG>(ball_.y),
            static_cast<LONG>(ball_.x + kBallSize),
            static_cast<LONG>(ball_.y + kBallSize)
        };
    }

    bool Game::Overlaps(const RECT& first, const RECT& second)
    {
        return first.left < second.right
            && first.right > second.left
            && first.top < second.bottom
            && first.bottom > second.top;
    }

    void Game::MovePaddle(Paddle& paddle, int direction)
    {
        paddle.y = std::clamp(
            paddle.y + direction * kPaddleSpeed,
            kPlayableTop,
            kPlayableBottom - kPaddleHeight);
    }

    void Game::BounceOffPaddle(const Paddle& paddle, int horizontalDirection)
    {
        const double ballCenter = ball_.y + kBallSize / 2.0;
        const double paddleCenter = paddle.y + kPaddleHeight / 2.0;
        const double hitPosition = std::clamp(
            (ballCenter - paddleCenter) / (kPaddleHeight / 2.0),
            -1.0,
            1.0);
        const double angle = hitPosition * kMaximumBounceAngle;

        ballSpeed_ = std::min(ballSpeed_ * kBallSpeedIncrease, kMaximumBallSpeed);
        ball_.velocityX = horizontalDirection * ballSpeed_ * std::cos(angle);
        ball_.velocityY = ballSpeed_ * std::sin(angle);
    }

    bool Game::UpdateBall()
    {
        ball_.x += ball_.velocityX;
        ball_.y += ball_.velocityY;

        if (ball_.y < kPlayableTop)
        {
            ball_.y = kPlayableTop;
            ball_.velocityY = std::abs(ball_.velocityY);
        }
        else if (ball_.y + kBallSize > kPlayableBottom)
        {
            ball_.y = kPlayableBottom - kBallSize;
            ball_.velocityY = -std::abs(ball_.velocityY);
        }

        const RECT ballRect = GetBallRect();
        const RECT leftPaddleRect = GetPaddleRect(paddles_[0]);
        const RECT rightPaddleRect = GetPaddleRect(paddles_[1]);

        if (ball_.velocityX < 0 && Overlaps(ballRect, leftPaddleRect))
        {
            ball_.x = leftPaddleRect.right;
            BounceOffPaddle(paddles_[0], 1);
        }
        else if (ball_.velocityX > 0 && Overlaps(ballRect, rightPaddleRect))
        {
            ball_.x = rightPaddleRect.left - kBallSize;
            BounceOffPaddle(paddles_[1], -1);
        }

        if (ball_.x + kBallSize < 0)
        {
            ++scores_[1];
            ResetBall(1);
            return true;
        }
        if (ball_.x > kCanvasWidth)
        {
            ++scores_[0];
            ResetBall(-1);
            return true;
        }
        return false;
    }
}
