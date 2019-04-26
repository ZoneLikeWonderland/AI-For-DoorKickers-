#include "playerAI.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>

// #include "geometry.cpp"
// #include "logic.cpp"
// void print(Point a) { cerr << a.x << "," << a.y << '\n'; }
// // fake function
// Human man;
// bool isWall(Point a) { return false; }
// Human GetUnit(int num) { return man; }
// Logic *logic;

#define fx(i, a, b) for (register int i = a; i <= (int)b; i++)
#define rep(i, N) for (register int i = 0; i < (int)N; i++)

extern Human GetEnemyUnit(int num);
extern Human GetUnit(int num);
extern Logic *logic;
extern Human past5frame[5][5];

extern int Astar(Point s, Point t, Point ROP[]);

Point ROP2[MAXM];

Point ForecastPos(int enemy, double consider = 0.8) {
	if (GetEnemyUnit(enemy).flash_time < meteor_delay)
		return Point(-1, -1);
	Point manpos = GetEnemyUnit(enemy).position;
	Point dest;
	if (dist(manpos, logic->map.bonus_places[0]) <
		meteor_delay * human_velocity + meteor_distance) {
		dest = logic->map.bonus_places[0];
		if (dist(manpos, logic->map.bonus_places[0]) <
			meteor_delay * human_velocity)
			return dest;
	} else if (dist(manpos, logic->map.bonus_places[1]) <
			   meteor_delay * human_velocity + meteor_distance) {
		dest = logic->map.bonus_places[1];
		if (dist(manpos, logic->map.bonus_places[1]) <
			meteor_delay * human_velocity)
			return dest;
	} else {
		if (abs(angle(past5frame[(logic->frame - 4) % 5][enemy].position,
					  manpos, logic->map.target_places[logic->faction ^ 1])) <
			PI / 2) {
			dest = logic->map.target_places[logic->faction ^ 1];
		} else {
			dest = logic->map.target_places[logic->faction];
		}
	}
	int roplen = Astar(manpos, dest, ROP2);
	Point u = ROP2[0] - manpos;
	u = u / dist(u, Point(0, 0)) * (meteor_delay * human_velocity * consider);
	return manpos + u;
}

const int disttobonus = 90;
Point ForecastFirePos(int enemy, int self) {
	Point manpos = GetEnemyUnit(enemy).position;
	// Point dest;
	// if (abs(manpos.x - manpos.y) < 40) {
	// 	if (abs(angle(past5frame[(logic->frame - 4) % 5][enemy].position,
	// 				  manpos, logic->map.target_places[logic->faction ^ 1])) <
	// 		PI / 4) {
	// 		dest = logic->map.target_places[logic->faction ^ 1];
	// 	} else {
	// 		dest = logic->map.target_places[logic->faction];
	// 	}
	// } else if (dist(manpos, logic->map.bonus_places[0]) < disttobonus) {
	// 	dest = logic->map.bonus_places[0];
	// } else if (dist(manpos, logic->map.bonus_places[1]) < disttobonus) {
	// 	dest = logic->map.bonus_places[1];
	// } else
	// 	dest = logic->map.target_places[logic->faction];
	// int roplen = Astar(manpos, dest, ROP2);
	// Point u = ROP2[0] - manpos;
	Point u = manpos - past5frame[(logic->frame - 4) % 5][enemy].position;
	double dis = dist(manpos, GetUnit(self).position);
	double co = cos(angle(manpos, manpos + u, GetUnit(self).position));
	double t = (2 * human_velocity * dis * co -
				sqrt(4 * human_velocity * human_velocity * dis * dis * co * co -
					 4 *
						 (human_velocity * human_velocity -
						  fireball_velocity * fireball_velocity) *
						 dis * dis)) /
			   2 /
			   (human_velocity * human_velocity -
				fireball_velocity * fireball_velocity);

	u = u / dist(u, Point(0, 0)) * (t * human_velocity);
	return manpos + u;
}