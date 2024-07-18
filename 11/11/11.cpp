#include <stdio.h>
#include <raylib.h>

// Oyun durumlarını tanımlamak için enum kullanalım
typedef enum { START_SCREEN, PLAYING, GAME_OVER } GameState;

// Sıvı yapısını tanımlayalım
typedef struct {
    Color color;
} Liquid;

// Yüksek skoru tutmak için bir yapı tanımlayalım
typedef struct {
    double bestTime;
    bool isSet;
} HighScore;

void DrawCurrentTime(double elapsedTime, int screenWidth, int screenHeight, bool isGameOver);
void DrawBestTime(HighScore highScore, int screenWidth, int screenHeight);
void DrawHelpScreen(int screenWidth, int screenHeight);
bool CheckGameOver(Liquid tubeLiquids[4][3]);
void DrawGlassTube(int x, int y, int width, int height, Color color);

int main() {
    const int screenWidth = 1000;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Three Color Liquids Game");

    // Ses cihazını başlatalım
    InitAudioDevice();

    // Müzik dosyasını yükleyelim
    Music backgroundMusic = LoadMusicStream("Enter your music file");
    PlayMusicStream(backgroundMusic);

    // Arka plan resmi yükleyelim
    Image backgroundImage = LoadImage("Enter backgrpund file");
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);

    // Oyun durumunu takip etmek için bir değişken tanımlayalım
    GameState gameState = START_SCREEN;

    // Oyun başladığında başlangıç ekranındaki düğmeye tıklandığını takip etmek için bir değişken
    bool startButtonClicked = false;

    // Başlangıç ekranı rengi
    Color startScreenColor = { 242, 241, 239, 255 };

    // Press Enter to Start metninin arka plan rengi
    Color startTextBackground = { 128, 0, 128, 255 }; // Mor rengi

    Color tubeColors[4] = { Fade(GRAY, 0.5f), Fade(GRAY, 0.5f), Fade(GRAY, 0.5f), Fade(GRAY, 0.5f) };

    Liquid tubeLiquids[4][3] = {
        {{Fade(RED, 0.9f)}, {Fade(GREEN, 0.9f)}, {Fade(BLUE, 0.9f)}},
        {{Fade(GREEN, 0.9f)}, {Fade(BLUE, 0.9f)}, {Fade(RED, 0.9f)}},
        {{Fade(BLUE, 0.9f)}, {Fade(RED, 0.9f)}, {Fade(GREEN, 0.9f)}},
        {{BLANK}, {BLANK}, {BLANK}}
    };

    int selectedTube = -1;
    Color selectedColor = BLANK;
    bool isLiquidGrabbed = false;

    double startTime = 0.0; // Oyun başlama zamanını tutar
    double elapsedTime = 0.0; // Geçen süreyi tutar

    HighScore highScore = { 0.0, false };

    bool musicPlaying = true;
    bool showHelp = false;

    while (!WindowShouldClose()) {
        UpdateMusicStream(backgroundMusic);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawTexture(backgroundTexture, 0, 0, WHITE);

        // Oyun durumuna göre ekranda ne gösterileceğini belirleyelim
        switch (gameState) {
        case START_SCREEN:
            // Başlangıç ekranını çizelim
            DrawRectangle(0, 0, screenWidth, screenHeight, startScreenColor);

            // Oyun ismini gösterelim
            DrawText("Three Color Liquids Game", screenWidth / 2 - MeasureText("Three Color Liquids Game", 40) / 2, screenHeight / 2 - 100, 40, BLACK);

            // Press Enter to Start metnini çizelim
            DrawRectangle(screenWidth / 2 - MeasureText("Press Enter to Start", 20) / 2 - 10, screenHeight / 2 + 50 - 10, MeasureText("Press Enter to Start", 20) + 20, 30 + 20, startTextBackground);
            DrawText("Press Enter to Start", screenWidth / 2 - MeasureText("Press Enter to Start", 20) / 2, screenHeight / 2 + 50, 20, BLACK);

            // Help metnini çizelim
            DrawRectangle(10, screenHeight - 60, 200, 50, GRAY);
            DrawText("Press H for Help", 20, screenHeight - 40, 20, BLACK);

            // Ses açma/kapama metnini çizelim
            DrawRectangle(screenWidth - 210, screenHeight - 60, 200, 50, GRAY);
            DrawText("Press M to Mute", screenWidth - 200, screenHeight - 40, 20, BLACK);

            // Başlatma düğmesine tıklanıp tıklanmadığını kontrol edelim
            if (IsKeyPressed(KEY_ENTER)) {
                startButtonClicked = true;
            }

            // Ses açma/kapama tuşuna basıldığını kontrol edelim
            if (IsKeyPressed(KEY_M)) {
                musicPlaying = !musicPlaying;
                if (musicPlaying) {
                    PlayMusicStream(backgroundMusic);
                }
                else {
                    PauseMusicStream(backgroundMusic);
                }
            }

            // Yardım tuşuna basıldığını kontrol edelim
            if (IsKeyPressed(KEY_H)) {
                showHelp = !showHelp;
            }
            // Başlatma düğmesine tıklanırsa oyun durumunu PLAYING olarak değiştirelim
            if (startButtonClicked) {
                gameState = PLAYING;
                startTime = GetTime(); // Oyun başlama zamanını kaydedelim
                startButtonClicked = false; // Değişkeni sıfırlayalım
            }

            if (showHelp) {
                DrawHelpScreen(screenWidth, screenHeight);
            }

            break;

        case PLAYING:
            for (int i = 0; i < 4; i++) {
                DrawGlassTube(200 + i * 150, 250, 100, 300, tubeColors[i]);

                for (int j = 0; j < 3; j++) {
                    DrawRectangle(200 + i * 150, 450 - j * 100, 100, 100, tubeLiquids[i][j].color);
                }
            }

            if (isLiquidGrabbed && selectedTube != -1) {
                // Sıvıyı fare ile birlikte hareket ettir
                DrawRectangle(GetMouseX() - 50, GetMouseY() - 50, 100, 100, selectedColor);
            }

            // Sıvının tüpler arasında taşınması
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int tubeIndex = (GetMouseX() - 200) / 150;

                if (tubeIndex >= 0 && tubeIndex < 4) {
                    if (!isLiquidGrabbed) {
                        // Sıvıyı tutma işlemi
                        for (int i = 2; i >= 0; i--) {
                            if (tubeLiquids[tubeIndex][i].color.a != 0) {
                                selectedTube = tubeIndex;
                                selectedColor = tubeLiquids[tubeIndex][i].color;
                                tubeLiquids[tubeIndex][i].color = BLANK;
                                isLiquidGrabbed = true;
                                break;
                            }
                        }
                    }
                    else {
                        // Sıvıyı bırakma işlemi
                        for (int i = 0; i < 3; i++) {
                            if (tubeLiquids[tubeIndex][i].color.a == 0) {
                                tubeLiquids[tubeIndex][i].color = selectedColor;
                                isLiquidGrabbed = false;
                                selectedTube = -1;
                                selectedColor = BLANK;
                                break;
                            }
                        }
                    }
                }
            }

            // Zamanı hesapla ve çiz
            elapsedTime = GetTime() - startTime;
            DrawCurrentTime(elapsedTime, screenWidth, screenHeight, false);

            // Ses açma/kapama tuşuna basıldığını kontrol edelim
            if (IsKeyPressed(KEY_M)) {
                musicPlaying = !musicPlaying;
                if (musicPlaying) {
                    PlayMusicStream(backgroundMusic);
                }
                else {
                    PauseMusicStream(backgroundMusic);
                }
            }

            // Yardım tuşuna basıldığını kontrol edelim
            if (IsKeyPressed(KEY_H)) {
                showHelp = !showHelp;
            }

            // Yardım ekranını çizelim
            if (showHelp) {
                DrawHelpScreen(screenWidth, screenHeight);
            }

            // Oyun bitiş kontrolü
            if (CheckGameOver(tubeLiquids)) {
                double elapsedTime = GetTime() - startTime;
                if (!highScore.isSet || elapsedTime < highScore.bestTime) {
                    highScore.bestTime = elapsedTime;
                    highScore.isSet = true;
                }
                gameState = GAME_OVER;
            }

            break;

        case GAME_OVER:
            // Oyun bitiş ekranını çizelim
            DrawRectangle(0, 0, screenWidth, screenHeight, startScreenColor);
            DrawText("You Win!", screenWidth / 2 - MeasureText("You Win!", 40) / 2, screenHeight / 2 - 100, 40, BLACK);
            DrawText("Press Enter to Restart", screenWidth / 2 - MeasureText("Press Enter to Restart", 20) / 2, screenHeight / 2 + 50, 20, BLACK);

            // Geçen süreyi ve en iyi süreyi göster
            DrawCurrentTime(elapsedTime, screenWidth, screenHeight, true);
            DrawBestTime(highScore, screenWidth, screenHeight);

            // Yeniden başlatma tuşuna basıldığını kontrol edelim
            if (IsKeyPressed(KEY_ENTER)) {
                // Tüpleri başlangıç durumuna getirelim
                Liquid newTubeLiquids[4][3] = {
                    {{Fade(RED, 0.9f)}, {Fade(GREEN, 0.9f)}, {Fade(BLUE, 0.9f)}},
                    {{Fade(GREEN, 0.9f)}, {Fade(BLUE, 0.9f)}, {Fade(RED, 0.9f)}},
                    {{Fade(BLUE, 0.9f)}, {Fade(RED, 0.9f)}, {Fade(GREEN, 0.9f)}},
                    {{BLANK}, {BLANK}, {BLANK}}
                };
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 3; j++) {
                        tubeLiquids[i][j] = newTubeLiquids[i][j];
                    }
                }

                // Oyun durumunu ve zamanlayıcıyı sıfırlayalım
                gameState = PLAYING;
                startTime = GetTime();
            }

            break;
        }

        EndDrawing();
    }
    UnloadTexture(backgroundTexture);
    UnloadMusicStream(backgroundMusic);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void DrawCurrentTime(double elapsedTime, int screenWidth, int screenHeight, bool isGameOver) {
    if (isGameOver) {
        char timeText[50];
        snprintf(timeText, 50, "Time: %.2f seconds", elapsedTime);
        DrawText(timeText, screenWidth - MeasureText(timeText, 20) - 20, 20, 20, BLACK);
    }
    else {
        char timeText[50];
        snprintf(timeText, 50, "Time: %.2f seconds", elapsedTime);
        DrawText(timeText, screenWidth - MeasureText(timeText, 20) - 20, 20, 20, BLACK);
    }
}

void DrawBestTime(HighScore highScore, int screenWidth, int screenHeight) {
    if (highScore.isSet) {
        char bestTimeText[50];
        snprintf(bestTimeText, 50, "Best Time: %.2f seconds", highScore.bestTime);
        DrawText(bestTimeText, screenWidth - MeasureText(bestTimeText, 20) - 20, 50, 20, BLACK);
    }
}

void DrawHelpScreen(int screenWidth, int screenHeight) {
    DrawRectangle(screenWidth / 4, screenHeight / 4, screenWidth / 2, screenHeight / 2, Fade(WHITE, 0.9f));
    DrawText("Help", screenWidth / 2 - MeasureText("Help", 30) / 2, screenHeight / 4 + 20, 30, BLACK);
    DrawText("1. Click on a tube to pick up the top liquid.", screenWidth / 4 + 20, screenHeight / 4 + 70, 20, BLACK);
    DrawText("2. Click on another tube to pour the liquid.", screenWidth / 4 + 20, screenHeight / 4 + 100, 20, BLACK);
    DrawText("3. Arrange the liquids so that each tube", screenWidth / 4 + 20, screenHeight / 4 + 130, 20, BLACK);
    DrawText("   contains only one color.", screenWidth / 4 + 20, screenHeight / 4 + 160, 20, BLACK);
    DrawText("Press H to close this help screen.", screenWidth / 4 + 20, screenHeight / 4 + 200, 20, BLACK);
}

bool CheckGameOver(Liquid tubeLiquids[4][3]) {
    for (int i = 0; i < 4; i++) {
        Color firstColor = tubeLiquids[i][0].color;
        for (int j = 1; j < 3; j++) {
            if (tubeLiquids[i][j].color.r != firstColor.r ||
                tubeLiquids[i][j].color.g != firstColor.g ||
                tubeLiquids[i][j].color.b != firstColor.b ||
                tubeLiquids[i][j].color.a != firstColor.a) {
                return false;
            }
        }
    }
    return true;
}

void DrawGlassTube(int x, int y, int width, int height, Color color) {
    DrawRectangle(x, y, width, height, Fade(color, 0.5f));
    DrawRectangleLines(x, y, width, height, Fade(BLACK, 0.5f));
    DrawRectangle(x + 5, y + 5, width - 10, height - 10, Fade(WHITE, 0.5f));
}
