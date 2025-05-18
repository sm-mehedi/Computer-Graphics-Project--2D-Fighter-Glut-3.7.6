#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <mmsystem.h>
#include <string>

// Global variables
bool scenario1Complete = false;
bool scenario2Complete = false;
bool inCombat = false;
bool gameOver = false;
bool victory = false;

float indianPlaneX = -1000.0f;
float indianPlaneY = 300.0f;
float bangladeshPlaneX = -1500.0f;
float bangladeshPlaneY = 200.0f;
float bombX = 0.0f;
float bombY = 0.0f;
bool bombDropped = false;
bool buildingDestroyed = false;
float buildingAlpha = 1.0f;
float fireScale = 0.0f;

float enemy1X = 800.0f;
float enemy1Y = 350.0f;
float enemy2X = 1000.0f;
float enemy2Y = 250.0f;
bool enemy1Destroyed = false;
bool enemy2Destroyed = false;
float enemy1Alpha = 1.0f;
float enemy2Alpha = 1.0f;

int score = 0;

void drawText(float x, float y, const char* string) {
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

void drawBuilding() {
    // Main building structure
    if (!buildingDestroyed) {
        glColor4f(0.5f, 0.5f, 0.5f, buildingAlpha);
        glBegin(GL_QUADS);
        glVertex2f(100.0f, 100.0f);
        glVertex2f(300.0f, 100.0f);
        glVertex2f(300.0f, 300.0f);
        glVertex2f(100.0f, 300.0f);
        glEnd();

        // Windows
        glColor4f(0.8f, 0.8f, 1.0f, buildingAlpha);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                glBegin(GL_QUADS);
                glVertex2f(120 + i * 40, 120 + j * 30);
                glVertex2f(140 + i * 40, 120 + j * 30);
                glVertex2f(140 + i * 40, 140 + j * 30);
                glVertex2f(120 + i * 40, 140 + j * 30);
                glEnd();
            }
        }
    }

    // Fire effect after destruction
    if (buildingDestroyed && fireScale < 1.5f) {
        fireScale += 0.01f;
        glPushMatrix();
        glTranslatef(200.0f, 100.0f, 0.0f);
        glScalef(fireScale, fireScale, 1.0f);
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
        glVertex2f(0.0f, 0.0f);
        glColor4f(1.0f, 0.5f, 0.0f, 0.6f);
        for (int i = 0; i <= 360; i += 30) {
            float angle = i * 3.14159f / 180.0f;
            glVertex2f(cos(angle) * 100.0f, sin(angle) * 150.0f);
        }
        glEnd();
        glPopMatrix();
    }
}

void drawIndianPlane() {
    if (indianPlaneX < 1200.0f) {
        glPushMatrix();
        glTranslatef(indianPlaneX, indianPlaneY, 0.0f);

        // Plane body
        glColor3f(0.7f, 0.7f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(100.0f, 0.0f);
        glVertex2f(100.0f, 20.0f);
        glVertex2f(0.0f, 20.0f);
        glEnd();

        // Plane nose
        glBegin(GL_TRIANGLES);
        glVertex2f(100.0f, 0.0f);
        glVertex2f(120.0f, 10.0f);
        glVertex2f(100.0f, 20.0f);
        glEnd();

        // Wings
        glBegin(GL_TRIANGLES);
        glVertex2f(30.0f, 0.0f);
        glVertex2f(50.0f, -30.0f);
        glVertex2f(70.0f, 0.0f);

        glVertex2f(30.0f, 20.0f);
        glVertex2f(50.0f, 50.0f);
        glVertex2f(70.0f, 20.0f);
        glEnd();

        // Tail
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 20.0f);
        glVertex2f(-10.0f, 40.0f);
        glVertex2f(0.0f, 40.0f);
        glEnd();

        // Indian flag marker
        glColor3f(0.0f, 0.5f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(10.0f, 30.0f);
        glVertex2f(30.0f, 30.0f);
        glVertex2f(30.0f, 40.0f);
        glVertex2f(10.0f, 40.0f);
        glEnd();

        glColor3f(1.0f, 0.5f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(10.0f, 40.0f);
        glVertex2f(30.0f, 40.0f);
        glVertex2f(30.0f, 50.0f);
        glVertex2f(10.0f, 50.0f);
        glEnd();

        glColor3f(0.8f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(20.0f, 40.0f);
        for (int i = 0; i <= 360; i += 30) {
            float angle = i * 3.14159f / 180.0f;
            glVertex2f(20.0f + cos(angle) * 8.0f, 40.0f + sin(angle) * 8.0f);
        }
        glEnd();

        glPopMatrix();
    }
}

void drawBangladeshPlane() {
    glPushMatrix();
    glTranslatef(bangladeshPlaneX, bangladeshPlaneY, 0.0f);

    // Plane body
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(100.0f, 0.0f);
    glVertex2f(100.0f, 20.0f);
    glVertex2f(0.0f, 20.0f);
    glEnd();

    // Plane nose
    glBegin(GL_TRIANGLES);
    glVertex2f(100.0f, 0.0f);
    glVertex2f(120.0f, 10.0f);
    glVertex2f(100.0f, 20.0f);
    glEnd();

    // Wings
    glBegin(GL_TRIANGLES);
    glVertex2f(30.0f, 0.0f);
    glVertex2f(50.0f, -30.0f);
    glVertex2f(70.0f, 0.0f);

    glVertex2f(30.0f, 20.0f);
    glVertex2f(50.0f, 50.0f);
    glVertex2f(70.0f, 20.0f);
    glEnd();

    // Tail
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 20.0f);
    glVertex2f(-10.0f, 40.0f);
    glVertex2f(0.0f, 40.0f);
    glEnd();

    // Bangladesh flag marker
    glColor3f(0.0f, 0.5f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(10.0f, 30.0f);
    glVertex2f(30.0f, 30.0f);
    glVertex2f(30.0f, 50.0f);
    glVertex2f(10.0f, 50.0f);
    glEnd();

    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(20.0f, 40.0f);
    for (int i = 0; i <= 360; i += 30) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(20.0f + cos(angle) * 8.0f, 40.0f + sin(angle) * 8.0f);
    }
    glEnd();

    glPopMatrix();
}

void drawBomb() {
    if (bombDropped && bombY > 100.0f) {
        glPushMatrix();
        glTranslatef(bombX, bombY, 0.0f);

        // Bomb body
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for (int i = 0; i <= 360; i += 30) {
            float angle = i * 3.14159f / 180.0f;
            glVertex2f(cos(angle) * 10.0f, sin(angle) * 15.0f);
        }
        glEnd();

        // Bomb fin
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, -15.0f);
        glVertex2f(-10.0f, -25.0f);
        glVertex2f(10.0f, -25.0f);
        glEnd();

        glPopMatrix();
    }
}

void drawEnemyPlane(float x, float y, float alpha) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Plane body
    glColor4f(0.7f, 0.7f, 0.7f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(80.0f, 0.0f);
    glVertex2f(80.0f, 15.0f);
    glVertex2f(0.0f, 15.0f);
    glEnd();

    // Plane nose
    glBegin(GL_TRIANGLES);
    glVertex2f(80.0f, 0.0f);
    glVertex2f(95.0f, 7.5f);
    glVertex2f(80.0f, 15.0f);
    glEnd();

    // Wings
    glBegin(GL_TRIANGLES);
    glVertex2f(20.0f, 0.0f);
    glVertex2f(35.0f, -20.0f);
    glVertex2f(50.0f, 0.0f);

    glVertex2f(20.0f, 15.0f);
    glVertex2f(35.0f, 35.0f);
    glVertex2f(50.0f, 15.0f);
    glEnd();

    // Tail
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 15.0f);
    glVertex2f(-8.0f, 30.0f);
    glVertex2f(0.0f, 30.0f);
    glEnd();

    // Indian flag marker
    glColor4f(0.0f, 0.5f, 0.0f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(5.0f, 25.0f);
    glVertex2f(20.0f, 25.0f);
    glVertex2f(20.0f, 30.0f);
    glVertex2f(5.0f, 30.0f);
    glEnd();

    glColor4f(1.0f, 0.5f, 0.0f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(5.0f, 30.0f);
    glVertex2f(20.0f, 30.0f);
    glVertex2f(20.0f, 35.0f);
    glVertex2f(5.0f, 35.0f);
    glEnd();

    glColor4f(0.8f, 0.0f, 0.0f, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(12.5f, 30.0f);
    for (int i = 0; i <= 360; i += 30) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(12.5f + cos(angle) * 5.0f, 30.0f + sin(angle) * 5.0f);
    }
    glEnd();

    glPopMatrix();
}

void drawRunway() {
    // Runway base
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(-1500.0f, 50.0f);
    glVertex2f(1500.0f, 50.0f);
    glVertex2f(1500.0f, 150.0f);
    glVertex2f(-1500.0f, 150.0f);
    glEnd();

    // Runway markings
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = -1400; i < 1400; i += 100) {
        glBegin(GL_QUADS);
        glVertex2f(i, 95.0f);
        glVertex2f(i + 50.0f, 95.0f);
        glVertex2f(i + 50.0f, 105.0f);
        glVertex2f(i, 105.0f);
        glEnd();
    }
}

void drawArmyBase() {
    // Base perimeter
    glColor3f(0.0f, 0.3f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-400.0f, 50.0f);
    glVertex2f(400.0f, 50.0f);
    glVertex2f(400.0f, 400.0f);
    glVertex2f(-400.0f, 400.0f);
    glEnd();

    // Barracks
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(-300.0f, 100.0f);
    glVertex2f(-100.0f, 100.0f);
    glVertex2f(-100.0f, 200.0f);
    glVertex2f(-300.0f, 200.0f);
    glEnd();

    // Control tower
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(50.0f, 100.0f);
    glVertex2f(150.0f, 100.0f);
    glVertex2f(150.0f, 250.0f);
    glVertex2f(50.0f, 250.0f);
    glEnd();

    // Tower top
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(30.0f, 250.0f);
    glVertex2f(170.0f, 250.0f);
    glVertex2f(170.0f, 280.0f);
    glVertex2f(30.0f, 280.0f);
    glEnd();

    // Flag pole
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(180.0f, 100.0f);
    glVertex2f(190.0f, 100.0f);
    glVertex2f(190.0f, 300.0f);
    glVertex2f(180.0f, 300.0f);
    glEnd();

    // Bangladesh flag
    glColor3f(0.0f, 0.5f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(190.0f, 270.0f);
    glVertex2f(250.0f, 270.0f);
    glVertex2f(250.0f, 300.0f);
    glVertex2f(190.0f, 300.0f);
    glEnd();

    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(220.0f, 285.0f);
    for (int i = 0; i <= 360; i += 30) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(220.0f + cos(angle) * 20.0f, 285.0f + sin(angle) * 15.0f);
    }
    glEnd();
}

void drawSoldiers() {
    if (gameOver && victory) {
        // Draw 3 soldiers
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(300.0f + i * 100.0f, 150.0f, 0.0f);

            // Head
            glColor3f(0.8f, 0.6f, 0.4f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0.0f, 0.0f);
            for (int j = 0; j <= 360; j += 30) {
                float angle = j * 3.14159f / 180.0f;
                glVertex2f(cos(angle) * 15.0f, sin(angle) * 15.0f);
            }
            glEnd();

            // Body
            glColor3f(0.0f, 0.3f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(-10.0f, -15.0f);
            glVertex2f(10.0f, -15.0f);
            glVertex2f(10.0f, -40.0f);
            glVertex2f(-10.0f, -40.0f);
            glEnd();

            // Legs
            glColor3f(0.2f, 0.2f, 0.2f);
            glBegin(GL_QUADS);
            glVertex2f(-10.0f, -40.0f);
            glVertex2f(-5.0f, -40.0f);
            glVertex2f(-5.0f, -60.0f);
            glVertex2f(-10.0f, -60.0f);

            glVertex2f(5.0f, -40.0f);
            glVertex2f(10.0f, -40.0f);
            glVertex2f(10.0f, -60.0f);
            glVertex2f(5.0f, -60.0f);
            glEnd();

            // Arms
            glBegin(GL_QUADS);
            glVertex2f(-15.0f, -20.0f);
            glVertex2f(-10.0f, -20.0f);
            glVertex2f(-10.0f, -30.0f);
            glVertex2f(-15.0f, -30.0f);

            glVertex2f(10.0f, -20.0f);
            glVertex2f(15.0f, -20.0f);
            glVertex2f(15.0f, -30.0f);
            glVertex2f(10.0f, -30.0f);
            glEnd();

            // Saluting arm
            if (i == 1) {
                glBegin(GL_QUADS);
                glVertex2f(10.0f, -20.0f);
                glVertex2f(15.0f, -20.0f);
                glVertex2f(15.0f, 0.0f);
                glVertex2f(10.0f, 0.0f);
                glEnd();
            }

            glPopMatrix();
        }
    }
}

void drawBullet(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, -2.0f);
    glVertex2f(10.0f, -2.0f);
    glVertex2f(10.0f, 2.0f);
    glVertex2f(0.0f, 2.0f);
    glEnd();

    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Sky background gradient
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.5f, 0.8f); // Top color
    glVertex2f(-1500.0f, 800.0f);
    glVertex2f(1500.0f, 800.0f);
    glColor3f(0.6f, 0.8f, 1.0f); // Bottom color
    glVertex2f(1500.0f, 0.0f);
    glVertex2f(-1500.0f, 0.0f);
    glEnd();

    // Ground
    glColor3f(0.0f, 0.4f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1500.0f, 0.0f);
    glVertex2f(1500.0f, 0.0f);
    glVertex2f(1500.0f, 50.0f);
    glVertex2f(-1500.0f, 50.0f);
    glEnd();

    if (!scenario1Complete) {
        // Scenario 1: Indian plane attack
        drawArmyBase();
        drawBuilding();
        drawIndianPlane();
        drawBomb();

        // Display scenario text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-400.0f, 700.0f, "Scenario 1: Indian Air Force Attack on Bangladesh Base");
    } else if (!scenario2Complete) {
        // Scenario 2: Bangladesh plane takeoff
        drawRunway();
        drawBangladeshPlane();

        // Display scenario text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-400.0f, 700.0f, "Scenario 2: Bangladesh Air Force Response - Takeoff!");
    } else if (inCombat) {
        // Combat scenario
        drawBangladeshPlane();
        if (!enemy1Destroyed) drawEnemyPlane(enemy1X, enemy1Y, enemy1Alpha);
        if (!enemy2Destroyed) drawEnemyPlane(enemy2X, enemy2Y, enemy2Alpha);

        // Draw bullets when firing
        if (bangladeshPlaneX > enemy1X - 200 || bangladeshPlaneX > enemy2X - 200) {
            for (int i = 0; i < 5; i++) {
                drawBullet(bangladeshPlaneX + 120 + i * 30, bangladeshPlaneY + 10);
            }
        }

        // Display combat text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-300.0f, 700.0f, "Air Combat: Engage Enemy Aircrafts!");
        drawText(-200.0f, 650.0f, "Score: ");
        char scoreStr[20];
        sprintf(scoreStr, "%d", score);
        drawText(-100.0f, 650.0f, scoreStr);
    } else if (gameOver) {
        // Game over screen
        drawRunway();
        drawBangladeshPlane();
        drawSoldiers();

        if (victory) {
            glColor3f(0.0f, 1.0f, 0.0f);
            drawText(-200.0f, 700.0f, "MISSION ACCOMPLISHED!");
            drawText(-150.0f, 650.0f, "Pilot Congratulated!");
        } else {
            glColor3f(1.0f, 0.0f, 0.0f);
            drawText(-200.0f, 700.0f, "MISSION FAILED!");
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-150.0f, 600.0f, "Final Score: ");
        char scoreStr[20];
        sprintf(scoreStr, "%d", score);
        drawText(-50.0f, 600.0f, scoreStr);

        drawText(-300.0f, 500.0f, "Press ESC to exit or R to restart");
    }

    glutSwapBuffers();
}

void update(int value) {
    if (!scenario1Complete) {
        // Scenario 1: Indian plane moves in and drops bomb
        if (indianPlaneX < 400.0f) {
            indianPlaneX += 5.0f;
        } else if (!bombDropped) {
            bombDropped = true;
            bombX = indianPlaneX + 60.0f;
            bombY = indianPlaneY - 20.0f;
        }

        if (bombDropped && bombY > 100.0f) {
            bombY -= 8.0f;
        } else if (bombDropped && !buildingDestroyed) {
            buildingDestroyed = true;
            PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME);
        }

        if (buildingDestroyed && buildingAlpha > 0.0f) {
            buildingAlpha -= 0.01f;
        }

        if (indianPlaneX < 1200.0f) {
            indianPlaneX += 5.0f;
        } else {
            scenario1Complete = true;
        }
    } else if (!scenario2Complete) {
        // Scenario 2: Bangladesh plane takes off
        if (bangladeshPlaneX < 800.0f) {
            bangladeshPlaneX += 8.0f;
            if (bangladeshPlaneY < 300.0f) {
                bangladeshPlaneY += 2.0f;
            }
        } else {
            scenario2Complete = true;
            inCombat = true;
        }
    } else if (inCombat) {
        // Combat scenario
        if (bangladeshPlaneX < 1500.0f) {
            bangladeshPlaneX += 5.0f;
        }

        if (!enemy1Destroyed && enemy1X > -200.0f) {
            enemy1X -= 3.0f;
            enemy1Y += sin(enemy1X * 0.01f) * 5.0f;
        }

        if (!enemy2Destroyed && enemy2X > -200.0f) {
            enemy2X -= 4.0f;
            enemy2Y += sin(enemy2X * 0.02f) * 3.0f;
        }

        // Check for collisions
        if (!enemy1Destroyed && abs(bangladeshPlaneX - enemy1X) < 100 && abs(bangladeshPlaneY - enemy1Y) < 50) {
            enemy1Destroyed = true;
            score += 100;
            PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME);
        }

        if (!enemy2Destroyed && abs(bangladeshPlaneX - enemy2X) < 100 && abs(bangladeshPlaneY - enemy2Y) < 50) {
            enemy2Destroyed = true;
            score += 100;
            PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME);
        }

        // Fade out destroyed enemies
        if (enemy1Destroyed && enemy1Alpha > 0.0f) {
            enemy1Alpha -= 0.02f;
        }

        if (enemy2Destroyed && enemy2Alpha > 0.0f) {
            enemy2Alpha -= 0.02f;
        }

        // Check if combat is over
        if ((enemy1Destroyed && enemy2Destroyed) || (enemy1X < -200.0f && enemy2X < -200.0f)) {
            inCombat = false;
            gameOver = true;
            victory = enemy1Destroyed && enemy2Destroyed;

            if (victory) {
                PlaySound(TEXT("victory.wav"), NULL, SND_ASYNC | SND_FILENAME);
            } else {
                PlaySound(TEXT("defeat.wav"), NULL, SND_ASYNC | SND_FILENAME);
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC key
        exit(0);
    } else if ((key == 'r' || key == 'R') && gameOver) {
        // Reset game
        scenario1Complete = false;
        scenario2Complete = false;
        inCombat = false;
        gameOver = false;
        victory = false;

        indianPlaneX = -1000.0f;
        indianPlaneY = 300.0f;
        bangladeshPlaneX = -1500.0f;
        bangladeshPlaneY = 200.0f;
        bombX = 0.0f;
        bombY = 0.0f;
        bombDropped = false;
        buildingDestroyed = false;
        buildingAlpha = 1.0f;
        fireScale = 0.0f;

        enemy1X = 800.0f;
        enemy1Y = 350.0f;
        enemy2X = 1000.0f;
        enemy2Y = 250.0f;
        enemy1Destroyed = false;
        enemy2Destroyed = false;
        enemy1Alpha = 1.0f;
        enemy2Alpha = 1.0f;

        score = 0;
    }
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-width/2, width/2, 0, height);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1500, 800);
    glutCreateWindow("Bangladesh vs India Air Combat Simulation");

    glClearColor(0.2f, 0.5f, 0.8f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
