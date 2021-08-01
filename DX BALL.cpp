#include "iGraphics.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#define PI 3.1416
#define SCRH 720
#define SCRW 1200

struct _paddle
{
    double height = 15;
    double width = 150;
    double x = SCRW / 2 - width / 2;
    double y = SCRH / 12;
    double dx = 30;
} paddle;

struct _ball
{
    double x = SCRW / 2;
    double y = SCRH / 8 - 6;
    double r = 8;
    double dx = 10, dy = 10;
    double velocity = 5;
    double theta = 40;
    double offset = paddle.width / 2;
    bool launched = false;
} ball;

struct _block
{
    double x, y, dx = 50, dy = 20;
    double x1 = x + dx;
    double y1 = y + dy;
    double color[3] = { -1,-1,-1 };
    int type = 0;
    int index = -1;
    bool visible;
    bool explode;
} blocks[15][32];

struct _pickup
{
    double x, y, dx = 32;
    int type;
    bool visible = false;
    char* name;
} pickup[5];

struct _bullet
{
    double x, y;
    double dy = 15;
    bool visible = false;
} bullets[20];

struct _PlayerInfo
{
    char name[33] = "";
    int score = 0;
} playerInfo[10];

int life = 3;
int offset = -1;
int startPoint;
int score = 0;
bool thruBrick = false, fireBall = false, paused = false, shoot = false, catchBall = false;
bool GamePages[5] = { true, false, false, false, false };

double sec = 0;
int minute;
char timerStr[10], scoreText[10];

char playerName[33] = { 0 };
int nameIndex = -1;
bool nameInput = false, namePrompt = false;
bool loadPromt = false;
FILE* fp;

int baseColor = 0;
bool chainExplode = false;
int m, n, startIndex;

char* filename[11] = { "sprites\\extraLife.bmp", "sprites\\fastBall.bmp", "sprites\\expandPaddle.bmp", "sprites\\fireBall.bmp", "sprites\\thruBrick.bmp", "sprites\\instaDeath.bmp", "sprites\\shrinkPaddle.bmp", "sprites\\shootPaddle.bmp", "sprites\\slowBall.bmp", "sprites\\smallBall.bmp", "sprites\\catchBall.bmp" };


/////////////////////////////////////////// BALL & PADDLE SECTION START ////////////////////////////////////////

void DrawPaddle()
{
    iSetColor(56, 115, 167);
    iFilledRectangle(paddle.x, paddle.y, paddle.width, paddle.height);
    iFilledCircle(paddle.x, paddle.y + paddle.height / 2, paddle.height / 2);
    iFilledCircle(paddle.x + paddle.width, paddle.y + paddle.height / 2, paddle.height / 2);
    iSetColor(255, 255, 255);
    iFilledRectangle(paddle.x + 10, paddle.y + 5, paddle.width - 20, paddle.height - 10);

    if (shoot || !ball.launched)
    {
        iSetColor(56, 115, 167);
        iFilledRectangle(paddle.x, paddle.y + paddle.height, 3, 10);
        iFilledRectangle(paddle.x + paddle.width, paddle.y + paddle.height, 3, 10);
    }
    if (!ball.launched)
    {
        iSetColor(255, 150, 150);
        iLine(paddle.x, paddle.y + paddle.height + 7, paddle.x + paddle.width, paddle.y + paddle.height + 7);
    }
}

void PaddleCollDetection()
{
    int n;
    if (abs((paddle.x + paddle.width / 2) - ball.x) < ball.r + paddle.width / 2)
    {
        if (abs((paddle.y + paddle.height / 2) - ball.y) < ball.r + paddle.height / 2)
        {
            if (catchBall && ball.launched)
            {
                ball.offset = ball.x - paddle.x;
                ball.launched = false;
            }
            ball.theta = (ball.x - paddle.x) * 150 / paddle.width;
            if (ball.theta < 20)
                ball.theta = 20;
            if (ball.theta > 160)
                ball.theta = 160;
            ball.dx = -ball.velocity * cos(ball.theta * PI / 180);
            ball.dy = ball.velocity * sin(ball.theta * PI / 180);
        }
    }
}

void delayTimer()
{
    if (!catchBall)
    {
        catchBall = true;
        iPauseTimer(1);
    }
}

void Reset()
{
    ball.launched = false;
    ball.offset = paddle.width / 2;
    ball.x = paddle.x + ball.offset;
    ball.y = paddle.y + paddle.height + ball.r - 2;
    ball.velocity = 5;
    ball.theta = 40;
    PaddleCollDetection();
}

void DrawBall()
{
    iSetColor(99, 99, 99);
    iFilledCircle(ball.x, ball.y, ball.r);
    if (fireBall || thruBrick)
    {
        if (fireBall)
            iSetColor(247, 86, 37);
        else if (thruBrick)
            iSetColor(23, 89, 191);
        iCircle(ball.x, ball.y, ball.r * 2 / 3);
        iCircle(ball.x, ball.y, ball.r * 3 / 2);
    }
}

void BallMovement()
{
    ball.velocity += 0.005;
    if (ball.velocity > 10)
        ball.velocity = 10;
    if (ball.launched)
    {
        ball.x += ball.dx;
        ball.y += ball.dy;

        if ((SCRW - ball.x) <= 0 || ball.x <= 0)
            ball.dx *= -1;
        if ((SCRH - ball.y) <= ball.r)
            ball.dy *= -1;
        if (ball.y < -(ball.r * 2))
        {
            life--;
            if (life < 0)
                life = 0;
            if (life)
                Reset();
        }
    }
    if (!ball.launched)
        ball.x = paddle.x + ball.offset;
}

/////////////////////////////////////////// BALL & PADDLE SECTION END ////////////////////////////////////////


/////////////////////////////////////////// POWERUP SECTION ////////////////////////////////////////

void ManagePowerUps(int i)
{
    if (i == 0)
    {
        if (life < 5)
            life++;
    }
    else if (i == 1)
    {
        ball.velocity *= 1.5;
        ball.dx *= 1.5;
        ball.dx *= 1.5;
    }
    else if (i == 2)
        paddle.width = 200;
    else if (i == 3)
    {
        fireBall = true;
        thruBrick = false;
    }
    else if (i == 4)
    {
        fireBall = false;
        thruBrick = true;
    }
    else if (i == 5)
        life = 0;
    else if (i == 6)
        paddle.width = 80;
    else if (i == 7)
        shoot = true;
    else if (i == 8)
    {
        ball.velocity /= 1.5;
        ball.dx /= 1.5;
        ball.dy /= 1.5;
    }
    else if (i == 9)
        ball.r = 5;
    else if (i == 10)
        catchBall = true;
}

void pickupCollDetection()
{
    int i;
    for (i = 0; i < 5; i++)
    {
        if (pickup[i].visible)
        {
            if (pickup[i].y < paddle.y + paddle.height && pickup[i].y > paddle.y - pickup[0].dx && pickup[i].x + pickup[0].dx > paddle.x && pickup[i].x < paddle.x + paddle.width)
            {
                pickup[i].visible = false;
                ManagePowerUps(pickup[i].type);
            }
        }
    }
}

void GeneratePowerUps()
{
    int x = rand() % 500;
    int i, j;
    for (i = 2; i <= 22; i += 2)
        if (x >= 2 && x <= i && ball.x > 40 && ball.x < SCRW - 40)
        {
            for (j = 0; j < 5; j++)
                if (!pickup[j].visible)
                {
                    pickup[j].visible = true;
                    pickup[j].type = x / 2 - 1;
                    pickup[j].x = ball.x;
                    pickup[j].y = ball.y;
                    pickup[j].name = filename[x / 2 - 1];
                    return;
                }
        }
}

void PickupMovement()
{
    int i;
    for (i = 0; i < 5; i++)
        if (pickup[i].visible)
        {
            pickup[i].y -= 5;
            if (pickup[i].y < -pickup[i].dx)
                pickup[i].visible = false;
        }
}

/////////////////////////////////////////// POWERUP SECTION END////////////////////////////////////////


/////////////////////////////////////////// BLOCK SECTION START////////////////////////////////////////

void BlockCollDetection()
{
    int i, j, k, q = 0;
    float diffx, diffy;

    for (i = 0; i < 15; i++)
    {
        for (j = 0; j < 24; j++)
        {
            if (blocks[i][j].visible && !blocks[i][j].explode && abs(ball.x - (blocks[i][j].x + 25)) <= (50 / 2 + ball.r) && abs(ball.y - (blocks[i][j].y + 10)) <= (blocks[0][0].dy / 2 + ball.r))
            {
                diffx = abs(ball.x * 1.0 - (blocks[i][j].x - blocks[i][j].x1) / 2.0);
                diffy = abs(ball.y * 1.0 - (blocks[i][j].y - blocks[i][j].y1) / 2.0);

                if (blocks[i][j].type == 1 && !chainExplode)
                {
                    blocks[i][j].explode = true;
                    m = blocks[i][j].index == 48 ? blocks[i][j].index : blocks[i][j].index + 1;
                    n = blocks[i][j].index == 0 ? blocks[i][j].index : blocks[i][j].index - 1;
                    chainExplode = true;
                }

                if (diffx / 50 > diffy / 20)
                {
                    if (!thruBrick)
                        ball.dx *= -1;
                    if (fireBall)
                    {
                        if (blocks[i][j].visible)
                            blocks[i][j].explode = true;
                        if (blocks[i - 1][j].visible)
                            blocks[i - 1][j].explode = true;
                        if (blocks[i + 1][j].visible)
                            blocks[i + 1][j].explode = true;
                        if (blocks[i][j + 1].visible)
                            blocks[i][j + 1].explode = true;
                        if (blocks[i][j - 1].visible)
                            blocks[i][j - 1].explode = true;
                        if (blocks[i - 1][j - 1].visible)
                            blocks[i - 1][j - 1].explode = true;
                        if (blocks[i - 1][j + 1].visible)
                            blocks[i - 1][j + 1].explode = true;
                        if (blocks[i + 1][j + 1].visible)
                            blocks[i + 1][j + 1].explode = true;
                        if (blocks[i + 1][j - 1].visible)
                            blocks[i + 1][j - 1].explode = true;
                        GeneratePowerUps();
                        q = 1;
                        break;
                    }
                    else
                    {
                        if (blocks[i][j].visible)
                        {
                            blocks[i][j].visible = false;
                            score++;
                            GeneratePowerUps();
                            q = 1;
                            break;
                        }
                    }
                }

                else
                {
                    if (!thruBrick)
                        ball.dy *= -1;
                    if (fireBall)
                    {
                        if (blocks[i][j].visible)
                            blocks[i][j].explode = true;
                        if (blocks[i - 1][j].visible)
                            blocks[i - 1][j].explode = true;
                        if (blocks[i + 1][j].visible)
                            blocks[i + 1][j].explode = true;
                        if (blocks[i][j + 1].visible)
                            blocks[i][j + 1].explode = true;
                        if (blocks[i][j - 1].visible)
                            blocks[i][j - 1].explode = true;
                        if (blocks[i - 1][j - 1].visible)
                            blocks[i - 1][j - 1].explode = true;
                        if (blocks[i - 1][j + 1].visible)
                            blocks[i - 1][j + 1].explode = true;
                        if (blocks[i + 1][j + 1].visible)
                            blocks[i + 1][j + 1].explode = true;
                        if (blocks[i + 1][j - 1].visible)
                            blocks[i + 1][j - 1].explode = true;
                        GeneratePowerUps();
                        q = 1;
                        break;
                    }
                    else
                    {
                        if (blocks[i][j].visible)
                        {
                            blocks[i][j].visible = false;
                            score++;
                            GeneratePowerUps();
                            q = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (q)
            break;
    }

    if (shoot)
    {
        for (i = 0; i < 15; i++)
            for (j = 0; j < 24; j++)
                for (k = 0; k < 20; k++)
                    if (blocks[i][j].visible && bullets[k].visible)
                        if (abs(bullets[k].x - (blocks[i][j].x + 50 / 2)) < (50 / 2 + 2) && bullets[k].y + 5 > blocks[i][j].y)
                        {
                            if (blocks[i][j].type == 1 && !chainExplode)
                            {
                                m = blocks[i][j].index == 48 ? blocks[i][j].index : blocks[i][j].index + 1;
                                n = blocks[i][j].index == 0 ? blocks[i][j].index : blocks[i][j].index - 1;
                                chainExplode = true;
                            }
                            blocks[i][j].explode = true;
                            bullets[k].visible = false;
                        }
    }

}

void InitializeBlocks()
{
    int i, j, k;
    int posx = 0, posy = SCRH - 80;
    for (k = 0; k <= 6; k += 2)
    {
        for (i = k; i < 15 - k; i++)
            for (j = k; j < 24 - k; j++)
            {
                blocks[i][j].y = SCRH - 80 - i * 20;
                blocks[i][j].x = j * 50;
                blocks[i][j].dx = 50;
                blocks[i][j].dy = 20;
                blocks[i][j].explode = false;
                blocks[i][j].color[0] = blocks[i][j].color[1] = blocks[i][j].color[2] = -1;
                if (i == k || i == 14 - k || j == k || j == 23 - k)
                    blocks[i][j].visible = true;
                if (k == 4)
                    blocks[i][j].type = 1;
                else
                    blocks[i][j].type = 0;
            }
    }

    k = 1;
    for (i = 4; i < 20; i++, k++)
        blocks[10][i].index = k;
    for (i = 10; i >= 4; i--, k++)
        blocks[i][19].index = k;
    for (i = 19; i >= 4; i--, k++)
        blocks[4][i].index = k;
    for (i = 4; i <= 10; i++, k++)
        blocks[i][4].index = k;
}

int _x = 0;

void ExplodeBlockAnimation()
{
    _x++;
    if (2 * (_x + 4) >= 20)
    {
        _x = 0;
    }
}

void DrawExplodeBlocks(int i, int j)
{
    iSetColor(231, 54, 12);
    iFilledRectangle(blocks[i][j].x, blocks[i][j].y, blocks[i][j].dx, blocks[i][j].dy);
    iSetColor(255, 162, 14);
    iFilledRectangle(blocks[i][j].x + _x, blocks[i][j].y + _x, blocks[i][j].dx - 2 * _x, blocks[i][j].dy - 2 * _x);
    iSetColor(255, 87, 0);
    iFilledRectangle(blocks[i][j].x + _x + 4, blocks[i][j].y + _x + 4, blocks[i][j].dx - 2 * (_x + 4), blocks[i][j].dy - 2 * (_x + 4));
}

void DrawBlocks()
{
    int i, j, k, r, g, b;
    for (j = 0, k = baseColor; j < 24; j++, k++)
    {
        for (i = 0; i < 15; i++)
        {
            r = 100 * sin(k * .2) + 128;
            g = 100 * sin(k * .2 + 2 * 3.1416 / 3) + 128;
            b = 100 * sin(k * .2 + 4 * 3.1416 / 3) + 128;
            iSetColor(r, g, b);

            if (blocks[i][j].visible)
            {
                if (blocks[i][j].type == 1)
                    DrawExplodeBlocks(i, j);
                else
                {
                    if (blocks[i][j].color[0] != -1)
                        iSetColor(blocks[i][j].color[0], blocks[i][j].color[1], blocks[i][j].color[2]);
                    iFilledRectangle(blocks[i][j].x, blocks[i][j].y, blocks[i][j].dx, blocks[i][j].dy);
                    iSetColor(r - 50, g - 50, b - 50);
                    iRectangle(blocks[i][j].x, blocks[i][j].y, blocks[i][j].dx, blocks[i][j].dy);
                }
            }
        }
    }

}

void Explode()
{
    int i, j;
    for (i = 0; i < 15; i++)
        for (j = 0; j < 24; j++)
        {
            if (blocks[i][j].explode && blocks[i][j].visible)
            {
                blocks[i][j].type = 1;
                blocks[i][j].x -= 1;
                blocks[i][j].y -= 1;
                blocks[i][j].dx += 2;
                blocks[i][j].dy += 2;
                if (blocks[i][j].dx > 65)
                {
                    blocks[i][j].visible = false;
                    score++;
                }
            }
        }
}

void ChainExploding()
{
    int i, j;
    if (chainExplode)
    {
        for (i = 4; i < 11; i++)
            for (j = 4; j < 20; j++)
                if (blocks[i][j].type == 1 && blocks[i][j].visible)
                    if (blocks[i][j].index == m || blocks[i][j].index == n)
                        blocks[i][j].explode = true;
        if (m < 48)
            m++;
        if (n > 2)
            n--;
        if (n == 1 && m == 42)
            iPauseTimer(3);
    }
}
/////////////////////////////////////////// BLOCKS SECTION END////////////////////////////////////////

/////////////////////////////////////////// BULLET SECTION START//////////////////////////////////////

void DrawBullets()
{
    int i;
    iSetColor(235, 103, 44);
    for (i = 0; i < 20; i++)
        if (bullets[i].visible)
            iFilledRectangle(bullets[i].x, bullets[i].y, 5, 10);
}

void BulletMotion()
{
    int i;
    for (i = 0; i < 20; i++)
        if (bullets[i].visible)
        {
            bullets[i].y += bullets[0].dy;
            if (bullets[i].y > SCRH)
                bullets[i].visible = false;
        }
}

void GenerateBullets()
{
    int i, ctr = 0;
    for (i = 0; i < 20; i++)
        if (!bullets[i].visible)
        {
            if (ctr == 0)
                bullets[i].x = paddle.x;
            else
                bullets[i].x = paddle.x + paddle.width;
            bullets[i].y = paddle.y + paddle.height + 5;
            bullets[i].visible = true;
            ctr++;
            if (ctr == 2)
                return;
        }
}

/////////////////////////////////////////// BULLET SECTION END//////////////////////////////////////

/////////////////////////////////////////// HUD SECTION START////////7*////////////////////////////////

void DrawLives()
{
    int i, posx = 5, posy = SCRH - 35;
    for (i = 0; i < life; i++)
    {
        iShowBMP2(posx, posy, "sprites\\heart.bmp", 0xFFFFFF);
        posx += 35;
    }
}

void ShowTimer()
{
    sec += 0.015;
    if (sec > 60)
    {
        minute++;
        sec = 0;
    }
    sprintf(timerStr, "%02d : %02d", minute, (int)sec);
    sprintf(scoreText, "SCORE : %d", score);
}

void GameCheck()
{
    if (!life)
    {
        iPauseTimer(0);
        GamePages[4] = true;
    }

    if (score == 200)
    {
        iPauseTimer(0);
        GamePages[4] = true;
    }
}
/////////////////////////////////////////// HUD SECTION END////////////////////////////////////////

/////////////////////////////////////////// MENU SECTION START////////////////////////////////////////

char* menuButtonNames[] = { "sprites\\help.bmp", "sprites\\resume.bmp", "sprites\\play.bmp", "sprites\\highscore.bmp", "sprites\\exit.bmp" };

void DrawMainMenu()
{
    int i, x = 250;
    iShowBMP(415, SCRH - 120, "sprites\\Dxball.bmp");
    iShowBMP2(530, 350, "sprites\\Dxball2.bmp", 0x080307);
    for (i = 0; i < 5; i++)
    {
        iShowBMP(160 * i + x, 150, menuButtonNames[i]);
    }
    if (loadPromt)
    {
        iSetColor(250, 0, 0);
        iText(SCRW / 3 + 140, 320, "NO SAVED GAMES FOUND!");
    }

}

void DrawPauseMenu()
{
    iSetColor(0, 102, 153);
    iFilledRectangle(SCRW / 3 - 60, SCRH / 3 - 60, 520, 420);
    iSetColor(0, 39, 77);
    iFilledRectangle(SCRW / 3 - 50, SCRH / 3 - 50, 500, 400);
    iSetColor(255, 255, 255);
    iText(SCRW / 3 - 50 + 170, SCRH / 3 - 50 + 350, "GAME PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    iShowBMP(SCRW / 3 - 50 + 75, SCRH / 3 - 50 + 175, "sprites\\play.bmp");
    iShowBMP(SCRW / 3 - 50 + 75, SCRH / 3 - 50 + 20, "sprites\\help.bmp");
    iShowBMP(SCRW / 3 - 50 + 300, SCRH / 3 - 50 + 175, "sprites\\restart.bmp");
    iShowBMP(SCRW / 3 - 50 + 300, SCRH / 3 - 50 + 20, "sprites\\exit.bmp");
}

void DrawGamePlay()
{
    iSetColor(250 - 20, 230 - 20, 230 - 20);
    iFilledRectangle(0, 0, 1200, 720);
    DrawBullets();      // BULLETS
    DrawPaddle();       // PADDLE
    DrawBlocks();       // BLOCKS
    DrawBall();         // BALL
    DrawLives();        // LIVES
    iSetColor(50, 50, 50);
    iText(580, SCRH - 25, timerStr, GLUT_BITMAP_TIMES_ROMAN_24);     // TIMER
    iText(SCRW - 150, SCRH - 25, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);    // SCORE
    for (int i = 0; i < 5; i++)
        if (pickup[i].visible)
            iShowBMP(pickup[i].x, pickup[i].y, pickup[i].name);       // PICKUP
    if (paused)
        DrawPauseMenu();
}

char* powerDescription[11] =
{
    "Grants the player an extra life",
    "Makes the ball move faster",
    "Makes the paddle wider",
    "Turns the ball into fireball that can explode several blocks",
    "Makes the ball go through the blocks",
    "Instant game over",
    "Makes the paddle shorter",
    "Allows the paddle to shoot bullets",
    "Makes the ball move slower",
    "Makes the ball small",
    "Makes the paddle to catch and relaunch the ball on every bounce"
};

void DrawHelpMenu()
{
    int i = 0;
    iSetColor(250, 250, 250);
    iText(500, SCRH - 40, "HELP MENU", GLUT_BITMAP_TIMES_ROMAN_24);
    iLine(0, SCRH - 60, SCRW, SCRH - 60);
    iText(20, SCRH - 90, "Clear all the blocks in the level to win the game. Don't let the ball fall down! Pick the power-ups according to your need.", GLUT_BITMAP_HELVETICA_18);
    iText(20, SCRH - 120, "Left click and drag to move the paddle. Right click to launch ball / shoot bullets.", GLUT_BITMAP_HELVETICA_18);
    iText(20, SCRH - 150, "Left button / 'a' : Move paddle to the Left", GLUT_BITMAP_HELVETICA_18);
    iText(20, SCRH - 180, "Right button / 'a' : Move paddle to the Right", GLUT_BITMAP_HELVETICA_18);
    iText(20, SCRH - 210, "Space button / Right Mouse Click : Launch ball / Shoot bullets", GLUT_BITMAP_HELVETICA_18);
    iText(20, SCRH - 240, "F1 / 'p' : Pause Menu", GLUT_BITMAP_HELVETICA_18);
    for (i = 0; i < 11; i++)
    {
        iShowBMP(20, 30 + i * 40, filename[i]);
        iText(20 + 50, 37 + i * 40, powerDescription[i], GLUT_BITMAP_HELVETICA_18);
    }
    iShowBMP(SCRW - 20 - 125, 20, "sprites\\exit.bmp");
}


bool gameOverMenuToggle = false;

void DrawGameOverMenu()
{
    iSetColor(0, 39, 77);
    iFilledRectangle(SCRW / 3 - 50, 0, 500, SCRH);
    iSetColor(255, 255, 255);
    if (!life)
        iText(SCRW / 3 + 130, SCRH - 50, "GAME OVER!", GLUT_BITMAP_TIMES_ROMAN_24);
    else
        iText(SCRW / 3 + 130, SCRH - 50, "YOU WON!", GLUT_BITMAP_TIMES_ROMAN_24);
    iLine(SCRW / 3 - 50, SCRH - 75, SCRW / 3 - 50 + 500, SCRH - 75);
    iText(SCRW / 3 - 30, SCRH - 150, "ENTER YOUR NAME: ", GLUT_BITMAP_HELVETICA_18);

    char scorestr[20];
    sprintf(scorestr, "YOUR SCORE : %d", score);
    iText(SCRW / 3 + 100, 400, scorestr, GLUT_BITMAP_TIMES_ROMAN_24);
    sprintf(scorestr, "TIME TAKEN : %02d:%02d", minute, (int)sec);
    iText(SCRW / 3 + 82, 360, scorestr, GLUT_BITMAP_TIMES_ROMAN_24);

    if (nameInput)
        iSetColor(255, 255, 255);
    else
        iSetColor(150, 150, 150);

    iRectangle(SCRW / 3 - 30, SCRH - 230, 450, 40);
    iText(SCRW / 3 - 20, SCRH - 217, playerName, GLUT_BITMAP_HELVETICA_18);

    if (namePrompt)
    {
        iSetColor(250, 0, 0);
        iText(SCRW / 3 + 80, 300, "BLANK NAME IS NOT ALLOWED!");
    }

    if (gameOverMenuToggle)
    {
        iShowBMP(SCRW / 3 - 20, 50, "sprites\\restart.bmp");
        iShowBMP(SCRW / 3 + 140, 50, "sprites\\highscore.bmp");
        iShowBMP(SCRW / 3 + 300, 50, "sprites\\exit.bmp");
    }
}

int flag;
void DrawHighScoreMenu()
{
    int i;
    iSetColor(255, 255, 255);
    iText(SCRW / 3 + 100, SCRH - 50, "HIGH SCORE", GLUT_BITMAP_TIMES_ROMAN_24);
    iLine(0, SCRH - 75, SCRW, SCRH - 75);
    iText(SCRW / 7, SCRH - 120, "NAME", GLUT_BITMAP_HELVETICA_18);
    iText(SCRW * 4.8 / 7, SCRH - 120, "SCORE", GLUT_BITMAP_HELVETICA_18);
    for (i = 0; i < 10; i++)
    {
        if (playerInfo[i].name[0])
            iText(SCRW / 7 - 10, SCRH - 180 - 40 * i, playerInfo[i].name, GLUT_BITMAP_HELVETICA_18);
        else
            iText(SCRW / 7 - 10, SCRH - 180 - 40 * i, "   -   ", GLUT_BITMAP_HELVETICA_18);

        char num[5] = " - ";
        if (playerInfo[i].score != -1)
            sprintf(num, "%d", playerInfo[i].score);
        iText(SCRW * 5 / 7, SCRH - 180 - 40 * i, num, GLUT_BITMAP_HELVETICA_18);
    }
    iShowBMP(SCRW - 150, 10, "sprites\\exit.bmp");
}
/////////////////////////////////////////// MENU SECTION END////////////////////////////////////////

/////////////////////////////////////////// SAVE & LOAD SECTION START///////////////////////////////

void Restart()
{
    int i;
    paddle.x = SCRW / 2 - paddle.width / 2;
    paddle.y = SCRH / 12;
    paddle.width = 150;
    ball.x = SCRW / 2;
    ball.y = SCRH / 8 - 6;
    ball.r = 8;
    ball.velocity = 5;
    ball.theta = 40;
    ball.dx = -ball.velocity * cos(ball.theta * PI / 180);
    ball.dy = ball.velocity * sin(ball.theta * PI / 180);
    ball.offset = paddle.width / 2;
    ball.launched = false;
    chainExplode = false;
    InitializeBlocks();
    for (i = 0; i < 20; i++)
        bullets[i].visible = false;
    for (i = 0; i < 5; i++)
        pickup[i].visible = false;
    life = 3;
    offset = -1;
    score = 0;
    thruBrick = fireBall = paused = shoot = catchBall = false;
    sec = -0.015;
    memset(playerName, 0, sizeof(playerName));
    nameIndex = -1;
    nameInput = namePrompt = false;
}

void SaveGame()
{
    int i, j;
    fp = fopen("SAVES.txt", "w");
    fprintf(fp, "%d\n", 1);
    fprintf(fp, "%f\n", paddle.width);
    fprintf(fp, "%f\n", ball.r);
    fprintf(fp, "%f\n", ball.velocity);
    fprintf(fp, "%d\n", life);
    fprintf(fp, "%f\n", sec);
    fprintf(fp, "%d\n", score);
    fprintf(fp, "%d %d %d %d\n", thruBrick, fireBall, shoot, catchBall);
    for (i = 0; i < 15; i++)
        for (j = 0; j < 24; j++)
            fprintf(fp, "%d ", blocks[i][j].visible);
    fprintf(fp, "%d %d", (blocks[6][6].type), (blocks[6][17].type));
    fclose(fp);
}

void LoadGame()
{
    int x, i, j;
    fp = fopen("SAVES.txt", "r");
    if (fp == NULL)
    {
        loadPromt = true;
        fclose(fp);
        return;
    }
    fscanf(fp, "%d", &x);
    if (!x)
    {
        loadPromt = true;
        fclose(fp);
        return;
    }
    fscanf(fp, "%lf", &paddle.width);
    fscanf(fp, "%lf", &ball.r);
    fscanf(fp, "%lf", &ball.velocity);
    fscanf(fp, "%d", &life);
    fscanf(fp, "%lf", &sec);
    fscanf(fp, "%d", &score);
    fscanf(fp, "%d%d%d%d", &thruBrick, &fireBall, &shoot, &catchBall);
    for (i = 0; i < 15; i++)
        for (j = 0; j < 24; j++)
            fscanf(fp, "%d", &blocks[i][j].visible);
    fscanf(fp, "%d%d", &(blocks[6][6].type), &(blocks[6][17].type));
    fclose(fp);
}

void LoadHighScore()
{
    int i, j, k;
    char x[350];
    fp = fopen("HIGHSCORE.txt", "r");
    if (fp == NULL)
    {
        fclose(fp);

        fp = fopen("HIGHSCORE.txt", "w");
        for (i = 0; i < 10; i++)
            fprintf(fp, "%d ", -1);
        for (i = 0; i < 10; i++)
            fprintf(fp, "%s,", "");
        fclose(fp);
        fp = fopen("HIGHSCORE.txt", "r");
    }

    for (i = 0; i < 10; i++)
        if (fscanf(fp, "%d", &(playerInfo[i].score)) != 1)
            exit(1);
    fgets(x, 350, fp);
    fclose(fp);
    for (i = 0, j = 1, k = 0; i < 10; j++)
    {
        if (x[j] == ',')
        {
            playerInfo[i].name[k] = '\0';
            i++;
            k = 0;
        }
        else
        {
            playerInfo[i].name[k] = x[j];
            k++;
        }
    }
}

void SaveHighScore()
{
    int i, j, k;

    LoadHighScore();

    if (score > playerInfo[9].score)
    {
        playerInfo[9].score = score;
        strcpy(playerInfo[9].name, playerName);
    }

    for (i = 0; i < 10; i++)
    {
        int maximum = 9;
        for (j = i; j < 10; j++)
        {
            if (playerInfo[j].score > playerInfo[maximum].score)
                maximum = j;
        }
        int t = playerInfo[i].score;
        playerInfo[i].score = playerInfo[maximum].score;
        playerInfo[maximum].score = t;

        char tc[33];
        strcpy(tc, playerInfo[i].name);
        strcpy(playerInfo[i].name, playerInfo[maximum].name);
        strcpy(playerInfo[maximum].name, tc);
    }

    fp = fopen("HIGHSCORE.txt", "w");
    for (i = 0; i < 10; i++)
        fprintf(fp, "%d ", playerInfo[i].score);
    for (i = 0; i < 10; i++)
        fprintf(fp, "%s,", playerInfo[i].name);
    fclose(fp);
}

/////////////////////////////////////////// SAVE & LOAD SECTION END///////////////////////////////

void Update()
{
    PaddleCollDetection();
    BallMovement();
    PickupMovement();
    pickupCollDetection();
    BlockCollDetection();
    Explode();
    BulletMotion();
    ShowTimer();
    GameCheck();
}

void SlowUpdate()
{
    baseColor++;
    ExplodeBlockAnimation();
}

void iDraw()
{
    iClear();

    if (GamePages[0])           // START
        DrawMainMenu();
    else if (GamePages[1])      // GAMEPLAY
        DrawGamePlay();
    else if (GamePages[2])       // HELP
        DrawHelpMenu();
    else if (GamePages[3])       // HIGHSCORE
        DrawHighScoreMenu();
    if (GamePages[4])           // GAME OVER
        DrawGameOverMenu();
}

void iMouseMove(int mx, int my)
{
    if (offset > 0)
        paddle.x = startPoint - offset;
    if (paddle.x + paddle.width > SCRW)
        paddle.x = SCRW - paddle.width;
    if (paddle.x < 0)
        paddle.x = 0;
    startPoint = mx;
}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (GamePages[0])    // start
        {
            if (mx >= 250 && mx <= 375 && my >= 150 && my <= 275)   // help
            {
                loadPromt = false;
                GamePages[0] = false;
                GamePages[2] = true;
            }
            if (mx >= 410 && mx <= 535 && my >= 150 && my <= 275)   // load game
            {
                Restart();
                LoadGame();
                if (!loadPromt)
                {
                    iResumeTimer(0);
                    GamePages[0] = false;
                    GamePages[1] = true;
                }
            }
            if (mx >= 570 && mx <= 695 && my >= 150 && my <= 275)   // new game
            {
                loadPromt = false;
                Restart();
                iResumeTimer(0);
                GamePages[0] = false;
                GamePages[1] = true;
            }
            if (mx >= 730 && mx <= 855 && my >= 150 && my <= 275)   // score
            {
                LoadHighScore();
                GamePages[3] = true;
                flag = 1;
                GamePages[0] = false;
            }
            if (mx >= 890 && mx <= 1015 && my >= 150 && my <= 275)  // exit
                exit(0);
        }

        else if (GamePages[1])   // game
        {
            if (paused)
            {
                if (mx >= SCRW / 3 - 50 + 75 && mx <= SCRW / 3 - 50 + 75 + 125 && my >= SCRH / 3 - 50 + 175 && my <= SCRH / 3 - 50 + 175 + 125)
                {
                    iResumeTimer(0);        // resume
                    paused = false;
                }
                if (mx >= SCRW / 3 - 50 + 75 && mx <= SCRW / 3 - 50 + 75 + 125 && my >= SCRH / 3 - 50 + 20 && my <= SCRH / 3 - 50 + 20 + 125)
                {
                    GamePages[1] = false;       // help
                    GamePages[2] = true;
                }
                if (mx >= SCRW / 3 - 50 + 300 && mx <= SCRW / 3 - 50 + 300 + 125 && my >= SCRH / 3 - 50 + 175 && my <= SCRH / 3 - 50 + 175 + 125)
                {
                    Restart();      // restart
                    iResumeTimer(0);
                    paused = false;
                }
                if (mx >= SCRW / 3 - 50 + 300 && mx <= SCRW / 3 - 50 + 300 + 125 && my >= SCRH / 3 - 50 + 20 && my <= SCRH / 3 - 50 + 20 + 125)
                {
                    SaveGame();
                    paused = false;         // exit
                    GamePages[1] = false;
                    GamePages[0] = true;
                }
            }
            else
            {
                if (mx >= paddle.x && mx <= paddle.x + paddle.width && my >= paddle.y && my <= paddle.y + paddle.height)
                {
                    offset = mx - paddle.x;
                    startPoint = mx;
                }
                else
                    offset = -1;
            }
        }

        else if (GamePages[2])     // help
        {
            if (mx >= SCRW - 20 - 125 && mx <= SCRW - 20 && my >= 20 && my <= 20 + 125)
            {
                GamePages[2] = false;
                if (paused)
                    GamePages[1] = true;
                else
                    GamePages[0] = true;
            }
        }

        else if (GamePages[3])      // score
        {
            if (mx > SCRW - 150 && mx < SCRW - 25 && my > 10 && my < 135)
            {
                if (flag == 1)
                {
                    flag = -1;
                    GamePages[3] = false;
                    GamePages[0] = true;
                    loadPromt = false;
                }
                if (flag == 0)
                {
                    flag = -1;
                    GamePages[3] = false;
                    GamePages[1] = true;
                    GamePages[4] = true;
                }
            }
        }

        if (GamePages[4])       // game over
        {
            if (!gameOverMenuToggle)
            {
                if (mx >= SCRW / 3 - 30 && mx <= SCRW / 3 + 420 && my >= SCRH - 230 && my <= SCRH - 190)
                    nameInput = true;
                else
                    nameInput = false;
            }

            else
            {
                if (mx >= SCRW / 3 - 20 && mx <= SCRW / 3 + 105 && my >= 50 && my <= 175)   // restart
                {
                    Restart();
                    iResumeTimer(0);
                    gameOverMenuToggle = false;
                    GamePages[4] = false;
                }
                if (mx >= SCRW / 3 + 140 && mx <= SCRW / 3 + 265 && my >= 50 && my <= 175)  // score
                {
                    LoadHighScore();
                    flag = 0;
                    GamePages[3] = true;
                    GamePages[4] = false;
                    GamePages[1] = false;
                }

                if (mx >= SCRW / 3 + 300 && mx <= SCRW / 3 + 425 && my >= 50 && my <= 175)  //exit
                {
                    fp = fopen("SAVES.txt", "w");
                    fprintf(fp, "%d", 0);
                    fclose(fp);
                    gameOverMenuToggle = false;
                    GamePages[4] = false;
                    GamePages[0] = true;
                }
            }
        }
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        if (!ball.launched && catchBall)
        {
            ball.launched = true;
            catchBall = false;
            iResumeTimer(1);
        }
        else if (shoot && ball.launched)
        {
            GenerateBullets();
            iResumeTimer(2);
        }
        else
            ball.launched = true;
    }
}

void iKeyboard(unsigned char key)
{
    if (GamePages[4] && nameInput)
    {
        if (key != '\b' && key != '\r')
        {
            nameIndex++;
            if (nameIndex > 32)
            {
                playerName[nameIndex] = 0;
                nameIndex--;
            }
            else
                playerName[nameIndex] = key;
        }
        else if (key == '\b')
        {
            playerName[nameIndex] = 0;
            if (nameIndex >= 0)
                nameIndex--;
        }
        else if (key == '\r')
        {
            if (*playerName)
            {
                nameInput = false;
                gameOverMenuToggle = true;
                namePrompt = false;
            }
            else
                namePrompt = true;
            SaveHighScore();
        }
    }

    else
    {
        if (key == 'p')
        {
            if (GamePages[1])
            {
                if (!paused)
                {
                    iPauseTimer(0);
                    iPauseTimer(1);
                }
                else
                {
                    iResumeTimer(0);
                    iResumeTimer(1);
                }
                paused = !paused;
            }
        }

        if (key == ' ')
        {
            if (!ball.launched && catchBall)
            {
                ball.launched = true;
                catchBall = false;
                iResumeTimer(1);
            }
            else if (shoot && ball.launched)
            {
                GenerateBullets();
                iResumeTimer(2);
            }
            else
                ball.launched = true;
        }

        if (key == 'd')
        {
            paddle.x += paddle.dx;
            if (paddle.x + paddle.width > SCRW)
                paddle.x = SCRW - paddle.width;
        }

        if (key == 'a')
        {
            paddle.x -= paddle.dx;
            if (paddle.x < 0)
                paddle.x = 0;
        }
    }
}

void iSpecialKeyboard(unsigned char key)
{
    if (key == GLUT_KEY_RIGHT)
    {
        paddle.x += paddle.dx;
        if (paddle.x + paddle.width > SCRW)
            paddle.x = SCRW - paddle.width;
        if (!ball.launched)
            ball.x += paddle.dx;
    }
    if (key == GLUT_KEY_LEFT)
    {
        paddle.x -= paddle.dx;
        if (paddle.x < 0)
            paddle.x = 0;
        if (!ball.launched)
            ball.x -= paddle.dx;
    }
    if (key == GLUT_KEY_F1)
    {
        if (GamePages[1])
        {
            if (!paused)
            {
                iPauseTimer(0);
                iPauseTimer(1);
            }
            else
            {
                iResumeTimer(0);
                iResumeTimer(1);
            }
            paused = !paused;
        }
    }
    if (key == GLUT_KEY_END)
    {
        exit(0);
    }
}

int main()
{
    srand(time(0));
    iSetTimer(15, Update);
    iSetTimer(100, delayTimer);
    iSetTimer(50, SlowUpdate);
    iSetTimer(25, ChainExploding);
    iPauseTimer(1);
    Restart();
    iInitialize(SCRW, SCRH, "DX BALL - 1905024");
    return 0;
}
