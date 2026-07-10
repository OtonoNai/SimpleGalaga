#include <iostream>
#include <windows.h>
#include <conio.h>
#include <random>
#include <algorithm>

using namespace std;

random_device rd;
mt19937_64 gen(rd());

struct GameWorld {
	static constexpr int SIZE_X = 40;
	static constexpr int SIZE_Y = 20;
	char text[GameWorld::SIZE_Y][GameWorld::SIZE_X];

	static constexpr int MAX_MOB = 5;
	static constexpr int MAX_BULLET = 3;
};

struct Player {
	static constexpr char TEXT = 'A';
	static constexpr int Y_POS = GameWorld::SIZE_Y - 2;
	const int max_hp = 5;

	int cur_hp = max_hp;
	int score = 0;
	int x_pos = GameWorld::SIZE_X / 2;
	int y_pos = GameWorld::SIZE_Y - 2;
};

struct Mob {
	static constexpr char TEXT = 'X';

	bool is_active = false;
	int x_pos = 0;
	int y_pos = 0;
	int prev_y_pos = 0;
	int move_interval = 0;
	int elapsed_frame_move = 0;
	int spawn_interval = 0;
	int elapsed_frame_spawn = 0;

	Mob() {
		uniform_int_distribution<int> respawn(30, 120);
		spawn_interval = respawn(gen);
	}

	void Activate() {
		uniform_int_distribution<int> distrib(1, GameWorld::SIZE_X - 2);
		uniform_int_distribution<int> respawn(30, 120);
		uniform_int_distribution<int> moveInterval(10, 60);
		is_active = true;
		x_pos = distrib(gen);
		y_pos = 1;
		prev_y_pos = 1;
		move_interval = moveInterval(gen);
		spawn_interval = respawn(gen);
		elapsed_frame_spawn = 0;
	}

	void Deactivate() {
		is_active = false;
		elapsed_frame_spawn = 0;
	}

	void Move() {
		if (!is_active) {
			return;
		}
		if (elapsed_frame_move++ < move_interval) {
			return;
		}

		prev_y_pos = y_pos++;
		elapsed_frame_move = 0;
	}

	bool CheckOutOfBounds() {
		if (!is_active) {
			return false;
		}
		if (y_pos == GameWorld::SIZE_Y - 1) {
			Deactivate();
			return true;
		}
		else {
			return false;
		}
	}
};

struct Bullet {
	static constexpr char TEXT = '^';

	bool is_active = false;
	int x_pos = 0;
	int y_pos = 0;
	int prev_y_pos = 0;
	int move_interval = 5;
	int elapsed_frame = 0;

	Bullet() {}

	void Activate(int x, int y) {
		is_active = true;

		x_pos = x;
		y_pos = prev_y_pos = y;

		elapsed_frame = 0;
	}

	void Deactivate() {
		is_active = false;
	}

	void Move() {
		if (!is_active) {
			return;
		}
		if (elapsed_frame++ < move_interval) {
			return;
		}

		prev_y_pos = y_pos--;
		elapsed_frame = 0;
	}

	bool CheckOutOfBounds() {
		if (!is_active) {
			return false;
		}
		if (y_pos == 0) {
			Deactivate();
			return true;
		}
		else {
			return false;
		}
	}
};

GameWorld world;
Player player;
Mob mobs[GameWorld::MAX_MOB];
Bullet bullets[GameWorld::MAX_BULLET];
CHAR_INFO screenBuffer[GameWorld::SIZE_Y][GameWorld::SIZE_X];
HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

void HideCursor()
{
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;

	SetConsoleCursorInfo(
		GetStdHandle(STD_OUTPUT_HANDLE),
		&cursorInfo
	);
}

void MoveCursor(int x, int y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE),
		pos
	);
}

void GenerateMobs() {
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (mobs[i].is_active) {
			continue;
		}
		if (mobs[i].elapsed_frame_spawn++ < mobs[i].spawn_interval) {
			continue;
		}
		mobs[i].Activate();
	}
}

void SpawnBullet() {
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		if (bullets[i].is_active) {
			continue;
		}
		else {
			bullets[i].Activate(player.x_pos, player.y_pos - 1);
			return;
		}
	}
}

void Collision() {
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		for (int j = 0; j < GameWorld::MAX_BULLET; j++) {
			if (!bullets[j].is_active || !mobs[i].is_active) {
				continue;
			}
			if (bullets[j].x_pos != mobs[i].x_pos) {
				continue;
			}

			int bulletMin = min(bullets[j].y_pos, bullets[j].prev_y_pos);
			int bulletMax = max(bullets[j].y_pos, bullets[j].prev_y_pos);
			int mobMin = min(mobs[i].y_pos, mobs[i].prev_y_pos);
			int mobMax = max(mobs[i].y_pos, mobs[i].prev_y_pos);

			if (bulletMax >= mobMin && mobMax >= bulletMin) {
                mobs[i].Deactivate();
                bullets[j].Deactivate();
                player.score++;
            }
		}
	}

}

void Input()
{
	if (_kbhit())
	{
		char key = _getch();

		switch (key)
		{
		case 'a':
			if (player.x_pos == 1) {
				return;
			}
			player.x_pos--;

			break;

		case 'd':
			if (player.x_pos == GameWorld::SIZE_X - 2) {
				return;
			}
			player.x_pos++;

			break;
		case 'w':
			SpawnBullet();
			break;
		}
	}
}

void DrawWorld() {
	for (int y = 0; y < GameWorld::SIZE_Y; ++y) {
		for (int x = 0; x < GameWorld::SIZE_X; ++x) {
			bool isEdge = (y == 0 || y == GameWorld::SIZE_Y - 1 || x == 0 || x == GameWorld::SIZE_X - 1);
			screenBuffer[y][x].Char.AsciiChar = isEdge ? '*' : ' ';
			screenBuffer[y][x].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		}
	}

	screenBuffer[player.y_pos][player.x_pos].Char.AsciiChar = Player::TEXT;
	screenBuffer[player.y_pos][player.x_pos].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		if (bullets[i].is_active) {
			screenBuffer[bullets[i].y_pos][bullets[i].x_pos].Char.AsciiChar = Bullet::TEXT;
			screenBuffer[bullets[i].y_pos][bullets[i].x_pos].Attributes = FOREGROUND_GREEN;
		}
	}

	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (mobs[i].is_active) {
			screenBuffer[mobs[i].y_pos][mobs[i].x_pos].Char.AsciiChar = Mob::TEXT;
			screenBuffer[mobs[i].y_pos][mobs[i].x_pos].Attributes = FOREGROUND_RED;
		}
	}

	COORD bufferSize = { GameWorld::SIZE_X, GameWorld::SIZE_Y };
	COORD bufferCoord = { 0, 0 };
	SMALL_RECT writeRegion = { 0, 0, GameWorld::SIZE_X - 1, GameWorld::SIZE_Y - 1 };
	WriteConsoleOutputA(hConsoleOut, (CHAR_INFO*)screenBuffer, bufferSize, bufferCoord, &writeRegion);

	MoveCursor(0, 21);
	cout << "Score : " << player.score << endl;
	cout << "HP : " << player.cur_hp << endl;
}

bool EndGame() {
	if (player.cur_hp > 0) {
		return false;
	}
	MoveCursor(0, 0);
	for (int y = 0; y < GameWorld::SIZE_Y; y++) {
		for (int x = 0; x < GameWorld::SIZE_X; x++) {
			world.text[y][x] = ' ';
			if ((y == 0 || y == GameWorld::SIZE_Y - 1) || (x == 0 || x == GameWorld::SIZE_X - 1)) {
				world.text[y][x] = '*';
			}
			cout << world.text[y][x];
		}
		cout << endl;
	}
	MoveCursor(1, 1);
	cout << "GAME OVER";
	MoveCursor(0, 20);
	return true;
}

void Update() {
	GenerateMobs();
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		bullets[i].Move();
	}
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		mobs[i].Move();
	}
}

void LateUpdate() {
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		bullets[i].CheckOutOfBounds();
	}
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (mobs[i].CheckOutOfBounds()) {
			player.cur_hp--;
		}
	}
}

int main()
{
	HideCursor();
	while (true) {
		Input();
		Update();
		Collision();
		LateUpdate();
		DrawWorld();
		if (EndGame()) {
			break;
		}
		Sleep(20);
	}
}