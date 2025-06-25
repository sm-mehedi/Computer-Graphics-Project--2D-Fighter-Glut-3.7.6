<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Top Gun: Air Defense</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        h1 {
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
        }
        h2 {
            color: #2980b9;
            margin-top: 25px;
        }
        .screenshot {
            width: 100%;
            max-width: 600px;
            border: 1px solid #ddd;
            border-radius: 4px;
            margin: 20px auto;
            display: block;
        }
        .code-block {
            background-color: #f8f9fa;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            font-family: 'Courier New', Courier, monospace;
            font-size: 14px;
        }
        .features {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        .feature-card {
            background-color: white;
            padding: 15px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .controls {
            background-color: #e8f4fc;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <h1>Top Gun: Air Defense</h1>
    
    <p>A 2D air combat game built with OpenGL and GLUT where you defend Bangladesh's airspace against enemy aircraft.</p>
    
    <h2>Game Features</h2>
    <div class="features">
        <div class="feature-card">
            <h3>Dynamic Day/Night Cycle</h3>
            <p>Experience dawn, day, afternoon, and night phases with changing celestial bodies and lighting.</p>
        </div>
        <div class="feature-card">
            <h3>Weather System</h3>
            <p>Random weather changes including rain effects that impact visibility.</p>
        </div>
        <div class="feature-card">
            <h3>Progressive Difficulty</h3>
            <p>Enemy spawn rate and numbers increase over time, with 6 difficulty levels.</p>
        </div>
        <div class="feature-card">
            <h3>Detailed Aircraft Models</h3>
            <p>Accurate representations of F-16 (player) and MiG-21 (enemy) with animations.</p>
        </div>
        <div class="feature-card">
            <h3>Cinematic Sequences</h3>
            <p>Takeoff cinematic and mission briefing sequences for immersion.</p>
        </div>
        <div class="feature-card">
            <h3>Bangladesh Airforce Theme</h3>
            <p>Custom airport with Bangladesh flag and patriotic elements.</p>
        </div>
    </div>

    <h2>Game Screens</h2>
    <p><em>Note: Screenshot placeholders - replace with actual game images</em></p>
    <img src="screenshots/intro.png" alt="Intro Screen" class="screenshot">
    <img src="screenshots/gameplay.png" alt="Gameplay Screen" class="screenshot">
    
    <h2>Controls</h2>
    <div class="controls">
        <p><strong>WASD</strong> - Move your aircraft</p>
        <p><strong>ENTER</strong> - Start game/confirm selections</p>
        <p><strong>ESC</strong> - Exit game</p>
        <p><strong>R</strong> - Restart after game over</p>
    </div>
    
    <h2>Technical Implementation</h2>
    <p>The game is implemented using:</p>
    <ul>
        <li>OpenGL for 2D rendering</li>
        <li>GLUT for window management and input</li>
        <li>C++ for game logic</li>
        <li>Windows API for sound playback</li>
    </ul>
    
    <h3>Key Code Structures</h3>
    <div class="code-block">
// Game states
enum GameState { INTRO, TAKEOFF_CINEMATIC, MISSION_BRIEFING, GAMEPLAY, GAME_OVER };

// Player and enemy structures
struct Fighter {
    float x, y;
    float speed;
    bool alive;
    vector&lt;pair&lt;float, float&gt;&gt; bullets;
};

struct Enemy {
    float x, y;
    float speed;
    bool alive;
    int type; // 0=MiG-21
};
    </div>
    
    <h2>Building and Running</h2>
    <p>Requirements:</p>
    <ul>
        <li>Windows OS</li>
        <li>Visual Studio with OpenGL/GLUT configured</li>
        <li>Required libraries: freeglut, winmm</li>
    </ul>
    
    <h2>Future Improvements</h2>
    <ul>
        <li>Add more enemy aircraft types</li>
        <li>Implement power-ups and special weapons</li>
        <li>Add mission objectives</li>
        <li>Improve collision detection</li>
        <li>Add particle effects for explosions</li>
    </ul>
    
    <h2>License</h2>
    <p>This project is open source under the MIT License.</p>
</body>
</html>
