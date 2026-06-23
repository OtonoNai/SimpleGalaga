#include <iostream>
#include <windows.h>
#include <conio.h>
#include <random>

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
	int move_interval = 0;
	int elapsed_frame_move = 0;
	int spawn_interval = 0;
	int elapsed_frame_spawn = 0;


	Mob() {}

	void Move() {
		if (!is_active) {
			return;
		}
		if (elapsed_frame_move++ < move_interval) {
			return;
		}

		y_pos++;
		elapsed_frame_move = 0;
	}

	void Deactivate() {
		is_active = false;
	}
};

struct Bullet {
	static constexpr char TEXT = '^';

	bool is_active = false;
	int x_pos = 0;
	int y_pos = 0;
	int move_interval = 5;
	int elapsed_frame = 0;

	Bullet() {}

	void Activate(int x, int y) {
		is_active = true;

		x_pos = x;
		y_pos = y;

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

		y_pos--;
		elapsed_frame = 0;
	}
};

GameWorld world;
Player player;
Mob mobs[GameWorld::MAX_MOB];
Bullet bullets[GameWorld::MAX_BULLET];

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
			return;
		}
		uniform_int_distribution<int> distrib(1, GameWorld::SIZE_X - 2);
		uniform_int_distribution<int> respawn(10, 40);
		uniform_int_distribution<int> moveInterval(30, 120);
		mobs[i].is_active = true;
		mobs[i].x_pos = distrib(gen);
		mobs[i].y_pos = 1;
		mobs[i].move_interval = moveInterval(gen);
		mobs[i].spawn_interval = respawn(gen);
		mobs[i].elapsed_frame_spawn = 0;
	}
}

void FindDeactivatedBullet() {
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
			if (bullets[j].is_active && mobs[i].is_active) {
				if (bullets[j].x_pos == mobs[i].x_pos && bullets[j].y_pos == mobs[i].y_pos) {
					mobs[i].is_active = false;
					bullets[j].is_active = false;
					player.score++;
				}
			}
			else if (bullets[j].is_active) {
				if (bullets[j].y_pos == 0) {
					bullets[j].Deactivate();
				}
			}
			else if (mobs[i].is_active) {
				if (mobs[i].y_pos == GameWorld::SIZE_Y - 1) {
					player.cur_hp--;
					mobs[i].Deactivate();
				}
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
			FindDeactivatedBullet();
			break;
		}
	}
}

void DrawWorld() {
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
	MoveCursor(player.x_pos, player.y_pos);
	cout << player.TEXT;
	for (int i = 0; i < GameWorld::MAX_BULLET; i++) {
		if (bullets[i].is_active) {
			MoveCursor(bullets[i].x_pos, bullets[i].y_pos);
			cout << Bullet::TEXT;
		}
	}
	for (int i = 0; i < GameWorld::MAX_MOB; i++) {
		if (mobs[i].is_active) {
			MoveCursor(mobs[i].x_pos, mobs[i].y_pos);
			cout << Mob::TEXT;
		}
	}
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
	MoveCursor(10, 10);
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

int main()
{
	HideCursor();
	while (true) {
		Input();
		Update();
		Collision();
		DrawWorld();
		if (EndGame()) {
			break;
		}
		Sleep(5);
	}
}