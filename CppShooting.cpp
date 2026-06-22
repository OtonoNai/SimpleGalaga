#include <iostream>
#include <windows.h>
#include <conio.h>
#include <random>

using namespace std;

random_device rd;
mt19937_64 gen(rd());

const int max_hp = 5;
int cur_hp = max_hp;
int score = 0;

const int world_x = 40;
const int world_y = 20;
char world[world_y][world_x];

const char mob_text = 'X';
int mob_flag = 0;
int mob[5][2] = { {0,0},{0,0}, {0,0}, {0,0}, {0,0} };
int mob_speed[5][2] = { {0,0},{0,0}, {0,0}, {0,0}, {0,0} }; //[speed][stop]
int mob_genspeed = 0;
int mob_genstop = 0;
enum mob_active {
	mob1 = 1 << 0,
	mob2 = 1 << 1,
	mob3 = 1 << 2,
	mob4 = 1 << 3,
	mob5 = 1 << 4,
};

const char player_text = 'A';
int player_x = world_x / 2;
int player_y = world_y - 2;

const char bullet_text = '^';
int bullet_flag = 0;
int bullet[3][2] = { {0,0}, {0,0}, {0,0} };
const int bullet_speed = 3;
int bullet_stop = 0;
enum bullet_active
{
	bullet1 = 1 << 0,
	bullet2 = 1 << 1,
	bullet3 = 1 << 2,
};

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

void AddBullet() {
	for (int i = 0; i < 3; i++) {
		if (!(bullet_flag & (1 << i))) {
			bullet_flag |= (1 << i);

			bullet[i][0] = player_x;
			bullet[i][1] = player_y - 1;

			break;
		}
	}
}

void BulletControl() {
	if (bullet_stop < bullet_speed) {
		bullet_stop++;
		return;
	}
	for (int i = 0; i < 3; i++) {
		if (bullet_flag & (1 << i)) {
			bullet[i][1]--;
		}
		if (bullet[i][1] == 0) {
			bullet_flag &= ~(1 << i);
		}
	}
	bullet_stop = 0;
}

void AddMob() {
	if (mob_genstop <= mob_genspeed) {
		mob_genstop++;
		return;
	}
	
	for (int i = 0; i < 5; i++) {
		if (!(mob_flag & (1 << i))) {
			uniform_int_distribution<int> distrib(1, world_x - 2);
			uniform_int_distribution<int> gentime(50, 60);
			uniform_int_distribution<int> speed(30, 50);
			mob_flag |= (1 << i);
			mob[i][0] = distrib(gen);
			mob[i][1] = 0;
			mob_speed[i][0] = speed(gen);
			mob_genspeed = gentime(gen);
			return;
		}
	}
}

void MobController() {
	for (int i = 0; i < 5; i++) {
		if (!(mob_flag & (1 << i))) {
			continue;
		}

		if (mob_speed[i][1] <= mob_speed[i][0]) {
			mob_speed[i][1]++;
			continue;
		}
		else {
			mob[i][1]++;
			mob_speed[i][1] = 0;
		}

		if ((mob[i][1] >= player_y) && (mob[i][0] == player_x)) {
			cur_hp--;
			mob_flag &= ~(1 << i);
		}

		if (mob[i][1] >= 20) {
			mob_flag &= ~(1 << i);
		}
	}
}

void Collision() {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 3; j++) {
			if ((bullet_flag & 1 << j) && (bullet[j][1] == mob[i][1]) && (bullet[j][0] == mob[i][0])) {
				mob_flag &= ~(1 << i);
				bullet_flag &= ~(1 << j);
				score++;
				break;
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
			if (player_x == 1) {
				return;
			}
			player_x--;

			break;

		case 'd':
			if (player_x == world_x - 2) {
				return;
			}
			player_x++;

			break;
		case 'w':
			AddBullet();
			break;
		}
	}
}

void DrawWorld() {
	MoveCursor(0, 0);
	for (int y = 0; y < world_y; y++) {
		for (int x = 0; x < world_x; x++) {
			world[y][x] = ' ';
			if ((y == 0 || y == world_y - 1) || (x == 0 || x == world_x - 1)) {
				world[y][x] = '*';
			}
			cout << world[y][x];
		}
		cout << endl;
	}
	MoveCursor(player_x, player_y);
	cout << player_text;
	for (int f = 0; f < 3; f++) {
		if (bullet_flag & (1 << f)) {
			MoveCursor(bullet[f][0], bullet[f][1]);
			cout << bullet_text;
		}
	}
	for (int m = 0; m < 5; m++) {
		if (mob_flag & (1 << m)) {
			MoveCursor(mob[m][0], mob[m][1]);
			cout << mob_text;
		}
	}
	MoveCursor(0, 21);
	cout << "Score : " << score << endl;
	cout << "HP : " << cur_hp << endl;
	//cout << bullet_flag;
}

bool EndGame() {
	if (cur_hp > 0) {
		return false;
	}
	MoveCursor(0, 0);
	for (int y = 0; y < world_y; y++) {
		for (int x = 0; x < world_x; x++) {
			world[y][x] = ' ';
			if ((y == 0 || y == world_y - 1) || (x == 0 || x == world_x - 1)) {
				world[y][x] = '*';
			}
			cout << world[y][x];
		}
		cout << endl;
	}
	MoveCursor(10, 10);
	cout << "GAME OVER";
	MoveCursor(0, 20);
	return true;
}

int main()
{
	HideCursor();
	while (true) {
		AddMob();
		Input();
		MobController();
		BulletControl();
		Collision();
		DrawWorld();
		if (EndGame()) {
			break;
		}
		Sleep(5);
	}

}