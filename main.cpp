#include <windows.h>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>
#include <string>

using namespace std;

// Game states
enum GameState { INTRO, TAKEOFF_CINEMATIC, MISSION_BRIEFING, GAMEPLAY, GAME_OVER };
GameState currentState = INTRO;
int cinematicTimer = 0;
int dayNightTimer = 0;
int dayPhase = 0; // 0=dawn, 1=day, 2=afternoon, 3=night
bool enemiesCanSpawn = false;
float cameraOffsetX = 0;
float cameraOffsetY = 0;
vector<pair<float,float>> enemyBullets;

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
    int type; // 0=MiG-21, 1=Helicopter, 2=Bomber
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

    // Fuselage (Main body - streamlined look)
    glColor3f(0.4f, 0.4f, 0.4f); // Gunmetal gray
    glBegin(GL_POLYGON);
    glVertex2f(-30, 0);   // Tail base
    glVertex2f(60, 6);    // Nose top
    glVertex2f(60, -6);   // Nose bottom
    glVertex2f(-30, 0);   // Back to tail
    glEnd();

    // Vertical stabilizer (tail fin)
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-15, 0);
    glVertex2f(-5, 0);
    glVertex2f(-10, 15);
    glEnd();

    // Cockpit canopy (smooth dome)
    glColor3f(0.0f, 0.2f, 0.6f);
    drawCircle(40, 0, 6, 30); // More vertices for smoother circle

    // Main wings
    glColor3f(0.6f, 0.6f, 0.6f); // Slight contrast
    glBegin(GL_POLYGON); // Left wing
    glVertex2f(10, 0);
    glVertex2f(35, 0);
    glVertex2f(20, 20);
    glVertex2f(-5, 18);
    glEnd();

    glBegin(GL_POLYGON); // Right wing
    glVertex2f(10, 0);
    glVertex2f(35, 0);
    glVertex2f(20, -20);
    glVertex2f(-5, -18);
    glEnd();

    // Tail wings (horizontal stabilizers)
    glBegin(GL_POLYGON); // Left tail wing
    glVertex2f(-20, 0);
    glVertex2f(-10, 0);
    glVertex2f(-5, 10);
    glVertex2f(-15, 10);
    glEnd();

    glBegin(GL_POLYGON); // Right tail wing
    glVertex2f(-20, 0);
    glVertex2f(-10, 0);
    glVertex2f(-5, -10);
    glVertex2f(-15, -10);
    glEnd();

    // Nose cone
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(60, 0);
    glVertex2f(70, 3);
    glVertex2f(70, -3);
    glEnd();

    // Engine exhaust
    glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(-30, 0, 3, 20);

    // Afterburner effect (sparks only when active or random flare)
    if (currentState == TAKEOFF_CINEMATIC || rand() % 10 > 5) {
        glColor3f(1.0f, 0.6f, 0.0f); // Orange flame
        glBegin(GL_TRIANGLES);
        glVertex2f(-30, 2);
        glVertex2f(-45, 0);
        glVertex2f(-30, -2);
        glEnd();

        glColor3f(1.0f, 1.0f, 0.0f); // Yellow inner flame
        glBegin(GL_TRIANGLES);
        glVertex2f(-30, 1);
        glVertex2f(-40, 0);
        glVertex2f(-30, -1);
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
    glVertex2f(0, 0);
    glVertex2f(50, 7);
    glVertex2f(50, -7);
    glEnd();

    // Cockpit
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(40, 0, 5, 20);

    // Wings
    glColor3f(0.2f, 0.4f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(10, 5);
    glVertex2f(35, 5);
    glVertex2f(25, 20);
    glVertex2f(5, 20);
    glVertex2f(10, -5);
    glVertex2f(35, -5);
    glVertex2f(25, -20);
    glVertex2f(5, -20);
    glVertex2f(40, 3);
    glVertex2f(50, 3);
    glVertex2f(50, -3);
    glVertex2f(40, -3);
    glEnd();

    // Exhaust
    glColor3f(0.8f, 0.2f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 3);
    glVertex2f(-10, 0);
    glVertex2f(0, -3);
    glEnd();

    glPopMatrix();
}

void drawBomber(float x, float y, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // Main body
    glColor3f(0.3f, 0.3f, 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(100, 0);
    glVertex2f(90, 15);
    glVertex2f(10, 15);
    glEnd();

    // Cockpit
    glColor3f(0.1f, 0.1f, 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(80, 0);
    glVertex2f(100, 0);
    glVertex2f(90, 15);
    glVertex2f(80, 15);
    glEnd();

    // Wings
    glColor3f(0.2f, 0.2f, 0.3f);
    glBegin(GL_QUADS);
    // Right wing
    glVertex2f(20, 0);
    glVertex2f(60, 0);
    glVertex2f(40, -40);
    glVertex2f(30, -40);
    // Left wing
    glVertex2f(20, 15);
    glVertex2f(60, 15);
    glVertex2f(40, 55);
    glVertex2f(30, 55);
    glEnd();

    // Engines
    glColor3f(0.1f, 0.1f, 0.1f);
    for (int i = 0; i < 4; i++) {
        drawCircle(20 + i * 20, -5, 8);
    }

    glPopMatrix();
}

void drawBullet(float x, float y) {
    glColor3f(1.0f, 1.0f, 0.0f);
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


void drawRunway() {
    // Runway - now grounded at bottom of screen
    glColor3f(0.3f, 0.3f, 0.3f);  // darker gray for contrast
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH * 2, 0);
    glVertex2f(WIDTH * 2, 60);  // shorter runway height
    glVertex2f(0, 60);
    glEnd();

    // Runway markings
    glColor3f(1.0f, 1.0f, 1.0f);
    for(int i = 0; i < WIDTH * 2; i += 100) {
        glBegin(GL_QUADS);
        glVertex2f(i, 30);
        glVertex2f(i + 50, 30);
        glVertex2f(i + 50, 25);
        glVertex2f(i, 25);
        glEnd();
    }
drawAirDefenseSystemTruck(300, 50); // X position, Y = top of runway
drawAirDefenseSystemTruck(600, 50); // Another one



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

void spawnEnemy() {
    if (!enemiesCanSpawn || enemies.size() >= 5) return;

    if (rand() % 100 < 3) {
        Enemy e;
        // Spawn ahead of current camera view:
        e.x = cameraOffsetX + WIDTH + (rand() % 300 + 50);
        e.y = 100 + rand() % (HEIGHT - 200);
        e.speed = 2.0f + (rand() % 30) / 10.0f;
        e.alive = true;

        // Random enemy type
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
        moon.x = WIDTH/2 - WIDTH * 0.4f * sin(cycleProgress * 3.14159f);
        moon.y = HEIGHT + 100;
    } else if (dayPhase == 1) { // Day
        sun.x = WIDTH/2 + WIDTH * 0.4f * cos(cycleProgress * 3.14159f);
        sun.y = HEIGHT * 0.2f + HEIGHT * 0.6f * (1 - cycleProgress);
        moon.x = WIDTH/2 - WIDTH * 0.4f * cos(cycleProgress * 3.14159f);
        moon.y = HEIGHT + 100;
    } else if (dayPhase == 2) { // Afternoon
        sun.x = WIDTH/2 - WIDTH * 0.4f * sin(cycleProgress * 3.14159f);
        sun.y = HEIGHT * 0.2f + HEIGHT * 0.6f * cycleProgress;
        moon.x = WIDTH/2 + WIDTH * 0.4f * sin(cycleProgress * 3.14159f);
        moon.y = HEIGHT - HEIGHT * 0.8f * (1 - cycleProgress);
    } else { // Night
        sun.x = WIDTH/2 - WIDTH * 0.4f * cos(cycleProgress * 3.14159f);
        sun.y = HEIGHT + 100;
        moon.x = WIDTH/2 + WIDTH * 0.4f * cos(cycleProgress * 3.14159f);
        moon.y = HEIGHT * 0.2f + HEIGHT * 0.6f * (1 - cycleProgress);
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
        }
    } else if (currentState == MISSION_BRIEFING) {
        if (cinematicTimer > 180) { // 3 seconds
            currentState = GAMEPLAY;
            enemiesCanSpawn = true;
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


    // Update player bullets
    for (auto& bullet : player.bullets) {
        bullet.first += 10.0f;
    }
    player.bullets.erase(remove_if(player.bullets.begin(), player.bullets.end(),
        [](const pair<float, float>& b) { return b.first > WIDTH; }), player.bullets.end());

    // Update enemies
    for (auto& enemy : enemies) {
        enemy.x -= enemy.speed;
    }
    enemies.erase(remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.x < -100 || !e.alive; }), enemies.end());

    // Spawn new enemies
    spawnEnemy();

    glutPostRedisplay();
    glutTimerFunc(16, updateGame, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 13: // Enter key
            if (currentState == INTRO) {
                currentState = TAKEOFF_CINEMATIC;
                cinematicTimer = 0;

                // Initialize player for takeoff
                player.x = -100;
                player.y = 50;
                player.speed = 5.0f;
                player.alive = true;
            }
            break;

        case 27: // ESC key
            exit(0);
            break;

        case 'f':
case 'F':
    if (currentState == GAMEPLAY && player.alive) {
        player.bullets.emplace_back(player.x + 60, player.y);
    }
    break;


        case 'r':
        case 'R':
            if (currentState == GAME_OVER) {
                // Reset game
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
    }
}

void specialKeys(int key, int x, int y) {
    if (currentState != GAMEPLAY) return;

    float moveAmount = 5.0f;
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
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    radarAngle += 1.5f;
if (radarAngle >= 360.0f)
    radarAngle -= 360.0f;

    // Draw sky
    drawSkyGradient();

    // Draw celestial bodies
    if (dayPhase != 3) drawCelestialBody(sun); // Draw sun during day
    if (dayPhase != 1) drawCelestialBody(moon); // Draw moon at night

    // Draw clouds
    for (auto& cloud : clouds) {
        drawCloud(cloud.x, cloud.y, 40);
    }

    // Draw rain
    if (isRaining) {
        for (auto& drop : raindrops) {
            drawRaindrop(drop.x, drop.y);
        }
    }

    glPushMatrix();
    glTranslatef(-cameraOffsetX, -cameraOffsetY, 0);

    if (currentState == TAKEOFF_CINEMATIC) {
        // Draw extended runway for takeoff scene
        drawRunway();

        // Draw player plane
        drawF16(player.x, player.y);

        // Draw some static planes on runway
        drawF16(WIDTH + 400, 50, 0.8f);
        drawMig21(WIDTH + 200, 60, 0.7f);
    } else if (currentState == MISSION_BRIEFING) {
        // Mission briefing text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(HEIGHT/2 + 60, "MISSION BRIEFING", GLUT_BITMAP_TIMES_ROMAN_24);
        drawCenteredText(HEIGHT/2 + 30, "Objective: Shoot down enemy aircraft");
        drawCenteredText(HEIGHT/2, "Enemy forces are approaching our airspace");
        drawCenteredText(HEIGHT/2 - 30, "Defend our territory at all costs!");
        drawCenteredText(HEIGHT/2 - 90, "Controls:");
        drawCenteredText(HEIGHT/2 - 120, "Arrow Keys - Move");
        drawCenteredText(HEIGHT/2 - 150, "Space - Fire");
    } else if (currentState == GAMEPLAY || currentState == GAME_OVER) {
        // Draw player
        drawF16(player.x, player.y);

        // Draw bullets
        for (auto& bullet : player.bullets) {
            drawBullet(bullet.first, bullet.second);
        }

        // Draw enemies
        for (auto& enemy : enemies) {
            if (enemy.alive) {
                if (enemy.type == 0) drawMig21(enemy.x, enemy.y);
                else if (enemy.type == 1) drawBomber(enemy.x, enemy.y);
            }
        }

        // Draw score
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(20, HEIGHT - 30, scoreText);

        // Draw time of day
        const char* timeText = "";
        switch(dayPhase) {
            case 0: timeText = "Dawn"; break;
            case 1: timeText = "Day"; break;
            case 2: timeText = "Afternoon"; break;
            case 3: timeText = "Night"; break;
        }
        drawText(WIDTH - 120, HEIGHT - 30, timeText);

        if (currentState == GAME_OVER) {
            glColor3f(1.0f, 0.0f, 0.0f);
            drawCenteredText(HEIGHT/2, "MISSION FAILED");
            drawCenteredText(HEIGHT/2 - 30, "Press R to restart");
        }
    } else { // INTRO
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(HEIGHT/2 + 60, "TOP GUN: AIR DEFENSE", GLUT_BITMAP_TIMES_ROMAN_24);
        drawCenteredText(HEIGHT/2 + 20, "You are the last line of defense against enemy air forces");
        drawCenteredText(HEIGHT/2 - 20, "Your mission is to shoot down as many enemy aircraft as possible");
        drawCenteredText(HEIGHT/2 - 80, "Press ENTER to begin mission");

        // Draw a plane silhouette for decoration
        drawF16(WIDTH/2 - 30, HEIGHT/2 - 150, 1.5f);
    }

    glPopMatrix();
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
    glutSpecialFunc(specialKeys);
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
