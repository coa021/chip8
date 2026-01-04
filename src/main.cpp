#include <iostream>
#include <raylib.h>

int main()
{
    std::cout << "Hello, World!" << std::endl;


    InitWindow(800, 600, "Test");
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Raylib works!", 300, 280, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
