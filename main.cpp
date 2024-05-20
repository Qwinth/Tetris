#define FIELD_WIDTH 10
#define FIELD_HEIGHT 18

#include <iostream>
#include <vector>
#include <random>
#include <raylib.h>
using namespace std;

random_device rd;
mt19937 rng(rd());
std::uniform_int_distribution<std::mt19937::result_type> frandom(0, 6);

struct pos_t {
    int X = 0;
    int Y = 0;
};

struct figure_t {
    vector<pos_t> blockpos;
    vector<pos_t> rightPeaks;
    vector<pos_t> leftPeaks;
    vector<pos_t> downPeaks;
    pos_t absPos;
    int rotatePoint;
};

enum collide_dir {
    ANY,
    RIGHT,
    LEFT,
    DOWN
};

vector<vector<bool>> field;

vector<pos_t> findRow(figure_t figure, int row) {
    vector<pos_t> ret;

    for (pos_t i : figure.blockpos) if (i.Y == row) ret.push_back(i);

    return ret;
}

vector<pos_t> findColumn(figure_t figure, int column) {
    vector<pos_t> ret;

    for (pos_t i : figure.blockpos) if (i.X == column) ret.push_back(i);

    return ret;
}

void findRightPeaks(figure_t& figure) {
    int minY = 0;
    int maxY = 0;
    int minX = 0;

    for (pos_t i : figure.blockpos) {
        if (i.X < minX) minX = i.X;
        if (i.Y > maxY) maxY = i.Y;
        if (i.Y < minY) minY = i.Y;
    }

    for (int i = minY; i <= maxY; i++) {
        pos_t peak = {minX, 0};

        for (pos_t& i : findRow(figure, i)) if (i.X >= peak.X) peak = i;

        figure.rightPeaks.push_back(peak);
    }
}

void findLeftPeaks(figure_t& figure) {
    int minY = 0;
    int maxY = 0;
    int maxX = 0;

    for (pos_t i : figure.blockpos) {
        if (i.X > maxX) maxX = i.X;
        if (i.Y > maxY) maxY = i.Y;
        if (i.Y < minY) minY = i.Y;
    }

    for (int i = minY; i <= maxY; i++) {
        pos_t peak = {maxX, 0};

        for (pos_t& i : findRow(figure, i)) if (i.X <= peak.X) peak = i;

        figure.leftPeaks.push_back(peak);
    }
}

void findDownPeaks(figure_t& figure) {
    vector<pos_t> peaks;
    int maxX = 0;
    int minX = 0;
    int minY = 0;

    for (pos_t i : figure.blockpos) {
        if (i.X > maxX) maxX = i.X;
        if (i.X < minX) minX = i.X;
        if (i.Y < minY) minY = i.Y;
    }

    for (int i = minX; i <= maxX; i++) {
        pos_t peak = {0, minY};

        for (pos_t& i : findColumn(figure, i)) if (i.Y >= peak.Y) peak = i;

        figure.downPeaks.push_back(peak);
    }
}

bool checkCollision(figure_t figure, collide_dir dir, bool check_my_pos = false) {

    if (dir == ANY && check_my_pos) {
        for (pos_t i : figure.downPeaks) if (figure.absPos.Y + i.Y >= FIELD_HEIGHT) return true;
        for (pos_t i : figure.rightPeaks) if (figure.absPos.X + i.X >= FIELD_WIDTH) return true;
        for (pos_t i : figure.leftPeaks) if (figure.absPos.X + i.X < 0) return true;
    }
    
    if (dir == DOWN || dir == ANY) {
        for (pos_t i : figure.downPeaks) {
            if (figure.absPos.Y + i.Y + 1 >= FIELD_HEIGHT) return true;
            else if (field[figure.absPos.X + i.X][figure.absPos.Y + i.Y + 1]) return true;
            else if (check_my_pos && field[figure.absPos.X + i.X][figure.absPos.Y + i.Y]) return true;
        }
    }

    else if (dir == RIGHT || dir == ANY) {
        for (pos_t i : figure.rightPeaks) {
            if (figure.absPos.X + i.X + 1 >= FIELD_WIDTH) return true;
            else if (field[figure.absPos.X + i.X + 1][figure.absPos.Y + i.Y]) return true;
            else if (check_my_pos && field[figure.absPos.X + i.X][figure.absPos.Y + i.Y]) return true;
        }
    }

    else if (dir == LEFT || dir == ANY) {
        for (pos_t i : figure.leftPeaks) {
            if (figure.absPos.X + i.X == 0) return true;
            else if (field[figure.absPos.X + i.X - 1][figure.absPos.Y + i.Y]) return true;
            else if (check_my_pos && field[figure.absPos.X + i.X][figure.absPos.Y + i.Y]) return true;
        }
    }

    return false;
}

void initField(int width, int height) {
    field.reserve(width);
    field.resize(width);

    for (auto& i : field) {
        i.reserve(height);
        i.resize(height);
    }
}

void drawField() {
    for (int i = 0; i < field.size(); i++) {
        vector<bool> column = field[i];

        for (int j = 0; j < column.size(); j++) {
            DrawRectangle(i * 50, j * 50, 50, 50, WHITE);
            DrawRectangle(i * 50 + 5, j * 50 + 5, 45, 45, (column[j]) ? DARKBLUE : BLACK);
        }
    }
}

bool checkFullRow(int row) {
    for (int i = 0; i < FIELD_WIDTH; i++) if (!field[i][row]) return false;

    return true;
}

void clearFullRow(int row) {
    for (int i = 0; i < FIELD_WIDTH; i++) field[i][row] = false;
}

void moveRows(int start) {
    for (int i = start - 1; i >= 0; i--) {
        for (int j = 0; j < FIELD_WIDTH; j++) field[j][i + 1] = field[j][i];
    }
}

void clearFullRows() {
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        if (checkFullRow(i)) {
            clearFullRow(i);
            moveRows(i);
        }
    }
}

void insertToField(figure_t figure) {
    for (pos_t i : figure.blockpos) field[i.X + figure.absPos.X][i.Y + figure.absPos.Y] = true;
}

void removeFromField(figure_t figure) {
    for (pos_t i : figure.blockpos) field[i.X + figure.absPos.X][i.Y + figure.absPos.Y] = false;
}

void moveFigureX(figure_t& figure, bool invert = false) {
    if (!invert && checkCollision(figure, RIGHT)) return;
    else if (invert && checkCollision(figure, LEFT)) return;

    removeFromField(figure);
    if (!invert) figure.absPos.X++;
    else figure.absPos.X--;
    insertToField(figure);
}

void moveFigureY(figure_t& figure) {
    if (checkCollision(figure, DOWN)) return;

    removeFromField(figure);
    figure.absPos.Y++;
    insertToField(figure);
}

void jumpToFloor(figure_t& figure) {
    while (!checkCollision(figure, DOWN)) {
        moveFigureY(figure);
    }
}

void normalizeFigure(figure_t& figure) {
    int minX = 0;
    int minY = 0;

    for (pos_t i : figure.blockpos) {
        if (i.X > minX) minX = i.X;
        if (i.Y > minY) minY = i.Y;
    }

    for (pos_t i : figure.blockpos) {
        if (i.X < minX) minX = i.X;
        if (i.Y < minY) minY = i.Y;
    }

    for (pos_t& i : figure.blockpos) {
        i.X -= minX;
        i.Y -= minY;
    }
}

void rotateFigure(figure_t& figure) {
    removeFromField(figure);

    figure_t newFigure;
    newFigure.absPos = figure.absPos;
    newFigure.rotatePoint = figure.rotatePoint;

    for (int i = 0; i < figure.blockpos.size(); i++) {
        pos_t pos = figure.blockpos[i];
        pos_t rotatePointPos = figure.blockpos[figure.rotatePoint];

        pos_t newpos;
        newpos.X = rotatePointPos.Y - pos.Y;
        newpos.Y = pos.X - rotatePointPos.X;

        newFigure.blockpos.push_back(newpos);
    }

    normalizeFigure(newFigure);

    findRightPeaks(newFigure);
    findLeftPeaks(newFigure);
    findDownPeaks(newFigure);

    if (!checkCollision(newFigure, ANY, true)) figure = newFigure;;

    insertToField(figure);   
}

figure_t create_I(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 1;
    figure.blockpos = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};

    normalizeFigure(figure);

    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_I(int X, int Y) { return create_I({X, Y}); }

figure_t create_J(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 1;
    figure.blockpos = {{0, 0}, {0, 1}, {0, 2}, {-1, 2}};

    normalizeFigure(figure);
    
    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_J(int X, int Y) { return create_J({X, Y}); }

figure_t create_L(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 1;
    figure.blockpos = {{0, 0}, {0, 1}, {0, 2}, {1, 2}};

    normalizeFigure(figure);
    
    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_L(int X, int Y) { return create_L({X, Y}); }

figure_t create_O(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 0;
    figure.blockpos = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};

    normalizeFigure(figure);

    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_O(int X, int Y) { return create_O({X, Y}); }

figure_t create_S(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 2;
    figure.blockpos = {{0, 0}, {1, 0}, {-1, 1}, {0, 1}};

    normalizeFigure(figure);

    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_S(int X, int Y) { return create_S({X, Y}); }

figure_t create_Z(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 2;
    figure.blockpos = {{0, 0}, {1, 0}, {1, 1}, {2, 1}};

    normalizeFigure(figure);
    
    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_Z(int X, int Y) { return create_Z({X, Y}); }

figure_t create_T(pos_t pos) {
    figure_t figure;
    figure.absPos = pos;
    figure.rotatePoint = 1;
    figure.blockpos = {{0, 0}, {1, 0}, {2, 0}, {1, 1}};

    normalizeFigure(figure);

    findRightPeaks(figure);
    findLeftPeaks(figure);
    findDownPeaks(figure);

    return figure;
}

figure_t create_T(int X, int Y) { return create_T({X, Y}); }

figure_t randomFigure() {
    int frand = frandom(rng);

    if (frand == 0) return create_I(3, 0);
    else if (frand == 1) return create_J(4, 0);
    else if (frand == 2) return create_L(4, 0);
    else if (frand == 3) return create_O(4, 0);
    else if (frand == 4) return create_S(4, 0);
    else if (frand == 5) return create_Z(4, 0);

    return create_T(4, 0);
}

int main() {    
    InitWindow(500, 900, "Tetris");
    SetTargetFPS(30);

    initField(FIELD_WIDTH, FIELD_HEIGHT);

    figure_t currentFigure = randomFigure();
    insertToField(currentFigure);

    double start = GetTime();
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (GetTime() - start >= 1) {
            if (checkCollision(currentFigure, DOWN)) {
                clearFullRows();
                currentFigure = randomFigure();
                insertToField(currentFigure);
            }

            start = GetTime();
            moveFigureY(currentFigure);
        } else if (IsKeyPressed(KEY_DOWN)) moveFigureY(currentFigure);
    

        if (IsKeyPressed(KEY_RIGHT)) moveFigureX(currentFigure);
        if (IsKeyPressed(KEY_LEFT)) moveFigureX(currentFigure, true);
        if (IsKeyPressed(KEY_UP)) rotateFigure(currentFigure);
        if (IsKeyPressed(KEY_SPACE)) {
            jumpToFloor(currentFigure);

            clearFullRows();
            currentFigure = randomFigure();
            insertToField(currentFigure);
        }
        
        drawField();

        EndDrawing();
    }
    
}
