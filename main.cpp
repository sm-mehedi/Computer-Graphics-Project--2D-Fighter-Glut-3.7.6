#include <windows.h>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

// Game states
enum GameState { INTRO = 0, TAKEOFF_CINEMATIC = 1, MISSION_BRIEFING = 2, GAMEPLAY = 3, GAME_OVER = 4 };
GameState currentState = INTRO;
int cinematicTimer = 0;
int dayNightTimer = 0;
int dayPhase = 0; // 0=dawn, 1=day, 2=afternoon, 3=night
bool enemiesCanSpawn = false;
float cameraOffsetX = 0;
float cameraOffsetY = 0;
vector<pair<float,float>> enemyBullets;
int autoFireTimer = 0;

// Gameplay variables
bool gameOver = false;
int score = 0;
float backgroundOffset = 0.0f;
float radarAngle = 0.0f;

// Screen dimensions
const int WIDTH = 1000;
const int HEIGHT = 600;

// Player fighter jet
struct Fighter {
    float x, y;
    float speed;
    bool alive;
    vector<pair<float, float>> bullets;
} player;

// Enemy planes
struct Enemy {
    float x, y;
    float speed;
    bool alive;
    int type; // 0=MiG-21


};

vector<Enemy> enemies;

// Weather effects
struct WeatherParticle {
    float x, y;
    float speed;
};

vector<WeatherParticle> raindrops;
vector<WeatherParticle> clouds;
bool isRaining = false;

// Celestial bodies
struct CelestialBody {
    float x, y;
    float radius;
    float r, g, b;
} sun, moon;

void playBGM() {
    PlaySound(TEXT("C:/Users/User/OneDrive/8th Semester/Computer Graphics K/Project_Top_Gun/Music/First Flight - 892 - Ace Combat 5 Original Soundtrack.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);

}

void initCelestialBodies() {
    sun.x = WIDTH/2;
    sun.y = HEIGHT + 100;
    sun.radius = 60;
    sun.r = 1.0f; sun.g = 0.9f; sun.b = 0.1f;

    moon.x = WIDTH/2;
    moon.y = HEIGHT + 100;
    moon.radius = 50;
    moon.r = 0.9f; moon.g = 0.9f; moon.b = 1.0f;
}

void drawCircle(float cx, float cy, float r, int segments = 360) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float theta = i * 3.14159f * 2 / segments;
        glVertex2f(cx + r * cos(theta), cy + r * sin(theta));
    }
    glEnd();
}
void drawF16(float x, float y, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // === Fuselage ===
    glColor3f(0.4f, 0.4f, 0.4f); // Gunmetal gray
    glBegin(GL_POLYGON);
    glVertex2f(-35, 0);   // Tail base
    glVertex2f(0, 8);
    glVertex2f(60, 4);    // Nose top
    glVertex2f(70, 0);    // Nose tip
    glVertex2f(60, -4);   // Nose bottom
    glVertex2f(0, -8);
    glVertex2f(-35, 0);   // Back to tail
    glEnd();

    // === Vertical Stabilizer (Tail Fin) ===
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-15, 4);
    glVertex2f(-5, 4);
    glVertex2f(-10, 18);
    glEnd();

    // === Cockpit Canopy ===
    glColor3f(0.0f, 0.2f, 0.6f); // Navy blue
    drawCircle(25, 0, 7, 40);

    // === Air Intake (underbody) ===
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(10, -5);
    glVertex2f(25, -5);
    glVertex2f(20, -12);
    glVertex2f(5, -12);
    glEnd();

    // === Wings ===
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_POLYGON); // Left wing
    glVertex2f(0, 2);
    glVertex2f(25, 2);
    glVertex2f(10, 24);
    glVertex2f(-15, 18);
    glEnd();

    glBegin(GL_POLYGON); // Right wing
    glVertex2f(0, -2);
    glVertex2f(25, -2);
    glVertex2f(10, -24);
    glVertex2f(-15, -18);
    glEnd();

    // === Tail Wings ===
    glBegin(GL_POLYGON); // Left tail wing
    glVertex2f(-30, 2);
    glVertex2f(-20, 2);
    glVertex2f(-10, 12);
    glVertex2f(-20, 12);
    glEnd();

    glBegin(GL_POLYGON); // Right tail wing
    glVertex2f(-30, -2);
    glVertex2f(-20, -2);
    glVertex2f(-10, -12);
    glVertex2f(-20, -12);
    glEnd();

    // === Nose Cone ===
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(70, 0);
    glVertex2f(80, 3);
    glVertex2f(80, -3);
    glEnd();

    // === Engine Exhaust ===
    glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(-35, 0, 3, 20);

    // === Afterburner Effect ===
    if (currentState == TAKEOFF_CINEMATIC || rand() % 10 > 5) {
        glColor3f(1.0f, 0.6f, 0.0f); // Orange outer flame
        glBegin(GL_TRIANGLES);
        glVertex2f(-35, 2);
        glVertex2f(-50, 0);
        glVertex2f(-35, -2);
        glEnd();

        glColor3f(1.0f, 1.0f, 0.0f); // Yellow core
        glBegin(GL_TRIANGLES);
        glVertex2f(-35, 1);
        glVertex2f(-45, 0);
        glVertex2f(-35, -1);
        glEnd();
    }

    glPopMatrix();
}

void drawMig21(float x, float y, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // === Stealth Body (flipped triangle) ===
    glColor3f(0.1f, 0.1f, 0.1f); // stealth black
    glBegin(GL_POLYGON);
    glVertex2f(30, 0);    // rear
    glVertex2f(0, 20);    // top wing
    glVertex2f(-80, 0);   // front nose (left-facing)
    glVertex2f(0, -20);   // bottom wing
    glEnd();

    // === Center fuselage ===
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-10, 3);
    glVertex2f(-70, 3);
    glVertex2f(-70, -3);
    glVertex2f(-10, -3);
    glEnd();

    // === Cockpit Bubble ===
    glColor3f(0.0f, 0.8f, 1.0f);
    drawCircle(-20, 0, 6, 30);

    // === Wing fins (extra side detailing) ===
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-10, 0);
    glVertex2f(10, 15);
    glVertex2f(5, 0);

    glVertex2f(-10, 0);
    glVertex2f(10, -15);
    glVertex2f(5, 0);
    glEnd();

    // === Exhaust ===
    glColor3f(1.0f, 0.4f, 0.0f); // fiery
    glBegin(GL_TRIANGLES);
    glVertex2f(35, 0);
    glVertex2f(30, 5);
    glVertex2f(30, -5);
    glEnd();

    glPopMatrix();
}

void drawBullet(float x, float y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y + 2);
    glVertex2f(x + 10, y + 2);
    glVertex2f(x + 10, y - 2);
    glVertex2f(x, y - 2);
    glEnd();
}


void drawExplosion(float x, float y, float size) {
    glColor3f(1.0f, 0.5f, 0.0f);
    drawCircle(x, y, size);
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCircle(x, y, size * 0.7f);
    glColor3f(1.0f, 0.0f, 0.0f);
    drawCircle(x, y, size * 0.4f);
}
void drawMissile(float x, float y, float height = 50.0f) {
    // Missile body
    glColor3f(0.6f, 0.6f, 0.6f);  // light grey
    glBegin(GL_QUADS);
    glVertex2f(x - 4, y);
    glVertex2f(x + 4, y);
    glVertex2f(x + 4, y + height);
    glVertex2f(x - 4, y + height);
    glEnd();

    // Warhead (tip)
    glColor3f(1.0f, 0.0f, 0.0f);  // red
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 6, y + height);
    glVertex2f(x + 6, y + height);
    glVertex2f(x, y + height + 10);
    glEnd();
}

void drawMissile1(float x, float y, float height = 70.0f) {
    // Missile body
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(x - 5, y);
    glVertex2f(x + 5, y);
    glVertex2f(x + 5, y + height);
    glVertex2f(x - 5, y + height);
    glEnd();

    // Warhead
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 7, y + height);
    glVertex2f(x + 7, y + height);
    glVertex2f(x, y + height + 12);
    glEnd();
}

void drawWheel(float x, float y, float radius = 10.0f) {
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int angle = 0; angle <= 360; angle += 10) {
        float rad = angle * M_PI / 180.0f;
        glVertex2f(x + cos(rad) * radius, y + sin(rad) * radius);
    }
    glEnd();
}

void drawAirDefenseSystemTruck(float baseX, float baseY) {
    // Truck body
    glColor3f(0.25f, 0.25f, 0.25f); // Dark military gray
    glBegin(GL_QUADS);
    glVertex2f(baseX - 60, baseY);
    glVertex2f(baseX + 60, baseY);
    glVertex2f(baseX + 60, baseY + 30);
    glVertex2f(baseX - 60, baseY + 30);
    glEnd();

    // Cabin
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(baseX - 60, baseY + 30);
    glVertex2f(baseX - 30, baseY + 30);
    glVertex2f(baseX - 30, baseY + 55);
    glVertex2f(baseX - 60, baseY + 55);
    glEnd();

    // Wheels
    drawWheel(baseX - 45, baseY);
    drawWheel(baseX,      baseY);
    drawWheel(baseX + 45, baseY);

    // Launcher base (on truck)
    glColor3f(0.18f, 0.18f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(baseX - 30, baseY + 30);
    glVertex2f(baseX + 30, baseY + 30);
    glVertex2f(baseX + 30, baseY + 38);
    glVertex2f(baseX - 30, baseY + 38);
    glEnd();

    // Missiles
    drawMissile1(baseX - 20, baseY + 38);
    drawMissile1(baseX,      baseY + 38);
    drawMissile1(baseX + 20, baseY + 38);
}
void drawMountains(float baseY) {
    glColor3f(0.4f, 0.4f, 0.45f);

    glBegin(GL_TRIANGLES);
    // Peak 1
    glVertex2f(0, baseY);
    glVertex2f(150, baseY + 200);
    glVertex2f(300, baseY);

    // Peak 2
    glVertex2f(250, baseY);
    glVertex2f(450, baseY + 260);
    glVertex2f(650, baseY);

    // Peak 3
    glVertex2f(600, baseY);
    glVertex2f(800, baseY + 220);
    glVertex2f(1000, baseY);

    // Peak 4
    glVertex2f(950, baseY);
    glVertex2f(1150, baseY + 250);
    glVertex2f(1350, baseY);

    // Peak 5
    glVertex2f(1300, baseY);
    glVertex2f(1500, baseY + 230);
    glVertex2f(1700, baseY);

    // Peak 6
    glVertex2f(1650, baseY);
    glVertex2f(1850, baseY + 210);
    glVertex2f(2050, baseY);

    // Peak 7
    glVertex2f(2000, baseY);
    glVertex2f(2200, baseY + 240);
    glVertex2f(2400, baseY);

    // Peak 8
    glVertex2f(2350, baseY);
    glVertex2f(2550, baseY + 200);
    glVertex2f(2750, baseY);
    glEnd();

    // Highlights
    glColor3f(0.55f, 0.55f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(150, baseY + 200);
    glVertex2f(175, baseY + 150);
    glVertex2f(200, baseY + 200);

    glVertex2f(450, baseY + 260);
    glVertex2f(475, baseY + 200);
    glVertex2f(500, baseY + 260);

    glVertex2f(800, baseY + 220);
    glVertex2f(825, baseY + 160);
    glVertex2f(850, baseY + 220);

    glVertex2f(1150, baseY + 250);
    glVertex2f(1175, baseY + 190);
    glVertex2f(1200, baseY + 250);

    glVertex2f(1500, baseY + 230);
    glVertex2f(1525, baseY + 170);
    glVertex2f(1550, baseY + 230);

    glVertex2f(1850, baseY + 210);
    glVertex2f(1875, baseY + 160);
    glVertex2f(1900, baseY + 210);

    glVertex2f(2200, baseY + 240);
    glVertex2f(2225, baseY + 180);
    glVertex2f(2250, baseY + 240);

    glVertex2f(2550, baseY + 200);
    glVertex2f(2575, baseY + 150);
    glVertex2f(2600, baseY + 200);
    glEnd();
}

void drawRunway() {

drawMountains(60);
drawAirDefenseSystemTruck(300, 50); // X position, Y = top of runway
drawAirDefenseSystemTruck(600, 50);
drawAirDefenseSystemTruck(900, 50);


   glColor3f(0.3f, 0.3f, 0.3f);
glBegin(GL_QUADS);
glVertex2f(0, 0);
glVertex2f(WIDTH * 10, 0);
glVertex2f(WIDTH * 10, 60);
glVertex2f(0, 60);
glEnd();




 glColor3f(0.3f, 0.3f, 0.3f);  // darker gray for contrast
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH * 2, 0);
    glVertex2f(WIDTH * 2, 60);
    glVertex2f(0, 60);
    glEnd();

    // Flagpole (thin vertical pole)
glColor3f(0.3f, 0.3f, 0.3f); // dark gray pole
float poleX = WIDTH + 170;    // moved further left
float poleBaseY = 60;
float poleHeight = 120;

glBegin(GL_LINES);
glVertex2f(poleX, poleBaseY);
glVertex2f(poleX, poleBaseY + poleHeight);
glEnd();

// Flag rectangle (green background)
float flagWidth = 60;
float flagHeight = 40;
glColor3f(0.0f, 0.6f, 0.0f);  // Bangladesh green
glBegin(GL_QUADS);
glVertex2f(poleX, poleBaseY + poleHeight - flagHeight);
glVertex2f(poleX + flagWidth, poleBaseY + poleHeight - flagHeight);
glVertex2f(poleX + flagWidth, poleBaseY + poleHeight);
glVertex2f(poleX, poleBaseY + poleHeight);
glEnd();

// Red circle in flag (slightly off-center toward pole side)
float circleCenterX = poleX + flagWidth * 0.35f;
float circleCenterY = poleBaseY + poleHeight - flagHeight / 2;
float circleRadius = flagHeight * 0.25f;

glColor3f(0.8f, 0.0f, 0.0f);  // Bangladesh red
glBegin(GL_TRIANGLE_FAN);
glVertex2f(circleCenterX, circleCenterY);
int numSegments = 40;
for (int i = 0; i <= numSegments; i++) {
    float angle = i * 2.0f * M_PI / numSegments;
    float x = circleCenterX + cos(angle) * circleRadius;
    float y = circleCenterY + sin(angle) * circleRadius;
    glVertex2f(x, y);
}
glEnd();

// July Monument - Symbol of Freedom
float monumentBaseX = WIDTH + 100;  // before the flagpole
float monumentBaseY = 60;
float monumentHeight = 100;

// Monument pedestal
glColor3f(0.25f, 0.25f, 0.25f);  // dark stone
glBegin(GL_QUADS);
glVertex2f(monumentBaseX - 10, monumentBaseY);
glVertex2f(monumentBaseX + 10, monumentBaseY);
glVertex2f(monumentBaseX + 10, monumentBaseY + 20);
glVertex2f(monumentBaseX - 10, monumentBaseY + 20);
glEnd();

// Flame base (rising upward shape - triangle)
glColor3f(0.8f, 0.0f, 0.0f);  // deep red
glBegin(GL_TRIANGLES);
glVertex2f(monumentBaseX - 8, monumentBaseY + 20);
glVertex2f(monumentBaseX + 8, monumentBaseY + 20);
glVertex2f(monumentBaseX, monumentBaseY + 40);
glEnd();

// Inner flame (orange)
glColor3f(1.0f, 0.5f, 0.0f);
glBegin(GL_TRIANGLES);
glVertex2f(monumentBaseX - 5, monumentBaseY + 30);
glVertex2f(monumentBaseX + 5, monumentBaseY + 30);
glVertex2f(monumentBaseX, monumentBaseY + 50);
glEnd();

// Flame tip (yellow)
glColor3f(1.0f, 1.0f, 0.0f);
glBegin(GL_TRIANGLES);
glVertex2f(monumentBaseX - 2, monumentBaseY + 45);
glVertex2f(monumentBaseX + 2, monumentBaseY + 45);
glVertex2f(monumentBaseX, monumentBaseY + monumentHeight);
glEnd();


// Airport terminal building (wider and taller)
glColor3f(0.45f, 0.45f, 0.45f);  // mid gray
glBegin(GL_QUADS);
glVertex2f(WIDTH + 250, 60);
glVertex2f(WIDTH + 650, 60);
glVertex2f(WIDTH + 650, 220);  // taller
glVertex2f(WIDTH + 250, 220);
glEnd();

// Glass panel effect (optional horizontal lines)
glColor3f(0.7f, 0.85f, 0.95f);  // soft glass blue
for (int i = 0; i < 5; ++i) {
    glBegin(GL_LINES);
    glVertex2f(WIDTH + 250, 80 + i * 28);
    glVertex2f(WIDTH + 650, 80 + i * 28);
    glEnd();
}

// Control tower (taller and more defined)
glColor3f(0.6f, 0.6f, 0.6f);  // light gray
glBegin(GL_QUADS);
glVertex2f(WIDTH + 500, 220);
glVertex2f(WIDTH + 560, 220);
glVertex2f(WIDTH + 560, 340);  // extended height
glVertex2f(WIDTH + 500, 340);
glEnd();

// Tower cabin
glColor3f(0.4f, 0.4f, 0.5f);  // darker top cabin
glBegin(GL_QUADS);
glVertex2f(WIDTH + 490, 340);
glVertex2f(WIDTH + 570, 340);
glVertex2f(WIDTH + 570, 370);
glVertex2f(WIDTH + 490, 370);
glEnd();
// Rotating radar on top of the tower
float radarCenterX = WIDTH + 530;
float radarCenterY = 390;
float radarLength = 50.0f;  // Increased length

// Calculate end point of rotating line
float endX = radarCenterX + radarLength * cos(radarAngle * M_PI / 180.0f);
float endY = radarCenterY + radarLength * sin(radarAngle * M_PI / 180.0f);

// Radar base (larger circular hub)
glColor3f(0.6f, 0.6f, 0.6f);
glBegin(GL_POLYGON);
for (int i = 0; i < 24; ++i) {
    float theta = i * 2.0f * M_PI / 24;
    glVertex2f(radarCenterX + 8 * cos(theta), radarCenterY + 8 * sin(theta)); // Bigger circle
}
glEnd();


// Rotating sweep line
glColor3f(0.0f, 1.0f, 0.0f);  // green
glBegin(GL_LINES);
glVertex2f(radarCenterX, radarCenterY);
glVertex2f(endX, endY);
glEnd();

// Wide Air Force building
float buildingX = WIDTH + 660;
float buildingY = 60;
float buildingWidth = 400;  // much wider
float buildingHeight = 160;  // same height as terminal
float panelWidth = 20;
float roofHeight = 40;

// Building main body - mid gray
glColor3f(0.45f, 0.45f, 0.45f);
glBegin(GL_QUADS);
glVertex2f(buildingX, buildingY);
glVertex2f(buildingX + buildingWidth, buildingY);
glVertex2f(buildingX + buildingWidth, buildingY + buildingHeight);
glVertex2f(buildingX, buildingY + buildingHeight);
glEnd();

// Roof - dark gray slanted flat roof
glColor3f(0.35f, 0.35f, 0.35f);
glBegin(GL_QUADS);
glVertex2f(buildingX - 20, buildingY + buildingHeight);
glVertex2f(buildingX + buildingWidth + 20, buildingY + buildingHeight);
glVertex2f(buildingX + buildingWidth + 20, buildingY + buildingHeight + roofHeight);
glVertex2f(buildingX - 20, buildingY + buildingHeight + roofHeight);
glEnd();

// Vertical panel lines for realism
glColor3f(0.55f, 0.55f, 0.55f);
for (float x = buildingX + panelWidth; x < buildingX + buildingWidth; x += panelWidth) {
    glBegin(GL_LINES);
    glVertex2f(x, buildingY);
    glVertex2f(x, buildingY + buildingHeight);
    glEnd();
}

// Horizontal window bands (optional glass effect)
glColor3f(0.7f, 0.85f, 0.95f);
for (int i = 1; i <= 3; ++i) {
    float y = buildingY + i * (buildingHeight / 4);
    glBegin(GL_LINES);
    glVertex2f(buildingX, y);
    glVertex2f(buildingX + buildingWidth, y);
    glEnd();
}

// Text label "BANGLADESH AIRFORCE" centered on building front
glColor3f(1.0f, 1.0f, 1.0f);
const char* label = "BANGLADESH AIRFORCE";
float textX = buildingX + buildingWidth / 2.0f - (strlen(label) * 9) / 2;  // approx width per char
float textY = buildingY + buildingHeight / 2 + 10;

glRasterPos2f(textX, textY);
for (const char* c = label; *c != '\0'; ++c) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}


// Glass panel effect for the new building (optional horizontal lines)
glColor3f(0.7f, 0.85f, 0.95f);  // soft glass blue
for (int i = 0; i < 5; ++i) {
    glBegin(GL_LINES);
    glVertex2f(WIDTH + 660, 80 + i * 28);
    glVertex2f(WIDTH + 1060, 80 + i * 28);
    glEnd();
}


    // Background trees
    for (int i = 0; i < WIDTH * 2; i += 100) {
    float trunkX = i + 40;
    float baseY = 60;

    // Trunk
    glColor3f(0.55f, 0.27f, 0.07f);
    glBegin(GL_QUADS);
    glVertex2f(trunkX, baseY);
    glVertex2f(trunkX + 10, baseY);
    glVertex2f(trunkX + 10, baseY + 30);
    glVertex2f(trunkX, baseY + 30);
    glEnd();

    // Christmas tree layers (3 triangles)
    glColor3f(0.0f, 0.6f, 0.0f);

    // Bottom layer
    glBegin(GL_TRIANGLES);
    glVertex2f(trunkX - 20, baseY + 30);
    glVertex2f(trunkX + 30, baseY + 30);
    glVertex2f(trunkX + 5, baseY + 60);
    glEnd();

    // Middle layer
    glBegin(GL_TRIANGLES);
    glVertex2f(trunkX - 15, baseY + 50);
    glVertex2f(trunkX + 25, baseY + 50);
    glVertex2f(trunkX + 5, baseY + 75);
    glEnd();

    // Top layer
    glBegin(GL_TRIANGLES);
    glVertex2f(trunkX - 10, baseY + 68);
    glVertex2f(trunkX + 20, baseY + 68);
    glVertex2f(trunkX + 5, baseY + 90);
    glEnd();
}


}

void drawCloud(float x, float y, float size) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
    drawCircle(x, y, size);
    drawCircle(x + size*0.7f, y, size*0.8f);
    drawCircle(x - size*0.5f, y, size*0.6f);
    drawCircle(x + size*0.3f, y + size*0.4f, size*0.7f);
}

void drawRaindrop(float x, float y) {
    glColor4f(0.5f, 0.5f, 1.0f, 0.7f);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x - 3, y + 10);
    glEnd();
}

void drawCelestialBody(const CelestialBody& body) {
    glColor3f(body.r, body.g, body.b);
    drawCircle(body.x, body.y, body.radius);

    // Add glow effect
    for (int i = 1; i <= 3; i++) {
        glColor4f(body.r, body.g, body.b, 0.3f/i);
        drawCircle(body.x, body.y, body.radius * (1.0f + 0.2f * i));
    }
}

void drawSkyGradient() {
    switch(dayPhase) {
        case 0: // Dawn
            glBegin(GL_QUADS);
            glColor3f(0.3f, 0.4f, 0.8f); // Top
            glVertex2f(0, HEIGHT);
            glVertex2f(WIDTH, HEIGHT);
            glColor3f(1.0f, 0.6f, 0.2f); // Horizon
            glVertex2f(WIDTH, HEIGHT/2);
            glVertex2f(0, HEIGHT/2);
            glColor3f(1.0f, 0.6f, 0.2f); // Horizon
            glVertex2f(0, HEIGHT/2);
            glVertex2f(WIDTH, HEIGHT/2);
            glColor3f(0.2f, 0.2f, 0.4f); // Bottom
            glVertex2f(WIDTH, 0);
            glVertex2f(0, 0);
            glEnd();
            break;

        case 1: // Day
            glBegin(GL_QUADS);
            glColor3f(0.2f, 0.5f, 1.0f); // Top
            glVertex2f(0, HEIGHT);
            glVertex2f(WIDTH, HEIGHT);
            glColor3f(0.6f, 0.8f, 1.0f); // Bottom
            glVertex2f(WIDTH, 0);
            glVertex2f(0, 0);
            glEnd();
            break;

        case 2: // Afternoon
            glBegin(GL_QUADS);
            glColor3f(0.8f, 0.4f, 0.1f); // Top
            glVertex2f(0, HEIGHT);
            glVertex2f(WIDTH, HEIGHT);
            glColor3f(1.0f, 0.6f, 0.3f); // Bottom
            glVertex2f(WIDTH, 0);
            glVertex2f(0, 0);
            glEnd();
            break;

        case 3: // Night
            glBegin(GL_QUADS);
            glColor3f(0.0f, 0.0f, 0.1f); // Top
            glVertex2f(0, HEIGHT);
            glVertex2f(WIDTH, HEIGHT);
            glColor3f(0.1f, 0.1f, 0.3f); // Bottom
            glVertex2f(WIDTH, 0);
            glVertex2f(0, 0);
            glEnd();
            break;
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
    drawText(WIDTH/2 - textWidth/2, y, text, font);
}
int gameTimeSeconds = 0;
int difficultyLevel = 1;
int maxEnemies = 3;

void updateDifficulty() {
    difficultyLevel = 1 + gameTimeSeconds / 10;  // Increase every 10s
    if (difficultyLevel > 6) difficultyLevel = 6;

    maxEnemies = 3 + difficultyLevel - 1;  // From 3 to 6 max
}

void spawnEnemy() {
    updateDifficulty();

    if (!enemiesCanSpawn || enemies.size() >= maxEnemies) return;

    // Increase spawn chance with difficulty
    int spawnChance = 2 + difficultyLevel * 2;  // from 4% to 14%
    if (rand() % 100 < spawnChance) {
        Enemy e;
        e.x = cameraOffsetX + WIDTH + (rand() % 300 + 50);
        e.y = 100 + rand() % (HEIGHT - 200);
        e.speed = 2.0f + (rand() % 30) / 10.0f;
        e.alive = true;

        int r = rand() % 10;
        if (r < 6)      e.type = 0;  // MiG-21
        else if (r < 9) e.type = 1;  // Helicopter
        else            e.type = 2;  // Bomber

        enemies.push_back(e);
    }
}


void updateDayNightCycle() {
    dayNightTimer++;
    if (dayNightTimer >= 900) { // 15 seconds per phase (60fps * 15)
        dayNightTimer = 0;
        dayPhase = (dayPhase + 1) % 4;

        // Random weather changes
        isRaining = (rand() % 4 == 0);
        if (isRaining) {
            raindrops.clear();
            for (int i = 0; i < 200; i++) {
                WeatherParticle r;
                r.x = rand() % WIDTH;
                r.y = rand() % HEIGHT;
                r.speed = 3 + rand() % 5;
                raindrops.push_back(r);
            }
        }
    }

    // Update celestial positions
    float cycleProgress = dayNightTimer / 900.0f;
    if (dayPhase == 0) { // Dawn
        sun.x = WIDTH/2 + WIDTH * 0.4f * sin(cycleProgress * 3.14159f);
        sun.y = HEIGHT - HEIGHT * 0.8f * cycleProgress;
        
    } else if (dayPhase == 1) { // Day
        sun.x = WIDTH/2 + WIDTH * 0.4f * cos(cycleProgress * 3.14159f);
        sun.y = HEIGHT * 0.2f + HEIGHT * 0.6f * (1 - cycleProgress);
       
    } else if (dayPhase == 2) { // Afternoon
        sun.x = WIDTH/2 - WIDTH * 0.4f * sin(cycleProgress * 3.14159f);
        sun.y = HEIGHT * 0.2f + HEIGHT * 0.6f * cycleProgress;
        moon.x = WIDTH/2 + WIDTH * 0.4f * sin(cycleProgress * 3.14159f);
        moon.y = HEIGHT - HEIGHT * 0.8f * (1 - cycleProgress);
    } else { // Night
        
        moon.x = WIDTH/2 + WIDTH * 0.4f * cos(cycleProgress * 3.14159f);
        moon.y = HEIGHT * 0.2f + HEIGHT * 0.6f * (1 - cycleProgress);
    }
}
bool briefingAcknowledged = false;



void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 13: // ENTER
            if (currentState == INTRO) {
                currentState = TAKEOFF_CINEMATIC;
                cinematicTimer = 0;
                player.x = -100;
                player.y = 50;
                player.speed = 5.0f;
                player.alive = true;
            } else if (currentState == MISSION_BRIEFING) {
                briefingAcknowledged = true;
            }
            break;

        case 27: // ESC
            exit(0);
            break;

        case ' ': // SPACE = Shoot
           // if (currentState == GAMEPLAY && player.alive) {
           //     player.bullets.emplace_back(player.x + 60, player.y);
           // }
            break;

        case 'r':
        case 'R': // Restart
            if (currentState == GAME_OVER) {
                currentState = INTRO;
                gameOver = false;
                score = 0;
                dayNightTimer = 0;
                dayPhase = 0;
                enemiesCanSpawn = false;
                player.bullets.clear();
                enemies.clear();
                raindrops.clear();
                clouds.clear();
            }
            break;

        // ========================
        // WASD MOVEMENT CONTROLS
        // ========================
        case 'w':
        case 'W':
            if (player.y < HEIGHT - 50) player.y += 5.0f;
            break;
        case 's':
        case 'S':
            if (player.y > 50) player.y -= 5.0f;
            break;
        case 'a':
        case 'A':
            if (player.x > 0) player.x -= 5.0f;
            break;
        case 'd':
        case 'D':
            if (player.x < WIDTH - 100) player.x += 5.0f;
            break;
    }
}

void updateCinematic(int value) {
    cinematicTimer++;

    if (currentState == TAKEOFF_CINEMATIC) {
        // Camera follows player during takeoff
        cameraOffsetX = player.x - 200;
        cameraOffsetY = player.y - 150;

        // Player takes off
        if (cinematicTimer < 60) {
            player.x += 2.0f; // Taxi
        } else if (cinematicTimer < 180) {
            player.x += 5.0f; // Accelerate
        } else if (cinematicTimer < 300) {
            player.x += 8.0f;
            player.y += 2.0f; // Lift off
        } else {
            // Transition to gameplay
            currentState = MISSION_BRIEFING;
            cinematicTimer = 0;
            player.x = 200;  // lock player to fixed screen position
player.y = 300;
        }
    } else if (currentState == MISSION_BRIEFING) {
    if (briefingAcknowledged) {
        currentState = GAMEPLAY;
        enemiesCanSpawn = true;
        cinematicTimer = 0; // optional reset
    } else {
        cinematicTimer++;
    }
}

    glutPostRedisplay();
    glutTimerFunc(16, updateCinematic, 0);
}

void updateGame(int value) {
    if (currentState != GAMEPLAY) {
        glutTimerFunc(16, updateGame, 0);
        return;
    }

    updateDayNightCycle();

    // Update game time and difficulty every second
static int timeAccumulator = 0;
timeAccumulator += 16;

if (timeAccumulator >= 1000) {
    gameTimeSeconds++;
    timeAccumulator = 0;

    // Increase difficulty every 10 seconds, up to level 6 (3 to 6 enemies)
    if (gameTimeSeconds % 10 == 0 && difficultyLevel < 6) {
        difficultyLevel++;
        maxEnemies = 3 + (difficultyLevel - 1); // Level 1 = 3 enemies, Level 6 = 8 enemies
        cout << ">>> Difficulty increased to Level " << difficultyLevel
             << " | Max Enemies: " << maxEnemies << "\n";
    }
}
    // Auto fire bullets every 10 frames
    autoFireTimer++;
    if (autoFireTimer >= 10) {
        if (player.alive) {
            player.bullets.emplace_back(player.x + 60, player.y);
            cout << "Bullet spawned at y = " << player.y << "\n";
        }
        autoFireTimer = 0;
    }

    // === Move player bullets forward ===
    for (auto& bullet : player.bullets) {
        bullet.first += 10.0f;
    }

    // === Check bullet collisions with enemies ===
    for (auto& bullet : player.bullets) {
        for (auto& enemy : enemies) {
            if (enemy.alive) {
                float bulletX = bullet.first;
                float bulletY = bullet.second;
                float enemyX = enemy.x;
                float enemyY = enemy.y;
                float enemyWidth = 60;
                float enemyHeight = 30;

                if (bulletX >= enemyX && bulletX <= enemyX + enemyWidth &&
                    bulletY >= enemyY && bulletY <= enemyY + enemyHeight) {
                    enemy.alive = false;
                    score += 10;
                    bullet.first = WIDTH + 100; // mark for removal
                    cout << "Enemy destroyed at (" << enemyX << ", " << enemyY << ")\n";
                }
            }
        }
    }

    // Remove off-screen bullets
    player.bullets.erase(remove_if(player.bullets.begin(), player.bullets.end(),
        [](const pair<float, float>& b) { return b.first > WIDTH; }), player.bullets.end());

    // === Update enemies ===
    for (auto& enemy : enemies) {
        enemy.x -= enemy.speed;

       // Only shoot if enemy is visible in camera frame
if (enemy.x >= cameraOffsetX && enemy.x <= cameraOffsetX + WIDTH) {
    if (rand() % 100 < 2) {
        enemyBullets.emplace_back(enemy.x - 10, enemy.y);
    }
}

    }

    // === Move enemy bullets ===
    for (auto& b : enemyBullets) {
    b.first -= 6.0f;  // move bullet

    // check for collision
    float bulletX = b.first;
    float bulletY = b.second;
    float playerWidth = 60;
    float playerHeight = 30;

    if (player.alive &&
        bulletX >= player.x && bulletX <= player.x + playerWidth &&
        bulletY >= player.y && bulletY <= player.y + playerHeight) {

        player.alive = false;
        currentState = GAME_OVER;
        cout << "Player hit by enemy bullet at (" << bulletX << ", " << bulletY << ")\n";
        break;
    }
}


    enemyBullets.erase(remove_if(enemyBullets.begin(), enemyBullets.end(),
        [](const pair<float, float>& b) { return b.first < 0; }), enemyBullets.end());

    // === Remove dead or off-screen enemies ===
    enemies.erase(remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.x < -100 || !e.alive; }), enemies.end());

    // Debug info
    if (!player.bullets.empty()) {
        cout << "First bullet position: " << player.bullets.front().first
             << ", " << player.bullets.front().second << "\n";
        cout << "Bullet y: " << player.bullets.front().second
             << ", Player y: " << player.y << "\n";
    }

    spawnEnemy();

    glutPostRedisplay();
    glutTimerFunc(16, updateGame, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    radarAngle += 1.5f;
    if (radarAngle >= 360.0f)
        radarAngle -= 360.0f;

    drawSkyGradient();

    if (dayPhase != 3) drawCelestialBody(sun);
    if (dayPhase != 1) drawCelestialBody(moon);

    for (auto& cloud : clouds) {
        drawCloud(cloud.x, cloud.y, 40);
    }

    if (isRaining) {
        for (auto& drop : raindrops) {
            drawRaindrop(drop.x, drop.y);
        }
    }

    // ========================
    // STATIC SCREENS (UI layer)
    // ========================
    if (currentState == INTRO) {
        glColor3f(0.0f, 0.0f, 0.0f);
        drawCenteredText(HEIGHT / 2 + 60, "TOP GUN: AIR DEFENSE", GLUT_BITMAP_TIMES_ROMAN_24);
        drawCenteredText(HEIGHT / 2 + 20, "You are the last line of defense against enemy air forces");
        drawCenteredText(HEIGHT / 2 - 20, "Your mission is to shoot down as many enemy aircraft as possible");
        drawCenteredText(HEIGHT / 2 - 80, "Press ENTER to begin mission");
        drawF16(WIDTH / 2 - 30, HEIGHT / 2 - 150, 1.5f); // intro plane
    }

    else if (currentState == MISSION_BRIEFING) {
        glColor3f(0.0f, 0.0f, 0.0f);
        drawCenteredText(HEIGHT / 2 + 60, "MISSION BRIEFING", GLUT_BITMAP_TIMES_ROMAN_24);
        drawCenteredText(HEIGHT / 2 + 30, "Objective: Shoot down enemy aircraft");
        drawCenteredText(HEIGHT / 2, "Enemy forces are approaching our airspace");
        drawCenteredText(HEIGHT / 2 - 30, "Defend our territory at all costs!");
        drawCenteredText(HEIGHT / 2 - 90, "Controls:");
        drawCenteredText(HEIGHT / 2 - 120, "WASD - Move");
        drawCenteredText(HEIGHT / 2 - 200, "Press ENTER to begin...");
    }

    // ================================
    // WORLD-SPACE SCENES
    // ================================
    if (currentState == TAKEOFF_CINEMATIC || currentState == GAMEPLAY || currentState == GAME_OVER) {
        glPushMatrix();

        if (currentState == TAKEOFF_CINEMATIC) {


         glPushMatrix();
            glTranslatef(-cameraOffsetX, 0.0f, 0.0f); // runway scrolls horizontally, stays grounded



        drawRunway();
         drawF16(player.x, player.y);
         drawF16(WIDTH + 400, 50, 0.8f);
        drawF16(WIDTH + 200, 60, 0.7f);
         glPopMatrix();

          glPushMatrix();
         glTranslatef(-cameraOffsetX, -cameraOffsetY, 0); // only translate during cinematic
            glPopMatrix();
        }



        if (currentState == GAMEPLAY || currentState == GAME_OVER) {
            drawF16(player.x, player.y);

            for (auto& bullet : player.bullets) {
                drawBullet(bullet.first, bullet.second, 1.0f, 1.0f, 0.0f); // yellow
            }

            for (auto& b : enemyBullets) {
                drawBullet(b.first, b.second, 1.0f, 0.0f, 0.0f); // red
            }

            for (auto& enemy : enemies) {
                if (enemy.alive) {
                    if (enemy.type == 0) drawMig21(enemy.x, enemy.y);

                }
            }

            if (currentState == GAME_OVER) {
                glColor3f(1.0f, 0.0f, 0.0f);
                drawCenteredText(HEIGHT / 2, "MISSION FAILED");
                drawCenteredText(HEIGHT / 2 - 30, "Press R to restart");
            }
        }

        glPopMatrix();
    }

    // ========================
    // UI OVERLAY (always on screen)
    // ========================
 if (currentState == GAMEPLAY || currentState == GAME_OVER) {
    // Score: White
    glColor3f(0.0f, 0.0f, 0.0f);
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    drawText(20, HEIGHT - 30, scoreText);

    // Difficulty: Yellow
    glColor3f(1.0f, 1.0f, 0.0f);
    char difficultyText[50];
    sprintf(difficultyText, "Difficulty: %d", difficultyLevel);
    drawText(WIDTH - 250, HEIGHT - 30, difficultyText);

    // Day phase (Time): Cyan
    glColor3f(0.0f, 1.0f, 1.0f);
    const char* timeText = "";
    switch (dayPhase) {
        case 0: timeText = "Dawn"; break;
        case 1: timeText = "Day"; break;
        case 2: timeText = "Afternoon"; break;
        case 3: timeText = "Night"; break;
    }
    drawText(WIDTH - 120, HEIGHT - 30, timeText);
}


    glutSwapBuffers();
}


int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Top Gun: Air Defense");

    // Initialize celestial bodies
    initCelestialBodies();
    playBGM();

    // Initialize clouds
    for (int i = 0; i < 10; i++) {
        WeatherParticle c;
        c.x = rand() % WIDTH;
        c.y = 200 + rand() % (HEIGHT - 200);
        c.speed = 0.5f + (rand() % 10) / 10.0f;
        clouds.push_back(c);
    }

    // Set up callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glutTimerFunc(0, updateCinematic, 0);
    glutTimerFunc(0, updateGame, 0);

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);

    // Initialize random seed
    srand(time(NULL));



    // Start the main loop
    glutMainLoop();
    return 0;
}
