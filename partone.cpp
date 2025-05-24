#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Game states
enum GameState { INTRO, BOMB_DROP_CINEMATIC, TAKEOFF_CINEMATIC, AERIAL_ENGAGEMENT, LANDING_CINEMATIC, CONGRATULATION, GAME_OVER };
GameState currentState = INTRO;
int cinematicTimer = 0;
int score = 0;
const int WIDTH = 1000;
const int HEIGHT = 600;

// Camera offset
float cameraOffsetX = 0.0f;
float cameraOffsetY = 0.0f;

// Player (F-16 "Crimson One")
struct Fighter {
    float x, y;
    float speedX, speedY;
    bool alive;
    vector<pair<float, float>> bullets;
} player;

// Enemy (MiG-21 "Bison")
struct Enemy {
    float x, y;
    float speed;
    bool alive;
    int type; // 0 = MiG-21
    int health;
};

vector<Enemy> enemies;

// Bomb drop variables
float indianJetX = WIDTH + 100;
float indianJetY = HEIGHT * 0.7f;
bool bombDropped = false;
float bombX, bombY;
float explosionSize = 0.0f;
bool buildingDestroyed = false;
float fireSize = 0.0f;
float destroyedBuildingYOffset = 0.0f;
float bombTargetX = 150;
float bombTargetY = 60;

// Aerial engagement variables
bool canPlayerFire = false;
int enemiesDestroyedInGame = 0;

// Landing cinematic variables
float landingPlayerSpeed = 5.0f;
bool playerLanded = false;

// Congratulation cinematic variables
float congratulationTimer = 0;

// Function Declarations
void drawCircle(float cx, float cy, float r, int segments);
void drawF16(float x, float y, float scale);
void drawMig21(float x, float y, float scale);
void drawBomb(float x, float y);
void drawFire(float x, float y, float size);
void drawDestroyedBuilding(float baseX, float baseY, float offset);
void drawRunway();
void drawText(float x, float y, const char* text, void* font);
void drawCenteredText(float y, const char* text, void* font);
void drawExplosion(float x, float y, float size);
void drawBullet(float x, float y);
void drawArmyPersonnel(float x, float y, float scale);
void drawSkyGradient();
void updateBombDropCinematic(int value);
void updateTakeoffCinematic(int value);
void updateAerialEngagement(int value);
void updateLandingCinematic(int value);
void updateCongratulationCinematic(int value);
void updateGame(int value);
bool checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);
void resetGame();

// Drawing Functions
void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * M_PI * i / segments;
        glVertex2f(cx + r * cos(theta), cy + r * sin(theta));
    }
    glEnd();
}

void drawF16(float x, float y, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // Fuselage
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(-30, 0); glVertex2f(60, 6); glVertex2f(60, -6); glVertex2f(-30, 0);
    glEnd();

    // Vertical stabilizer
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-15, 0); glVertex2f(-5, 0); glVertex2f(-10, 15);
    glEnd();

    // Cockpit
    glColor3f(0.0f, 0.2f, 0.6f);
    drawCircle(40, 0, 6, 30);

    // Main wings
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_POLYGON); // Left wing
    glVertex2f(10, 0); glVertex2f(35, 0); glVertex2f(20, 20); glVertex2f(-5, 18);
    glEnd();
    glBegin(GL_POLYGON); // Right wing
    glVertex2f(10, 0); glVertex2f(35, 0); glVertex2f(20, -20); glVertex2f(-5, -18);
    glEnd();

    // Tail wings
    glBegin(GL_POLYGON); // Left tail wing
    glVertex2f(-20, 0); glVertex2f(-10, 0); glVertex2f(-5, 10); glVertex2f(-15, 10);
    glEnd();
    glBegin(GL_POLYGON); // Right tail wing
    glVertex2f(-20, 0); glVertex2f(-10, 0); glVertex2f(-5, -10); glVertex2f(-15, -10);
    glEnd();

    // Nose cone
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(60, 0); glVertex2f(70, 3); glVertex2f(70, -3);
    glEnd();

    // Engine exhaust
    glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(-30, 0, 3, 20);

    // Enhanced afterburner effect
    if (currentState == TAKEOFF_CINEMATIC || currentState == AERIAL_ENGAGEMENT || rand() % 10 > 5) {
        glColor3f(1.0f, 0.6f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-30, 2); glVertex2f(-50, 0); glVertex2f(-30, -2);
        glEnd();
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-30, 1); glVertex2f(-45, 0); glVertex2f(-30, -1);
        glEnd();
    }

    glPopMatrix();
}

void drawMig21(float x, float y, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // Body
    glColor3f(0.4f, 0.5f, 0.3f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0); glVertex2f(50, 7); glVertex2f(50, -7);
    glEnd();

    // Cockpit
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(40, 0, 5, 20);

    // Wings
    glColor3f(0.2f, 0.4f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(10, 5); glVertex2f(35, 5); glVertex2f(25, 20); glVertex2f(5, 20);
    glVertex2f(10, -5); glVertex2f(35, -5); glVertex2f(25, -20); glVertex2f(5, -20);
    glVertex2f(40, 3); glVertex2f(50, 3); glVertex2f(50, -3); glVertex2f(40, -3);
    glEnd();

    // Exhaust
    glColor3f(0.8f, 0.2f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 3); glVertex2f(-10, 0); glVertex2f(0, -3);
    glEnd();

    glPopMatrix();
}

void drawBomb(float x, float y) {
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_POLYGON);
    glVertex2f(x - 5, y); glVertex2f(x + 5, y); glVertex2f(x + 5, y + 20); glVertex2f(x - 5, y + 20);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 5, y + 20); glVertex2f(x + 5, y + 20); glVertex2f(x, y + 25);
    glEnd();
}

void drawFire(float x, float y, float size) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.5f, 0.0f, 0.8f); // Orange with transparency
    drawCircle(x, y, size * 0.7f, 24);
    glColor4f(1.0f, 0.8f, 0.0f, 0.7f); // Yellow
    drawCircle(x + size * 0.3f, y + size * 0.2f, size * 0.5f, 18);
    glColor4f(0.8f, 0.2f, 0.0f, 0.6f); // Red
    drawCircle(x - size * 0.2f, y - size * 0.1f, size * 0.3f, 12);
    glDisable(GL_BLEND);
}

void drawDestroyedBuilding(float baseX, float baseY, float offset) {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(baseX - 50, baseY + offset);
    glVertex2f(baseX + 50, baseY + offset);
    glVertex2f(baseX + 50, baseY + 80 + offset);
    glVertex2f(baseX - 50, baseY + 80 + offset);
    glEnd();

    // Rubble
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex2f(baseX - 40, baseY + 60 + offset);
    glVertex2f(baseX - 20, baseY + 70 + offset);
    glVertex2f(baseX - 30, baseY + 90 + offset);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(baseX + 20, baseY + 50 + offset);
    glVertex2f(baseX + 40, baseY + 40 + offset);
    glVertex2f(baseX + 30, baseY + 30 + offset);
    glVertex2f(baseX + 10, baseY + 40 + offset);
    glEnd();
}

void drawRunway() {
    // Runway
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(WIDTH * 2, 0); glVertex2f(WIDTH * 2, 60); glVertex2f(0, 60);
    glEnd();

    // Runway markings
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < WIDTH * 2; i += 100) {
        glBegin(GL_QUADS);
        glVertex2f(i, 30); glVertex2f(i + 50, 30); glVertex2f(i + 50, 25); glVertex2f(i, 25);
        glEnd();
    }

    // Administrative complex
    float buildingBaseX = 150;
    float buildingBaseY = 60;
    if (!buildingDestroyed) {
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(buildingBaseX - 50, buildingBaseY);
        glVertex2f(buildingBaseX + 50, buildingBaseY);
        glVertex2f(buildingBaseX + 50, buildingBaseY + 100);
        glVertex2f(buildingBaseX - 50, buildingBaseY + 100);
        glEnd();
    } else {
        drawDestroyedBuilding(buildingBaseX, buildingBaseY, destroyedBuildingYOffset);
        drawFire(buildingBaseX, buildingBaseY + 50, fireSize);
    }

    // Bangladesh flag
    float poleX = WIDTH + 170;
    float poleBaseY = 60;
    float poleHeight = 120;
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
    glVertex2f(poleX, poleBaseY); glVertex2f(poleX, poleBaseY + poleHeight);
    glEnd();
    float flagWidth = 60;
    float flagHeight = 40;
    glColor3f(0.0f, 0.6f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(poleX, poleBaseY + poleHeight - flagHeight);
    glVertex2f(poleX + flagWidth, poleBaseY + poleHeight - flagHeight);
    glVertex2f(poleX + flagWidth, poleBaseY + poleHeight);
    glVertex2f(poleX, poleBaseY + poleHeight);
    glEnd();
    float circleCenterX = poleX + flagWidth * 0.35f;
    float circleCenterY = poleBaseY + poleHeight - flagHeight / 2;
    float circleRadius = flagHeight * 0.25f;
    glColor3f(0.8f, 0.0f, 0.0f);
    drawCircle(circleCenterX, circleCenterY, circleRadius, 40);

    // Maintenance crews
    drawArmyPersonnel(WIDTH + 300, 60, 0.6f);
    drawArmyPersonnel(WIDTH + 350, 60, 0.6f);

    // Airport terminal
    glColor3f(0.45f, 0.45f, 0.45f);
    glBegin(GL_QUADS);
    glVertex2f(WIDTH + 250, 60); glVertex2f(WIDTH + 650, 60);
    glVertex2f(WIDTH + 650, 220); glVertex2f(WIDTH + 250, 220);
    glEnd();
    glColor3f(0.7f, 0.85f, 0.95f);
    glBegin(GL_LINES);
    for (int i = 0; i < 5; ++i) {
        glVertex2f(WIDTH + 250, 80 + i * 28);
        glVertex2f(WIDTH + 650, 80 + i * 28);
    }
    glEnd();

    // Control tower
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(WIDTH + 500, 220); glVertex2f(WIDTH + 560, 220);
    glVertex2f(WIDTH + 560, 340); glVertex2f(WIDTH + 500, 340);
    glEnd();
    glColor3f(0.4f, 0.4f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(WIDTH + 490, 340); glVertex2f(WIDTH + 570, 340);
    glVertex2f(WIDTH + 570, 370); glVertex2f(WIDTH + 490, 370);
    glEnd();

    // Air Force building
    float buildingX = WIDTH + 660;
    float buildingY = 60;
    float buildingWidth = 400;
    float buildingHeight = 160;
    glColor3f(0.45f, 0.45f, 0.45f);
    glBegin(GL_QUADS);
    glVertex2f(buildingX, buildingY); glVertex2f(buildingX + buildingWidth, buildingY);
    glVertex2f(buildingX + buildingWidth, buildingY + buildingHeight); glVertex2f(buildingX, buildingY + buildingHeight);
    glEnd();
    glColor3f(0.7f, 0.85f, 0.95f);
    glBegin(GL_LINES);
    for (int i = 0; i < 5; ++i) {
        glVertex2f(buildingX, buildingY + 20 + i * 28);
        glVertex2f(buildingX + buildingWidth, buildingY + 20 + i * 28);
    }
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    const char* label = "SHAHEEN AIRBASE";
    float textX = buildingX + buildingWidth / 2.0f - (strlen(label) * 9) / 2;
    float textY = buildingY + buildingHeight / 2 + 10;
    drawText(textX, textY, label, GLUT_BITMAP_HELVETICA_18);

    // Trees
    glColor3f(0.0f, 0.6f, 0.0f);
    for (int i = 0; i < WIDTH * 2; i += 100) {
        glBegin(GL_TRIANGLES);
        glVertex2f(i + 40, 60); glVertex2f(i + 20, 100); glVertex2f(i + 60, 100);
        glVertex2f(i + 40, 80); glVertex2f(i + 20, 120); glVertex2f(i + 60, 120);
        glEnd();
    }
}

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void drawCenteredText(float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    int textWidth = 0;
    for (const char* c = text; *c != '\0'; c++) {
        textWidth += glutBitmapWidth(font, *c);
    }
    drawText(WIDTH / 2 - textWidth / 2, y, text, font);
}

void drawExplosion(float x, float y, float size) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.5f, 0.0f, 0.8f); // Orange
    drawCircle(x, y, size * 0.7f, 24);
    glColor4f(1.0f, 1.0f, 0.0f, 0.7f); // Yellow
    drawCircle(x, y, size * 0.5f, 18);
    glColor4f(1.0f, 0.0f, 0.0f, 0.6f); // Red
    drawCircle(x, y, size * 0.3f, 12);
    glDisable(GL_BLEND);
}

void drawBullet(float x, float y) {
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x, y + 2); glVertex2f(x + 10, y + 2); glVertex2f(x + 10, y - 2); glVertex2f(x, y - 2);
    glEnd();
}

void drawArmyPersonnel(float x, float y, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    // Body
    glColor3f(0.2f, 0.3f, 0.2f); // Military green
    glBegin(GL_QUADS);
    glVertex2f(-10, 0); glVertex2f(10, 0); glVertex2f(10, 30); glVertex2f(-10, 30);
    glEnd();

    // Head
    glColor3f(0.8f, 0.7f, 0.6f); // Skin tone
    drawCircle(0, 35, 8, 20);

    // Arms (corrected)
    glColor3f(0.2f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
    // Left arm
    glVertex2f(-15, 20); glVertex2f(-10, 25); glVertex2f(-10, 15); glVertex2f(-15, 10);
    // Right arm (completed)
    glVertex2f(10, 25); glVertex2f(15, 20); glVertex2f(15, 10); glVertex2f(10, 15);
    glEnd();

    glPopMatrix();
}

void drawSkyGradient() {
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.7f, 1.0f); // Light blue at top
    glVertex2f(0, HEIGHT); glVertex2f(WIDTH, HEIGHT);
    glColor3f(1.0f, 0.7f, 0.4f); // Soft orange at bottom
    glVertex2f(WIDTH, 0); glVertex2f(0, 0);
    glEnd();
}

// Update Functions
void updateBombDropCinematic(int value) {
    cinematicTimer++;
    indianJetX -= 7.0f; // Faster approach for dramatic effect

    if (indianJetX < bombTargetX + 50 && !bombDropped) {
        bombDropped = true;
        bombX = indianJetX;
        bombY = indianJetY - 20;
        // PlaySound(TEXT("bomb_drop.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
    }

    if (bombDropped) {
        bombY -= 10.0f;
        if (bombY <= bombTargetY + 10 && !buildingDestroyed) {
            buildingDestroyed = true;
            explosionSize = 1.0f;
            fireSize = 1.0f;
            // PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
        }
    }

    if (buildingDestroyed) {
        explosionSize += 4.0f;
        fireSize += 2.0f;
        destroyedBuildingYOffset -= 0.7f;
        if (explosionSize > 200) explosionSize = 200;
        if (fireSize > 100) fireSize = 100;
        if (destroyedBuildingYOffset < -30) destroyedBuildingYOffset = -30;
    }

    if (indianJetX < -200 && cinematicTimer > 300) {
        currentState = TAKEOFF_CINEMATIC;
        cinematicTimer = 0;
        player.x = -100;
        player.y = 50;
        player.speedX = 0; player.speedY = 0;
        player.alive = true;
        bombDropped = false;
        buildingDestroyed = false;
        explosionSize = 0.0f;
        fireSize = 0.0f;
        destroyedBuildingYOffset = 0.0f;
        indianJetX = WIDTH + 100;
        glutTimerFunc(16, updateTakeoffCinematic, 0);
        return;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateBombDropCinematic, 0);
}

void updateTakeoffCinematic(int value) {
    cinematicTimer++;
    cameraOffsetX = player.x - WIDTH / 4;
    if (cameraOffsetX < 0) cameraOffsetX = 0;

    if (cinematicTimer < 60) {
        player.x += 3.0f;
    } else if (cinematicTimer < 180) {
        player.x += 6.0f;
    } else if (cinematicTimer < 300) {
        player.x += 10.0f;
        player.y += 3.0f;
    } else if (cinematicTimer < 450) {
        player.x += 12.0f;
        player.y += 4.0f;
    } else {
        currentState = AERIAL_ENGAGEMENT;
        cinematicTimer = 0;
        canPlayerFire = true;
        player.x = WIDTH / 4;
        player.y = HEIGHT / 2;
        cameraOffsetX = 0;

        Enemy e1, e2;
        e1.x = WIDTH + 100; e1.y = HEIGHT * 0.3f; e1.speed = 4.0f; e1.alive = true; e1.type = 0; e1.health = 3;
        e2.x = WIDTH + 300; e2.y = HEIGHT * 0.7f; e2.speed = 3.5f; e2.alive = true; e2.type = 0; e2.health = 3;
        enemies.push_back(e1);
        enemies.push_back(e2);
        glutTimerFunc(16, updateAerialEngagement, 0);
        return;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateTakeoffCinematic, 0);
}

void updateAerialEngagement(int value) {
    cinematicTimer++;

    // Player bullet movement
    for (auto& bullet : player.bullets) {
        bullet.first += 15.0f;
    }
    player.bullets.erase(remove_if(player.bullets.begin(), player.bullets.end(),
        [](const pair<float, float>& b) { return b.first > WIDTH; }), player.bullets.end());

    // Enemy movement with enhanced AI
    for (auto& enemy : enemies) {
        if (enemy.alive) {
            enemy.x -= enemy.speed;
            enemy.y += sin(enemy.x / 50.0f + cinematicTimer * 0.1f) * 1.0f; // Dynamic wobble
            if (rand() % 100 < 5) enemy.y += (player.y > enemy.y ? 2.0f : -2.0f); // Track player
        }
    }
    enemies.erase(remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.x < -100 || !e.alive; }), enemies.end());

    // Collision detection
    for (auto& bullet : player.bullets) {
        for (auto& enemy : enemies) {
            if (enemy.alive && checkCollision(bullet.first, bullet.second, 10, 4, enemy.x, enemy.y, 70, 40)) {
                enemy.health--;
                bullet.first = -100;
                if (enemy.health <= 0) {
                    enemy.alive = false;
                    score += 100;
                    enemiesDestroyedInGame++;
                    // PlaySound(TEXT("enemy_destroyed.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
                }
                break;
            }
        }
    }

    // Check for player-enemy collision (game over condition)
    for (auto& enemy : enemies) {
        if (enemy.alive && checkCollision(player.x, player.y, 70, 40, enemy.x, enemy.y, 70, 40)) {
            currentState = GAME_OVER;
            player.alive = false;
            glutTimerFunc(16, updateGame, 0);
            return;
        }
    }

    if (enemiesDestroyedInGame >= 2) {
        currentState = LANDING_CINEMATIC;
        cinematicTimer = 0;
        canPlayerFire = false;
        enemies.clear();
        player.bullets.clear();
        player.x = WIDTH / 2;
        player.y = HEIGHT * 0.7f;
        playerLanded = false;
        glutTimerFunc(16, updateLandingCinematic, 0);
        return;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateAerialEngagement, 0);
}

void updateLandingCinematic(int value) {
    cinematicTimer++;

    if (player.y > 60) {
        player.x -= landingPlayerSpeed;
        player.y -= 2.0f;
    } else {
        player.y = 60;
        if (!playerLanded) {
            playerLanded = true;
            // PlaySound(TEXT("landing.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
        }
        player.x -= 2.0f;
        if (player.x < 100) {
            currentState = CONGRATULATION;
            cinematicTimer = 0;
            glutTimerFunc(16, updateCongratulationCinematic, 0);
            return;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateLandingCinematic, 0);
}

void updateCongratulationCinematic(int value) {
    cinematicTimer++;

    if (cinematicTimer > 300) {
        currentState = INTRO;
        resetGame();
        glutTimerFunc(16, updateGame, 0);
        return;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateCongratulationCinematic, 0);
}

void updateGame(int value) {
    switch (currentState) {
        case INTRO: break;
        case BOMB_DROP_CINEMATIC: updateBombDropCinematic(value); break;
        case TAKEOFF_CINEMATIC: updateTakeoffCinematic(value); break;
        case AERIAL_ENGAGEMENT: updateAerialEngagement(value); break;
        case LANDING_CINEMATIC: updateLandingCinematic(value); break;
        case CONGRATULATION: updateCongratulationCinematic(value); break;
        case GAME_OVER: break;
    }
    glutTimerFunc(16, updateGame, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw dawn sky gradient
    drawSkyGradient();

    glPushMatrix();
    glTranslatef(-cameraOffsetX, -cameraOffsetY, 0);

    switch (currentState) {
        case INTRO:
            glColor3f(1.0f, 1.0f, 1.0f);
            drawCenteredText(HEIGHT / 2 + 80, "OPERATION CRIMSON AIUB", GLUT_BITMAP_TIMES_ROMAN_24);
            drawCenteredText(HEIGHT / 2 + 40, "Defend Shaheen Airbase!");
            drawCenteredText(HEIGHT / 2 - 20, "Press ENTER to begin mission...");
            drawF16(WIDTH / 2 - 30, HEIGHT / 2 - 150, 1.5f);
            break;

        case BOMB_DROP_CINEMATIC:
            drawRunway();
            drawMig21(indianJetX, indianJetY, 0.8f);
            if (bombDropped) drawBomb(bombX, bombY);
            if (explosionSize > 0) drawExplosion(bombTargetX, bombTargetY + 50, explosionSize);
            break;

        case TAKEOFF_CINEMATIC:
            drawRunway();
            drawF16(player.x, player.y);
            drawF16(WIDTH + 400, 50, 0.8f);
            drawMig21(WIDTH + 200, 60, 0.7f);
            break;

        case AERIAL_ENGAGEMENT:
            glColor3f(0.2f, 0.5f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(0, 0); glVertex2f(WIDTH, 0); glVertex2f(WIDTH, HEIGHT); glVertex2f(0, HEIGHT);
            glEnd();
            drawF16(player.x, player.y);
            for (auto& bullet : player.bullets) drawBullet(bullet.first, bullet.second);
            for (auto& enemy : enemies) {
                if (enemy.alive) drawMig21(enemy.x, enemy.y);
                else drawExplosion(enemy.x, enemy.y, 30);
            }
            glColor3f(1.0f, 1.0f, 1.0f);
            char scoreText[50];
            sprintf(scoreText, "Score: %d", score);
            drawText(20, HEIGHT - 30, scoreText);
            char enemiesText[50];
            sprintf(enemiesText, "Enemies: %d / 2", enemiesDestroyedInGame);
            drawText(WIDTH - 150, HEIGHT - 30, enemiesText);
            break;

        case LANDING_CINEMATIC:
            drawRunway();
            drawF16(player.x, player.y);
            break;

        case CONGRATULATION:
            drawRunway();
            drawF16(player.x, player.y);
            // More personnel for celebration
            for (int i = -3; i <= 3; i++) {
                drawArmyPersonnel(player.x + i * 40, 60, 0.8f);
            }
            glColor3f(0.0f, 1.0f, 0.0f);
            drawCenteredText(HEIGHT / 2 + 50, "MISSION ACCOMPLISHED!", GLUT_BITMAP_TIMES_ROMAN_24);
            drawCenteredText(HEIGHT / 2 + 20, "Shaheen Airbase is safe! Well done, Crimson One!");
            char finalScoreText[50];
            sprintf(finalScoreText, "Final Score: %d", score);
            drawCenteredText(HEIGHT / 2 - 10, finalScoreText);
            drawCenteredText(HEIGHT / 2 - 70, "Press R to restart");
            break;

        case GAME_OVER:
            glColor3f(1.0f, 0.0f, 0.0f);
            drawCenteredText(HEIGHT / 2, "MISSION FAILED", GLUT_BITMAP_TIMES_ROMAN_24);
            drawCenteredText(HEIGHT / 2 - 30, "Press R to restart");
            break;
    }

    glPopMatrix();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 13: // Enter
            if (currentState == INTRO) {
                currentState = BOMB_DROP_CINEMATIC;
                cinematicTimer = 0;
                glutTimerFunc(16, updateGame, 0);
            }
            break;
        case 27: // ESC
            exit(0);
            break;
        case 'f':
        case 'F':
            if (currentState == AERIAL_ENGAGEMENT && player.alive && canPlayerFire) {
                player.bullets.emplace_back(player.x + 60, player.y);
                // PlaySound(TEXT("laser.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
            }
            break;
        case 'r':
        case 'R':
            if (currentState == CONGRATULATION || currentState == GAME_OVER) {
                resetGame();
                currentState = INTRO;
                glutPostRedisplay();
            }
            break;
    }
}

void specialKeys(int key, int x, int y) {
    if (currentState != AERIAL_ENGAGEMENT) return;

    float moveAmount = 8.0f;
    switch (key) {
        case GLUT_KEY_UP:
            if (player.y < HEIGHT - 50) player.y += moveAmount;
            break;
        case GLUT_KEY_DOWN:
            if (player.y > 50) player.y -= moveAmount;
            break;
        case GLUT_KEY_LEFT:
            if (player.x > 0) player.x -= moveAmount;
            break;
        case GLUT_KEY_RIGHT:
            if (player.x < WIDTH - 100) player.x += moveAmount;
            break;
    }
    glutPostRedisplay();
}

bool checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    bool overlapX = (x1 < x2 + w2) && (x1 + w1 > x2);
    bool overlapY = (y1 < y2 + h2) && (y1 + h1 > y2);
    return overlapX && overlapY;
}

void resetGame() {
    cinematicTimer = 0;
    score = 0;
    indianJetX = WIDTH + 100;
    indianJetY = HEIGHT * 0.7f;
    bombDropped = false;
    explosionSize = 0.0f;
    buildingDestroyed = false;
    fireSize = 0.0f;
    destroyedBuildingYOffset = 0.0f;
    canPlayerFire = false;
    enemiesDestroyedInGame = 0;
    landingPlayerSpeed = 5.0f;
    playerLanded = false;
    congratulationTimer = 0;
    player.x = 0;
    player.y = 0;
    player.speedX = 0; player.speedY = 0;
    player.alive = true;
    player.bullets.clear();
    enemies.clear();
    cameraOffsetX = 0; cameraOffsetY = 0;
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Operation Crimson AIUB");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, updateGame, 0);
    glutMainLoop();
    return 0;
}
