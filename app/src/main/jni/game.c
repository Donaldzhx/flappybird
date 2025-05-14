#include "android_native_app_glue.h"
#include <GLES2/gl2.h>
#include "game.h"
#include "utils.h"
#include "texture.h"
#include "audio.h"
#include "init.h"
#include "mouse.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SIZE_SPACE_PIPE 3.3f
#define SPACE_BETWEEN_PIPES 5

typedef enum {
    IDLE,
    FADE_IN,
    FADE_OUT,
    READY_GAME,
    GO_GAME,
    STOP_GAME,
    FADE_OUT_GAMEOVER,
    FALL_BIRD,
    FADE_IN_PANEL
} GameState;

typedef struct {
    float x, y;
    float velocity;
    float angle;
    float width;
    float height;
    GLuint curTexture;
    int frame;
    uint64_t preTime;
} Bird;

typedef struct {
    float x, y;
    float width, height;
    float offset;
} Pipe;

// buttons
GLuint t_pause;
GLuint t_ok;
GLuint t_menu;
GLuint t_resume;
GLuint t_score;
GLuint t_share;
GLuint t_start;

// sprites
GLuint t_0;
GLuint t_1;
GLuint t_2;
GLuint t_3; 
GLuint t_4;
GLuint t_5;
GLuint t_6;
GLuint t_7;
GLuint t_8;
GLuint t_9;

GLuint t_0_small;
GLuint t_1_small;
GLuint t_2_small;
GLuint t_3_small;
GLuint t_4_small;
GLuint t_5_small;
GLuint t_6_small;
GLuint t_7_small;
GLuint t_8_small;
GLuint t_9_small;

GLuint t_bacground_day;
GLuint t_base;
GLuint t_bronze_metal;
GLuint t_gameover;
GLuint t_gold_medal;
GLuint t_logo;
GLuint t_message;
GLuint t_new;
GLuint t_panel;
GLuint t_pipe_green;
GLuint t_platinum_medal;
GLuint t_silver_medal;
GLuint t_sparkle_sheet;
GLuint t_yellowbird_downflap;
GLuint t_yellowbird_midflap;
GLuint t_yellowbird_upflap;

GameState currentState = IDLE;
Bird bird = {0};
Pipe pipes[2] = {0};

// idle scene
float logoY;
float birdY;
float logoVelocity;
float birdVelocity;
uint64_t idleTime = 0;
GLuint curidlebirdTexture = yellowbird_midflap;
GLuint idlebirdTexture[3] = {yellowbird_downflap, yellowbird_midflap, yellowbird_upflap};
int idleFrame = getTickCount();

int offsetBase = 0;
int alpha = 0;
bool fadeOut = false;
int fadeOutAlpha = 255;
float panelY = 0;
GLuint medalTexture = 0;

int gameSpeed = WindowSizeX / 135;
int score = 0;
int bestScore = 0;
bool newBestScore = false;

float ScaleX(float percent)
{
    return (percent / 100.0f) * WindowSizeX;
}

float ScaleY(float percent)
{
    return (percent / 100.0f) * WindowSizeY;
}

int Random(int min, int max)
{
    return min + rand() % (max - min + 1);
}

bool InitGame()
{
    t_start = LoadTexture("buttons/start.png");
    t_ok = LoadTexture("buttons/ok.png");
    
    t_0 = LoadTexture("numbers/0.png");
    t_1 = LoadTexture("numbers/1.png");
    t_2 = LoadTexture("numbers/2.png");
    t_3 = LoadTexture("numbers/3.png");
    t_4 = LoadTexture("numbers/4.png");
    t_5 = LoadTexture("numbers/5.png");
    t_6 = LoadTexture("numbers/6.png");
    t_7 = LoadTexture("numbers/7.png");
    t_8 = LoadTexture("numbers/8.png");
    t_9 = LoadTexture("numbers/9.png");

    t_0_small = LoadTexture("numbers/0_small.png");
    t_1_small = LoadTexture("numbers/1_small.png");
    t_2_small = LoadTexture("numbers/2_small.png");
    t_3_small = LoadTexture("numbers/3_small.png");
    t_4_small = LoadTexture("numbers/4_small.png");
    t_5_small = LoadTexture("numbers/5_small.png");
    t_6_small = LoadTexture("numbers/6_small.png");
    t_7_small = LoadTexture("numbers/7_small.png");
    t_8_small = LoadTexture("numbers/8_small.png");
    t_9_small = LoadTexture("numbers/9_small.png");

    t_bacground_day = LoadTexture("backgrounds/background_day.png");
    t_base = LoadTexture("backgrounds/base.png");
    t_bronze_metal = LoadTexture("backgrounds/bronze_metal.png");
    t_silver_medal = LoadTexture("backgrounds/silver_medal.png");
    t_gold_medal = LoadTexture("backgrounds/gold_medal.png");
    t_platinum_medal = LoadTexture("backgrounds/platinum_medal.png");
    t_gameover = LoadTexture("backgrounds/gameover.png");
    t_logo = LoadTexture("backgrounds/logo.png");
    t_message = LoadTexture("backgrounds/message.png");
    t_new = LoadTexture("backgrounds/new.png");
    t_panel = LoadTexture("backgrounds/panel.png"); 
    t_pipe_green = LoadTexture("backgrounds/pipe_green.png");
    t_sparkle_sheet = LoadTexture("backgrounds/sparkle_sheet.png");
    t_yellowbird_downflap = LoadTexture("backgrounds/yellowbird_downflap.png");
    t_yellowbird_midflap = LoadTexture("backgrounds/yellowbird_midflap.png");
    t_yellowbird_upflap = LoadTexture("backgrounds/yellowbird_upflap.png");

    bird.x = ScaleX(18.52f);
    bird.y = ScaleY(20.0f);
    bird.velocity = 0.0f;
    bird.angle = 0.0f;
    bird.width = ScaleX(11.11f);
    bird.height = ScaleY(4.17f);
    bird.curTexture = t_yellowbird_midflap;
    bird.frame = 0;
    bird.preTime = 0;

    
    pipes[0].x = ScaleX(100.0f);
    pipes[0].y = ScaleY(37.5f);
    pipes[0].width = ScaleX(15.0f);
    pipes[0].height = ScaleY(37.5f);
    pipes[0].offset = Random(ScaleY(-SPACE_BETWEEN_PIPES), ScaleY(SPACE_BETWEEN_PIPES));

    pipes[1].x = ScaleX(100.0f) + ScaleX(60.0f);
    pipes[1].y = ScaleY(37.5f);
    pipes[1].width = ScaleX(15.0f);
    pipes[1].height = ScaleY(37.5f);
    pipes[1].offset = Random(ScaleY(-SPACE_BETWEEN_PIPES), ScaleY(SPACE_BETWEEN_PIPES));

    logoY = ScaleY(20.83f);
    birdY = ScaleY(20.83f);
    logoVelocity = 1.1f;
    birdVelocity = 1.1f;

    panelY = ScaleY(100.0f);

    // load best score
    char filepath[256];
    snprintf(filePath, sizeof(filePath), "%s/save.txt", g_app->activity->internalDataPath);

    FILE* file = fopen(filepath, "r");
    if (file)
    {
        int loadbestscore = 0;
        fscanf(file, "%d", &loadbestscore);
        fclose(file);
        bestScore = loadbestscore
    }

    return true;
}

float MoveTowards(float current, float target, float maxDelta)
{
    if (fabs(target - current) <= maxDelta)
    {
        return target;
    }
    return current + (target > current ? maxDelta : -maxDelta);
}

void AnimatedBird()
{
    uint64_t curTime = getTickCount();
    if (curTime - bird.preTime > 100)
    {
        bird.preTime = curTime;
        bird.frame = (bird.frame + 1) % 3;
        switch(bird.frame)
        {
            case 0: bird.curTexture = t_yellowbird_downflap; break;
            case 1: bird.curTexture = t_yellowbird_midflap; break;
            case 2: bird.curTexture = t_yellowbird_upflap; break;
        }
    }
}

void ApplyGravity()
{
    bird.velocity += 0.65f;
    bird.y = bird.y + bird.velocity;

    float targetAngle = bird.velocity > 0 ? 90.0f : -30.0f;
    bird.angle = MoveTorwards(bird.angle, targetAngle, 2.0f);

    if (bird.angle > 90.0f) bird.angle = 90.0f;
}

void Jump()
{
    bird.velocity = -13.5f;
    bird.angle = -30.0f;
}

bool CheckCollision()
{
    // detect collison with pipes
    for (int i = 0; i < 2; i++)
    {
        // upper pipe
        float topPipeX = pipes[i].x;
        float topPipeY = pipes[i].y + pipes[i].offset - (bird.height * SIZE_SPACE_PIPE);
        float topPipeWidth = pipes[i].w;
        float topPipeHeight = -(pipes[i].h + pipes[i].offset - (bird.height * SIZE_SPACE_PIPE));

        // lower pipe
        float bottomPipeX = pipes[i].x;
        float bottomPipeY = pipes[i].y + pipes[i].offset;
        float bottomPipeWidth = pipes[i].w;
        float bottomPipeHeght = pipes[i].h - pipes[i].offset

        // checking collsion for upper pipe
        if (bird.x < topPipeX + topPipeWidth &&
            bird.x + bird.width > topPipeX &&
            bird.y < topPipeY &&
            bird.y + bird.height > topPipeY + topPipeHeight)
        {
            return true;
        }

        // checking collision for lower pipe
        if (bird.x < bottomPipeX + bottomPipeWidth &&
            bird.x + bird.width > bottomPipeX &&
            bird.y < bottomPipeY + bottomPipeHeight &&
            bird.y + bird.height > bottomPipeY)
        {
            return true;
        }

        // checking space beween pipes;
        float gapStartY = topPipeY + topPipeHeight;
        float gapEndY = bottomPipeY;
        if (bird.x < bottomPipeX + bottomPipeWidth &&
            bird.x + bird.width > bottomPipeX &&
            bird.y + bird.height > gapEndY &&
            bird.y < gapEndY)
        {
            // collison not detect for bird
            return true;
        }

    }
    // ground collison
    float baseHeight = ScaleY(75.0f);
    if (bird.y + bird.width > baseHeight)
    {
        return true;
    }

    // sky collision
    if (bird.y <= 0)
    {
        return true;
    }

    return false;
}

void RenderBird()
{
    RenderTexturePro(bird.currentTexture, bird.x, bird.y, bird.width, bird.height, bird.angle);
}

void RenderPipe()
{
    for (int i = 0; i < 2; i++)
    {
        RenderTexture(t_pipe_green, pipes[i].x, pipes[i].y + pipes[i].offset - (bird.height * SIZE_SPACE_PIPE), pipes[i].w, -(pipes[i].h + pipes[i].offset - (bird.height * SIZE_SPACE_PIPE)));

        RenderTexture(t_pipe_green, pipes[i].x, pipes[i].y + pipes[i].offset, pipes[i].w, pipes[i].h - pipes[i].offset);
    }
}

void UpdateBirdTextureForLogo()
{
    uint64_t curTime = getTickCount();
    if (curTime - idleTime > 100)
    {
        idleTime = curTime;
        idleFrame = (idleFrame + 1) % 3;
        curidlebirdTexture = birdTexturesForLogo[idleFrame];
    }
}

void RenderScore(int score, float x, float y, float digitWidth, float digitHeigt)
{
    char scoreStr[10];
    sprintf(scoreStr, "%d", score);
    int len = strlen(scoreStr);

    for (int i = 0; i < len; i++)
    {
        int digit = scoreStr[i] - '0';
        GLuint texture = 0;

        switch(digit)
        {
            case 0: texture = t_0; break;
            case 1: texture = t_1; break;
            case 2: texture = t_2; break;
            case 3: texture = t_3; break;
            case 4: texture = t_4; break;
            case 5: texture = t_5; break;
            case 6: texture = t_6; break;
            case 7: texture = t_7; break;
            case 8: texture = t_8; break;
            case 9: texture = t_9; break;
        }

        RenderTexture(texture, x + i * digitWidth, y, digitWidth, digitHeight);
    }
}

void RenderSmallScore(int score, float x, float y, float digitWidth, float digitHeight)
{
    char scoreStr[10];
    sprintf(scoreStr, "%d", score);
    int len = strlen(scoreStr);

    for (int i = 0; i < len, i++)
    {
        int digit = scoreStr[i] - '0';
        GLuint testure = 0;

        switch(digit)
        {
            case 0: texture = t_0_small; break;
            case 1: texture = t_1_small; break;
            case 2: texture = t_2_small; break;
            case 3: texture = t_3_small; break;
            case 4: texture = t_4_small; break;
            case 5: texture = t_5_small; break;
            case 6: texture = t_6_small; break;
            case 7: texture = t_7_small; break;
            case 8: texture = t_8_small; break;
            case 9: texture = t_9_small; break;
        }

        RenderTexture(texture, x + i * digitWidth, y, digitWidth, digitHeight)
    }
}

void Render()
{
    // background
    RenderTexture(t_background_day, 0, 0 , windowSizeX, windowSizeY);

    // cycle base texture
    if (currentState != STOP_GAME && currentState != FADE_OUT_GAMEOVER && currentState != FALL_BIRD && currentState != FADE_IN_PANEL)
    {
        offsetBase = offsetBase - gameSpeed;
    }

    RenderTexture(t_base, offsetbase, ScaleY(75.0f), ScaleX(100.0f), ScaleY(25.0f));


    if (offsetBase < 0)
    {
        RenderTexture(t_base, ScaleX(100.0f) + offsetBase, ScaleY(75.0f), ScaleX(100.0f), ScaleY(25.0f));
    }

    if (offsetbase <= -ScaleX(100.0f))
    {
        offsetBase = 0;
    }

    logoY += logovelocity;
    birdY += birdVelocity;

    if (logoY > ScaleY(20.83f) + 25 || logoY < ScaleY(20.83f) - 25)
    {
        logoVelocity = -logoVelocity;
    }

    if (birdY > ScaleY(20.83f) + 25 || birdY < ScaleY(20.83f) - 25)
    {
        birdVelocity = -birdVelocity;
    }

    if (currentState == IDLE || currentState == FADE_IN)
    {
        RenderTexture(t_logo, ScaleX(15.0f), logoY, ScaleX(55.56f), ScaleY(5.21f));

        UpdateBirdTextureForLogo();

        RenderTexture(curTextureAnimBirdForLogo, ScaleX(75), birdY, bird.width, bird.height);

        if (ButtonBump(t_start, ScaleX(10.0f), ScaleY(65.0f), ScaleX(35.0f), ScaleY(6.0f)))
        {
            // later
            // PlayAudio("audio/click_sound.ogg");
            currentState = FADE_IN;
        }

        // button SCORE
        if (ButtonBump(t_score, ScaleX(55.0f), ScaleY(65.0f), ScaleX(35.0f), ScaleY(6.0f)))
        {
            // later
            // PlayAudio("audio/click_sound.ogg");
            currentState = FADE_IN;
        }
    }
    else if (currentState == FADE_OUT || currentState == READY_GAME) //Ready?
    {
        RenderBird();

        RenderTexture(t_message, ScaleX(10.0f), ScaleY(9.0f), ScaleX(80.0f), ScaleY(50.0f));
        if (Button(0, 0, ScaleX(100.0f), ScaleY(100.0f)))
        {
            currentState = GO_GAME;
        }
    }
    else if (currentState == GO_GAME)
    {
        ApplyGravity();
        AnimateBird();

        for (int i = 0; i < 2; i++)
        {
            pipes[i].x = pipes[i].x - gameSpeed;
            if (pipes[i].x < -ScaleX(15.0f))
            {
                pipes[i].x = ScaleX(115.0f);
                pipes[i].offset = Random(ScaleY(-SPACE_BETWEEN_PIPES), ScaleY(SPACE_BETWEEN_PIPES));
            }

            if (bird.x + (bird.width / 2) >= pipes[i].x + pipes[i].w && bird.x + (bird.width / 2) <= pipes[i].x + pipes[i].w + gameSpeed)
            {
                score++;
                PlayAudio("audio/point.mp3");
            }
        }

        if (CheckCollision())
        {
            currentState = STOP_GAME;
            PlayAudio("audio/hit.mp3");
        }

        if (IsClick(0, 0, ScaleX(100.0f),ScaleY(100.0f)))
        {
            Jump();
            PlayAudio("audio/wing.mp3");
        }

        RenderPipes();
        RenderBird();

        if (score > 0)
        {
            RenderScore(score, ScaleX(45.0f), ScaleY(7.0f), ScaleX(8.0f), ScaleY(5.0f));
        }
    }   
    else if (currentState == STOP_GAME)
    {
        if (score > bestScore)
        {
            bestScore = score;
            newBestScore = true;

            char filePath[256];
            snprintf(filePath, sizeof(filePath), "%s/save.txt", g_app->activity->internalDataPath);

            FILE* file = fopen(filePath, "w");
            if (file)
            {
                fprintf(file, "%d", bestScore);
                fclose(file);
            }
        }
        currentState = FADE_OUT_GAMEOVER;
    }
    else if (currentState == FADE_OUT_GAMEOVER)
    {
        fadeOutAlpha = fadeOutAlpha - 5;
        if (fadeOutAlpha <= 0)
        {
            fadeOutAlpha = 0;
            currentState = FALL_BIRD;
            PlayAudio("audio/die.mp3");
        }

        RenderPipes();
        RenderBird();

        uint32_t color = 0x00FFFFFF | (fadeOutAlpha << 24);
        CreateBox(color, 0, 0, windowSizeX, windowSizeY);
    }
    else if (currentState == FALL_BIRD)
    {
        ApplyGravity();
        RenderPipes();
        RenderBird();

        if (bird.y + bird.height >= ScaleY(75.0f) - bird.height)
        {
            bird.y = ScaleY(75.0f) - bird.height;
            currentState = FADE_IN_PANEL;
        }
    }
    else if (currentState == FADE_IN_PANEL)
    {
        RenderPipes();
        RenderBIrds();

        panelY = MoveTowards(PanelY, ScaleY(30.0f), 20.0f);
        RenderTexture(t_panel, ScaleX(15.0f), panelY, ScaleX(70.0f), ScaleY(17.5f));

        // Render default score
        RenderSmallScore()
    }

}



