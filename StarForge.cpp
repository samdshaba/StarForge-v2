#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

using namespace std;

// Constants
const int MAP_SIZE = 12;
const char PLAYER = 'P';
const char ORE = 'O';
const char ENERGY = 'E';
const char SCRAP = 'S';
const char DRONE = 'D';
const char EMPTY = '.';
const char HAZARD = 'X';

// Game state
struct Player {
    int x, y;
    int ore, energy, scrap;
    int health;
    bool hasShield;
    int repairKits;
    int score;
};

struct Drone {
    int x, y;
    bool active;
};

struct Game {
    vector<vector<char>> map;
    Player player;
    vector<Drone> drones;
    bool gameOver;
    int ticks;
};

// Initialize game
void initGame(Game& game) {
    game.map = vector<vector<char>>(MAP_SIZE, vector<char>(MAP_SIZE, EMPTY));
    game.player = {0, 0, 0, 0, 0, 100, false, 0, 0};
    game.drones = vector<Drone>(3, {0, 0, false});
    game.gameOver = false;
    game.ticks = 0;

    // Place resources and drones
    srand(time(0));
    for (int i = 0; i < 12; ++i) {
        int rx = rand() % MAP_SIZE;
        int ry = rand() % MAP_SIZE;
        if (game.map[rx][ry] == EMPTY && (rx != 0 || ry != 0)) {
            int type = rand() % 3;
            game.map[rx][ry] = (type == 0) ? ORE : (type == 1) ? ENERGY : SCRAP;
        }
    }
    for (int i = 0; i < 3; ++i) {
        int dx = rand() % MAP_SIZE;
        int dy = rand() % MAP_SIZE;
        if (game.map[dx][dy] == EMPTY && (dx != 0 || dy != 0)) {
            game.drones[i] = {dx, dy, true};
            game.map[dx][dy] = DRONE;
        }
    }
}

// Display map
void displayMap(const Game& game) {
    system("clear"); // Use "cls" on Windows
    cout << "=== StarForge v2 ===" << endl;
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            if (i == game.player.x && j == game.player.y) {
                cout << PLAYER << " ";
            } else {
                cout << game.map[i][j] << " ";
            }
        }
        cout << endl;
    }
    cout << "Score: " << game.player.score << " | Health: " << game.player.health 
         << " | Ore: " << game.player.ore << " | Energy: " << game.player.energy 
         << " | Scrap: " << game.player.scrap << " | Shield: " 
         << (game.player.hasShield ? "Yes" : "No") << " | Repair Kits: " 
         << game.player.repairKits << endl;
}

// Move player
void movePlayer(Game& game, char direction) {
    int newX = game.player.x;
    int newY = game.player.y;

    if (direction == 'w' && newX > 0) newX--;
    else if (direction == 's' && newX < MAP_SIZE - 1) newX++;
    else if (direction == 'a' && newY > 0) newY--;
    else if (direction == 'd' && newY < MAP_SIZE - 1) newY++;

    // Check for resources, hazards, or drones
    if (game.map[newX][newY] == ORE) {
        game.player.ore++;
        game.player.score += 10;
        game.map[newX][newY] = EMPTY;
    } else if (game.map[newX][newY] == ENERGY) {
        game.player.energy++;
        game.player.score += 15;
        game.map[newX][newY] = EMPTY;
    } else if (game.map[newX][newY] == SCRAP) {
        game.player.scrap++;
        game.player.score += 5;
        game.map[newX][newY] = EMPTY;
    } else if (game.map[newX][newY] == HAZARD) {
        if (!game.player.hasShield) {
            game.player.health -= 20;
            cout << "Hit by meteor! Health -20" << endl;
        } else {
            cout << "Shield protected you!" << endl;
            game.player.hasShield = false;
        }
        game.map[newX][newY] = EMPTY;
    } else if (game.map[newX][newY] == DRONE) {
        if (!game.player.hasShield) {
            game.player.health -= 15;
            cout << "Attacked by drone! Health -15" << endl;
        } else {
            cout << "Shield blocked drone attack!" << endl;
            game.player.hasShield = false;
        }
        // Drone doesn't disappear
    }

    game.player.x = newX;
    game.player.y = newY;
}

// Move drones
void moveDrones(Game& game) {
    for (auto& drone : game.drones) {
        if (!drone.active) continue;
        game.map[drone.x][drone.y] = EMPTY;
        int dx = game.player.x - drone.x;
        int dy = game.player.y - drone.y;
        if (abs(dx) > abs(dy)) {
            drone.x += (dx > 0) ? 1 : -1;
        } else {
            drone.y += (dy > 0) ? 1 : -1;
        }
        if (drone.x >= 0 && drone.x < MAP_SIZE && drone.y >= 0 && drone.y < MAP_SIZE) {
            if (drone.x == game.player.x && drone.y == game.player.y) {
                if (!game.player.hasShield) {
                    game.player.health -= 15;
                    cout << "Drone attacked you! Health -15" << endl;
                } else {
                    cout << "Shield blocked drone attack!" << endl;
                    game.player.hasShield = false;
                }
            } else if (game.map[drone.x][drone.y] == EMPTY) {
                game.map[drone.x][drone.y] = DRONE;
            }
        }
    }
}

// Craft items
void craftItem(Game& game) {
    cout << "Crafting menu:\n1. Shield (2 Ore, 1 Energy)\n2. Repair Kit (1 Ore, 1 Scrap)\nChoose (1-2 or other to cancel): ";
    int choice;
    cin >> choice;
    if (choice == 1 && game.player.ore >= 2 && game.player.energy >= 1 && !game.player.hasShield) {
        game.player.ore -= 2;
        game.player.energy -= 1;
        game.player.hasShield = true;
        cout << "Crafted a Shield!" << endl;
    } else if (choice == 2 && game.player.ore >= 1 && game.player.scrap >= 1) {
        game.player.ore -= 1;
        game.player.scrap -= 1;
        game.player.repairKits++;
        cout << "Crafted a Repair Kit!" << endl;
    } else {
        cout << "Cannot craft: insufficient resources or already have a shield." << endl;
    }
}

// Use repair kit
void useRepairKit(Game& game) {
    if (game.player.repairKits > 0 && game.player.health < 100) {
        game.player.repairKits--;
        game.player.health = min(100, game.player.health + 30);
        cout << "Used Repair Kit! Health +30" << endl;
    } else {
        cout << "No Repair Kits or full health!" << endl;
    }
}

// Random event: Meteor shower
void randomEvent(Game& game) {
    if (rand() % 100 < 15) { // 15% chance
        int hx = rand() % MAP_SIZE;
        int hy = rand() % MAP_SIZE;
        if (game.map[hx][hy] == EMPTY && (hx != game.player.x || hy != game.player.y)) {
            game.map[hx][hy] = HAZARD;
            cout << "Meteor shower! A hazard appeared at (" << hx << ", " << hy << ")." << endl;
        }
    }
}

// Save game
void saveGame(const Game& game) {
    ofstream file("starforge_save.txt");
    if (file.is_open()) {
        file << game.player.x << " " << game.player.y << " " << game.player.ore << " "
             << game.player.energy << " " << game.player.scrap << " " << game.player.health << " "
             << game.player.hasShield << " " << game.player.repairKits << " " << game.player.score << endl;
        for (const auto& row : game.map) {
            for (char cell : row) {
                file << cell << " ";
            }
            file << endl;
        }
        for (const auto& drone : game.drones) {
            file << drone.x << " " << drone.y << " " << drone.active << endl;
        }
        file << game.ticks << endl;
        file.close();
        cout << "Game saved!" << endl;
    }
}

// Load game
bool loadGame(Game& game) {
    ifstream file("starforge_save.txt");
    if (file.is_open()) {
        file >> game.player.x >> game.player.y >> game.player.ore >> game.player.energy
             >> game.player.scrap >> game.player.health >> game.player.hasShield
             >> game.player.repairKits >> game.player.score;
        game.map = vector<vector<char>>(MAP_SIZE, vector<char>(MAP_SIZE));
        for (auto& row : game.map) {
            for (char& cell : row) {
                file >> cell;
            }
        }
        for (auto& drone : game.drones) {
            file >> drone.x >> drone.y >> drone.active;
        }
        file >> game.ticks;
        file.close();
        cout << "Game loaded!" << endl;
        return true;
    }
    return false;
}

// Main game loop
int main() {
    Game game;
    cout << "Load saved game? (y/n): ";
    char load;
    cin >> load;
    if (load == 'y' && !loadGame(game)) {
        cout << "No save file found. Starting new game." << endl;
        initGame(game);
    } else if (load != 'y') {
        initGame(game);
    }

    cout << "Welcome to StarForge v2! Use WASD to move, 'c' to craft, 'r' to use Repair Kit, 's' to save, 'q' to quit.\n";

    while (!game.gameOver) {
        displayMap(game);
        cout << "Enter action: ";
        char action;
        cin >> action;

        if (action == 'q') {
            cout << "Game Over. Final Score: " << game.player.score << ". Thanks for playing!" << endl;
            break;
        } else if (action == 'c') {
            craftItem(game);
        } else if (action == 'r') {
            useRepairKit(game);
        } else if (action == 's') {
            saveGame(game);
        } else if (action == 'w' || action == 'a' || action == 's' || action == 'd') {
            movePlayer(game, action);
            if (game.ticks % 3 == 0) moveDrones(game);
            randomEvent(game);
            game.player.score += 1; // Score for surviving
            game.ticks++;
        }

        if (game.player.health <= 0) {
            cout << "You died! Final Score: " << game.player.score << ". Game Over." << endl;
            game.gameOver = true;
        }
    }

    return 0;
}
