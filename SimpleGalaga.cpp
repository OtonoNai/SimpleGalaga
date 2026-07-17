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

class Player {
public:
	static constexpr char TEXT = 'A';
	static const int max_hp = 5;
	const int y_pos = GameWorld::SIZE_Y - 2;

	void MoveLeft() {
		if (x_pos == 1) {
			return;
		}
		--x_pos;
	}

	void MoveRight() {
		if (x_pos == GameWorld::SIZE_X - 2) {
			return;
		}
		++x_pos;
	}

	int GetX() {
		return x_pos;
	}

	int GetHp() {
		return cur_hp;
	}

	void GetDamaged() {
		--cur_hp;
	}

	int GetScore() {
		return score;
	}

	void SetScoreUp() {
		++score;
	}

private:
	int cur_hp = max_hp;
	int score = 0;
	int x_pos = GameWorld::SIZE_X / 2;
};

class Mob {
public:
	static constexpr char TEXT = 'X';

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

	bool IsOutOfBounds() {
		if (!is_active) {
			return false;
		}
		if (y_pos == GameWorld::SIZE_Y - 1) {
			Deactivate();
			return true;
		}

		return false;
	}

	bool IsActive() {
		return is_active;
	}

	int GetSpawnInterval() {
		return spawn_interval;
	}

	int GetElapsedFrameSpawn() {
		return elapsed_frame_spawn;
	}

	void AddElapsedFrameSpawn() {
		++elapsed_frame_spawn;
	}

	int GetX() {
		return x_pos;
	}

	int GetY() {
		return y_pos;
	}

	int GetPrevY() {
		return prev_y_pos;
	}

private:
	bool is_active = false;

	int x_pos = 0;
	int y_pos = 0;
	int prev_y_pos = 0;

	int move_interval = 0;
	int elapsed_frame_move = 0;
	int spawn_interval = 0;
	int elapsed_frame_spawn = 0;
};

class Bullet {
public:
	static constexpr char TEXT = '^';

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

	bool IsOutOfBounds() {
		if (!is_active) {
			return false;
		}
		if (y_pos == 0) {
			Deactivate();
			return true;
		}

		return false;
	}

	int GetY() {
		return y_pos;
	}

	int GetPrevY() {
		return prev_y_pos;
	}

	int GetX() {
		return x_pos;
	}

	bool IsActive() {
		return is_active;
	}

private:
	bool is_active = false;

	int x_pos = 0;
	int y_pos = 0;
	int prev_y_pos = 0;

	int move_interval = 5;
	int elapsed_frame = 0;
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

void MoveCursor(short x, short y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE),
		pos
	);
}

void SpawnMobs() {
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (mobs[i].IsActive()) {
			continue;
		}
		if (mobs[i].GetElapsedFrameSpawn() < mobs[i].GetSpawnInterval()) {
			mobs[i].AddElapsedFrameSpawn();
			continue;
		}
		mobs[i].Activate();
	}
}

void SpawnBullet() {
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		if (bullets[i].IsActive()) {
			continue;
		}

		bullets[i].Activate(player.GetX(), player.y_pos - 1);
		return;
	}
}

void Collision() {
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (!mobs[i].IsActive()) {
			continue;
		}

		for (int j = 0; j < GameWorld::MAX_BULLET; j++) {
			if (!bullets[j].IsActive() || !mobs[i].IsActive()) {
				continue;
			}
			if (bullets[j].GetX() != mobs[i].GetX()) {
				continue;
			}

			int bullet_min = min(bullets[j].GetY(), bullets[j].GetPrevY());
			int bullet_max = max(bullets[j].GetY(), bullets[j].GetPrevY());
			int mob_min = min(mobs[i].GetY(), mobs[i].GetPrevY());
			int mob_max = max(mobs[i].GetY(), mobs[i].GetPrevY());

			if (bullet_max >= mob_min && mob_max >= bullet_min) {
				mobs[i].Deactivate();
				bullets[j].Deactivate();
				player.SetScoreUp();
			}
		}

		if (mobs[i].GetX() != player.GetX()) {
			continue;
		}

		int mob_min = min(mobs[i].GetY(), mobs[i].GetPrevY());
		int mob_max = max(mobs[i].GetY(), mobs[i].GetPrevY());

		if (mob_min <= player.y_pos && player.y_pos <= mob_max) {
			mobs[i].Deactivate();
			player.GetDamaged();
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
			player.MoveLeft();
			break;
		case 'd':
			player.MoveRight();
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

	screenBuffer[player.y_pos][player.GetX()].Char.AsciiChar = Player::TEXT;
	screenBuffer[player.y_pos][player.GetX()].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		if (bullets[i].IsActive()) {
			screenBuffer[bullets[i].GetY()][bullets[i].GetX()].Char.AsciiChar = Bullet::TEXT;
			screenBuffer[bullets[i].GetY()][bullets[i].GetX()].Attributes = FOREGROUND_GREEN;
		}
	}

	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (mobs[i].IsActive()) {
			screenBuffer[mobs[i].GetY()][mobs[i].GetX()].Char.AsciiChar = Mob::TEXT;
			screenBuffer[mobs[i].GetY()][mobs[i].GetX()].Attributes = FOREGROUND_RED;
		}
	}

	COORD bufferSize = { GameWorld::SIZE_X, GameWorld::SIZE_Y };
	COORD bufferCoord = { 0, 0 };
	SMALL_RECT writeRegion = { 0, 0, GameWorld::SIZE_X - 1, GameWorld::SIZE_Y - 1 };
	WriteConsoleOutputA(hConsoleOut, (CHAR_INFO*)screenBuffer, bufferSize, bufferCoord, &writeRegion);

	MoveCursor(0, 21);
	cout << "Score : " << player.GetScore() << endl;
	cout << "HP : " << player.GetHp() << endl;
}

bool EndGame() {
	if (player.GetHp() > 0) {
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
	SpawnMobs();
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		bullets[i].Move();
	}
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		mobs[i].Move();
	}
}

void LateUpdate() {
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		bullets[i].IsOutOfBounds();
	}
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		mobs[i].IsOutOfBounds();
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