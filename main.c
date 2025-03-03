#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

/*

raylib library uses DEG always, easier to most newcomers. raymath library always uses RAD,

Since down direction is +y, CW is positive rotation

*/

/* Macro Definitions */

#define BIRD_WIDTH 70
#define BIRD_HEIGHT 50

#define BIRD_COLOR WHITE
#define PIPE_COLOR GREEN

#define BIRD_JUMP_SPEED 500
#define GRAVITY 1700

#define BIRD_JUMP_ROTATION 15

#define PIPE_COUNT_IN_SCREEN 2
#define PIPE_SPEED_BASE 200 /* Default is 200 */
#define PIPE_SPEED_MAX 450
#define PIPE_WIDTH 100

#define PIPE_GAP_MIN (BIRD_HEIGHT * 3)
#define PIPE_GAP_MAX (BIRD_HEIGHT * 5)

/* Struct definitions */

typedef struct Bird
{
    Rectangle rectangle;                /* Rectangle of the bird contaning top left pos and width & height */
    float velocity;                     /* Velocity of the bird in y-axis */
    float rotation;                     /* Rotation of the bird around its center in Degrees */
    Color color;
} Bird;

typedef struct Pipe
{
    Rectangle upperPipeRect;
    Rectangle lowerPipeRect;
    Color color;
    bool hasBeenPassed;                 /* A variable to determine whether the pipe has been passed by the bird to track score */
} Pipe;


/* Global Variable Declarations */

static const int screenWidth = 800;
static const int screenHeight = 800;

static const int pipeOffset = (screenWidth - (PIPE_COUNT_IN_SCREEN - 1) * PIPE_WIDTH) / PIPE_COUNT_IN_SCREEN;


static const int targetFPS = 60;

static float pipeSpeed = PIPE_SPEED_BASE;

static Bird bird;

static int nearestPipeIndex = 0;

static int score = 0;
static int highScore = 0;

static bool isGameRunning = true;


/* Function Declarations */

static void InitGame(Pipe pipeArray[]);             /* Called only once at the beginning to initialize the game */
static void UpdateGame(Pipe pipeArray[]);           /* Called every frame */
static void DrawFrame(Pipe pipeArray[]);            /* Draws the current frame */
static void UnloadGame(void);           /* Unloads game and frees dynamic memory allocations */
static void UpdateDrawFrame(Pipe pipeArray[]);      /* Updates the game and draws the updated/current frame */

static void ResetGame(Pipe pipeArray[]);

static void DrawTopLeftScore(void);
static void DrawPauseScreen(void);

static void InitializeBird(void);
static void UpdateBird(void);
static void ResetBird(void);

static Pipe CreatePipe(Rectangle upperPipeRect, Rectangle lowerPipeRect);
static Pipe CreateRandomPipe(int x);
static void InitializePipes(Pipe pipeArray[]);
static void MovePipe(Pipe *pipe);
static void UpdatePipes(Pipe pipeArray[]);
static void ResetPipes(Pipe pipeArray[]);

static void DrawBird(void);
static void DrawPipes(Pipe pipeArray[]);

static void CheckCollision(Pipe pipeArray[]);

static Vector2 Vector2RotateRel(Vector2 vectorToRotate, Vector2 origin, float angle);

int main(void)
{
    Pipe pipeArray[PIPE_COUNT_IN_SCREEN];

    InitGame(pipeArray);

    while (!WindowShouldClose())
    {
        UpdateDrawFrame(pipeArray);
    }

    UnloadGame();

    return 0;
}

void InitGame(Pipe pipeArray[])
{
    InitWindow(screenWidth, screenHeight, "Flappy Rectangle");
    SetTargetFPS(targetFPS);

    InitializeBird();
    InitializePipes(pipeArray);
}

void UpdateGame(Pipe pipeArray[])
{
    if (isGameRunning)
    {
        UpdateBird();
        UpdatePipes(pipeArray);

        CheckCollision(pipeArray);

        if (pipeSpeed <= PIPE_SPEED_MAX)
            pipeSpeed += 0.03;

        /* printf("%f\n", pipeSpeed); */

    }
    else
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            /* Restart the game */
            ResetGame(pipeArray);
        }
    }
}

void DrawFrame(Pipe pipeArray[])
{
    BeginDrawing();
        
        ClearBackground(BLACK);
        
        DrawBird();
        DrawPipes(pipeArray);

        if (isGameRunning)
        {
            DrawTopLeftScore();
        }
        else
        {
            DrawPauseScreen();
        }

    EndDrawing();
}

void UpdateDrawFrame(Pipe pipeArray[])
{
    UpdateGame(pipeArray);
    DrawFrame(pipeArray);
}

void UnloadGame(void)
{
    CloseWindow();
}

void ResetGame(Pipe pipeArray[])
{
    ResetBird();
    ResetPipes(pipeArray);
    isGameRunning = true;
    score = 0;
    pipeSpeed = PIPE_SPEED_BASE;
}

void DrawTopLeftScore(void)
{
    DrawText(TextFormat("Score: %d", score), 10, 0, 30, WHITE);
    DrawText(TextFormat("High Score: %d", highScore), 10, 40, 20, WHITE);
}

void DrawPauseScreen(void)
{
    int scoreFont = 50;
    int highScoreFont = 40;
    int replayInfoFont = 30;

    static int frameCounterReplayText = 0; /* This variable cannot be auto, must be static */

    frameCounterReplayText++;
    
    int textWidth = MeasureText(TextFormat("Score: %d", score), scoreFont);
    DrawText(TextFormat("Score: %d", score), GetScreenWidth() / 2 - textWidth / 2, 100, scoreFont, WHITE);

    textWidth = MeasureText(TextFormat("High Score: %d", highScore), highScoreFont);
    DrawText(TextFormat("High Score: %d", highScore), GetScreenWidth() / 2 - textWidth / 2, 175, highScoreFont, WHITE);

    if (frameCounterReplayText <= 45) /* Only draw the 'Press Space' text every 45 frame to make a blink animation */
    {
        textWidth = MeasureText("Press 'Space' to Play Again", replayInfoFont);
        DrawText("Press 'Space' to Play Again", GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2, replayInfoFont, WHITE);
    }
    else if (frameCounterReplayText > 90)
    {
        frameCounterReplayText = 0;
    }
}

void InitializeBird(void)
{
    Rectangle birdRectangle = (Rectangle) {
        .x = 200,
        .y = GetScreenHeight() / 2,
        .width = BIRD_WIDTH,
        .height = BIRD_HEIGHT
    };

    bird.rectangle = birdRectangle;
    bird.velocity = 0;
    bird.rotation = 0;
    bird.color = BIRD_COLOR;
}

void UpdateBird(void)
{
    /* Get player input */
    if (IsKeyPressed(KEY_SPACE))
    {
        bird.velocity = -BIRD_JUMP_SPEED;
    }

    /* Apply gravity to the bird */
    bird.velocity += GRAVITY * GetFrameTime();

    if (bird.velocity < 0) bird.rotation = -BIRD_JUMP_ROTATION; /* CCW */
    else if (bird.velocity > 0) bird.rotation = BIRD_JUMP_ROTATION; /* CW */
    else bird.rotation = 0;

    bird.rectangle.y += bird.velocity * GetFrameTime();
}

void ResetBird(void)
{
     Rectangle birdRectangle = (Rectangle) {
        .x = 200,
        .y = GetScreenHeight() / 2,
        .width = BIRD_WIDTH,
        .height = BIRD_HEIGHT
    };

    bird.rectangle = birdRectangle;
    bird.velocity = 0;
    bird.rotation = 0;
}

void ResetPipes(Pipe pipeArray[])
{
    InitializePipes(pipeArray);
}

void DrawBird(void)
{
    /* Rotate around the center of the rectangle */
    
   float birdRotationRAD = bird.rotation * DEG2RAD;

    Vector2 middle = (Vector2) {bird.rectangle.x + bird.rectangle.width / 2, bird.rectangle.y + bird.rectangle.height / 2};
    
    Vector2 topLeft = (Vector2) {bird.rectangle.x, bird.rectangle.y};
    Vector2 topRight = (Vector2) {bird.rectangle.x + bird.rectangle.width, bird.rectangle.y};
    Vector2 bottomLeft = (Vector2) {bird.rectangle.x, bird.rectangle.y + bird.rectangle.height};
    Vector2 bottomRight = (Vector2) {bird.rectangle.x + bird.rectangle.width, bird.rectangle.y + bird.rectangle.height};

    Vector2 rotatedTopLeft = Vector2RotateRel(topLeft, middle, birdRotationRAD);
    Vector2 rotatedTopRight = Vector2RotateRel(topRight, middle, birdRotationRAD);
    Vector2 rotatedBottomLeft = Vector2RotateRel(bottomLeft, middle, birdRotationRAD);
    Vector2 rotatedBottomRight = Vector2RotateRel(bottomRight, middle, birdRotationRAD);

    DrawLineV(rotatedTopLeft, rotatedTopRight, bird.color);
    DrawLineV(rotatedTopLeft, rotatedBottomLeft, bird.color);

    DrawLineV(rotatedBottomRight, rotatedBottomLeft, bird.color);
    DrawLineV(rotatedBottomRight, rotatedTopRight, bird.color);
}   

void DrawPipe(Pipe pipe)
{
    DrawRectangleRec(pipe.upperPipeRect, pipe.color);
    DrawRectangleRec(pipe.lowerPipeRect, pipe.color);
}

void DrawPipes(Pipe pipeArray[])
{
    int i;
    for (i = 0; i < PIPE_COUNT_IN_SCREEN; i++)
    {
        DrawPipe(pipeArray[i]);
    }
}

Pipe CreatePipe(Rectangle upperPipeRect, Rectangle lowerPipeRect)
{
    Pipe newPipe = (Pipe) {
        .upperPipeRect = upperPipeRect,
        .lowerPipeRect = lowerPipeRect,
        .color = PIPE_COLOR,
        .hasBeenPassed = false
    };

    return newPipe;
}

Pipe CreateRandomPipe(int x)
{
    int upperPipeBottomY = GetRandomValue(20, screenHeight - PIPE_GAP_MAX);
    int pipeGap = GetRandomValue(PIPE_GAP_MIN, PIPE_GAP_MAX);
    int lowerPipeUpperY = upperPipeBottomY + pipeGap;

    Rectangle upperPipeRect = (Rectangle) {
        .x = x,
        .y = 0,
        .width = PIPE_WIDTH,
        .height = upperPipeBottomY
    };

    Rectangle lowerPipeRect = (Rectangle) {
        .x = x,
        .y = lowerPipeUpperY,
        .width = PIPE_WIDTH,
        .height = screenHeight - lowerPipeUpperY
    };

    return CreatePipe(upperPipeRect, lowerPipeRect);
}

void InitializePipes(Pipe pipeArray[])
{
    int i;
    for (i = 0; i < PIPE_COUNT_IN_SCREEN; i++)
    {
        pipeArray[i] = CreateRandomPipe(screenWidth + i * (pipeOffset + PIPE_WIDTH));
    }
}

void MovePipe(Pipe *pipe)
{
    (*pipe).lowerPipeRect.x -= pipeSpeed * GetFrameTime();
    (*pipe).upperPipeRect.x -= pipeSpeed * GetFrameTime();
}

void UpdatePipes(Pipe pipeArray[])
{
    int i;
    for (i = 0; i < PIPE_COUNT_IN_SCREEN; i++)
    {
        if (!pipeArray[i].hasBeenPassed && bird.rectangle.x >= pipeArray[i].upperPipeRect.x + 5)
        {
            score++;
            pipeArray[i].hasBeenPassed = true;
        }

        if (pipeArray[i].upperPipeRect.x + PIPE_WIDTH < bird.rectangle.x) /* Collision is not possible anymore */
        {
            if (i == PIPE_COUNT_IN_SCREEN - 1)
                nearestPipeIndex = 0;
            else
                nearestPipeIndex = i + 1;
        }

        /* If the pipe is out of the screen */
        if (pipeArray[i].upperPipeRect.x <= -PIPE_WIDTH)
        {
            pipeArray[i] = CreateRandomPipe(screenWidth); /* It creates a new Pipe object with 'hasBeenPassed' attribute set to false */
        }

        MovePipe(&pipeArray[i]);
    }
}

void CheckCollision(Pipe pipeArray[])
{
    Vector2 middle = (Vector2) {bird.rectangle.x + bird.rectangle.width / 2, bird.rectangle.y + bird.rectangle.height / 2};
    
    Vector2 topRight = (Vector2) {bird.rectangle.x + bird.rectangle.width, bird.rectangle.y};
    Vector2 rotatedTopRight = Vector2RotateRel(topRight, middle, bird.rotation * DEG2RAD);

    Vector2 bottomRight = (Vector2) {bird.rectangle.x + bird.rectangle.width, bird.rectangle.y + bird.rectangle.height};
    Vector2 rotatedBottomRight = Vector2RotateRel(bottomRight, middle, bird.rotation * DEG2RAD);

    /* If the bird is ascending */
    if (bird.rotation < 0)
    {
        /* Collision with the ceiling */
        if (rotatedTopRight.y < 0)
        {
            isGameRunning = false;
            highScore = (score > highScore) ? score : highScore;
            printf("Collision with ceiling\n");
            return;
        }
    }
    /* if the bird is descending */
    else if (bird.rotation > 0)
    {        
        /* Collision with the floor */
        if (rotatedBottomRight.y > screenHeight)
        {
            isGameRunning = false;
            highScore = (score > highScore) ? score : highScore;
            printf("Collision with floor\n");
            return;
        }
    }

    /* Check collision with pipes */

    if (CheckCollisionPointRec(rotatedTopRight, pipeArray[nearestPipeIndex].upperPipeRect)
        || CheckCollisionPointRec(rotatedBottomRight, pipeArray[nearestPipeIndex].lowerPipeRect)
        || CheckCollisionPointRec(rotatedTopRight, pipeArray[nearestPipeIndex].lowerPipeRect)
        || CheckCollisionPointRec(rotatedBottomRight, pipeArray[nearestPipeIndex].upperPipeRect))
    {
            isGameRunning = false;
            highScore = (score > highScore) ? score : highScore;
            printf("Collision with a pipe\n");
            return;
    }
}

Vector2 Vector2RotateRel(Vector2 vectorToRotate, Vector2 origin, float angle)
{
    /* Rotate vector by angle in RAD relative to given origin */
    Vector2 vectorToRotateRelOrigin = Vector2Subtract(vectorToRotate, origin);

    Vector2 rotatedVectorRelOrigin = Vector2Rotate(vectorToRotateRelOrigin, angle);

    return Vector2Add(rotatedVectorRelOrigin, origin);
}