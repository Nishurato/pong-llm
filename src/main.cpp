#include "Application.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    Pong::Application application;
    return application.Run(instance, showCommand);
}
