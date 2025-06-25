# 🎯 Top Gun: Air Defense

_A 2D air combat game built with OpenGL and GLUT where you defend Bangladesh's airspace against enemy aircraft._

---

## ✨ Game Features

- **🌅 Dynamic Day/Night Cycle**  
  Experience dawn, day, afternoon, and night phases with moving celestial bodies.

- **🌧️ Weather System**  
  Random rain effects that impact gameplay visibility.

- **📈 Progressive Difficulty**  
  Enemies increase over time with 6 total difficulty levels.

- **🛩️ Detailed Aircraft Models**  
  Includes F-16 (player) and MiG-21 (enemy) with visual animations.

- **🎬 Cinematic Sequences**  
  Takeoff and mission briefings for extra immersion.

- **🇧🇩 Bangladesh Air Force Theme**  
  Features a custom airport with patriotic elements like the national flag.

---



## 🎮 Controls

| Action              | Key       |
|---------------------|-----------|
| Move Aircraft       | `W`, `A`, `S`, `D` |
| Start Game / Confirm| `ENTER`   |
| Exit Game           | `ESC`     |
| Restart After Game Over | `R`  |

---

## 🛠️ Technical Implementation

- **Language:** C++
- **Graphics:** OpenGL (2D)
- **Window/Input Handling:** GLUT


---

### 📦 Key Code Structures

```cpp
// Game states
enum GameState { INTRO, TAKEOFF_CINEMATIC, MISSION_BRIEFING, GAMEPLAY, GAME_OVER };

// Fighter structure
struct Fighter {
    float x, y;
    float speed;
    bool alive;
    vector<pair<float, float>> bullets;
};

// Enemy structure
struct Enemy {
    float x, y;
    float speed;
    bool alive;
    int type; // 0 = MiG-21
};
