#include "playerAI.h"
#include "playerbasic.cpp"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <queue>
#include <sstream>
#include <thread>

#include "Astar.cpp"

#define fx(i, a, b) for (register int i = a; i <= (int)b; i++)
#define rep(i, N) for (register int i = 0; i < (int)N; i++)

using namespace std;

enum STATE {
	RushToCrystal,
	RushToEnemyTarget,
	BackToOurTarget,
	RushToBonus,
	ProtectCrystal,
	WanderBouns,
	RandomAction,
	FollowEnemy
} state[5];

string STATEstr[] = {"RushToCrystal", "RushToEnemyTarget", "BackToOurTarget",
					 "RushToBonus",   "ProtectCrystal",	"WanderBouns",
					 "RandomAction",  "FollowEnemy"};

double RandDouble() { return rand() / (double)RAND_MAX; }
double RandDouble(double L, double R) { return RandDouble() * (R - L) + L; }

Human past5frame[5][5];

// Way search
using OriginAstar::move_smart;
extern void initwaypoint();
extern bool DONE;
extern Map map;
extern int Astar(Point s, Point t, Point ROP[]);
Point ROP[MAXM];

// avoid fireball
extern void findthreat(int num);
extern Point dealthreat(int num, Point v, double consider = 0.95);

// forecast position
extern Point ForecastPos(int enemy, double consider = 0.8);
extern Point ForecastFirePos(int enemy, int self);

/* Behavior layer */
// forward to point
void Forward(int num, Point dest, bool flash = true, double consider = 0.95) {
	if (!DONE) {
		Point step;
		move_smart(num, dest, step);
		flash_s(num, step);
	} else {
		Point pos = GetUnit(num).position;
		int roplen = Astar(pos, dest, ROP);
		if (canflash(num) && flash) {
			flash_s(num, bestjump(roplen, ROP, pos, human_velocity));
		} else {
			findthreat(num);
			Point u = ROP[0] - pos;
			Point v = dealthreat(num, u, consider);
			move_relative(num, v);
		}
	}
}

/* Character layer */
// state decide
void Decide() {
	// bonus
	rep(i, 2) {
		if (dist(GetUnit(i).position,
				 GetEnemyUnit(GetNearestEnemy(i)).position) < splash_radius)
			state[i] = FollowEnemy;
		else if (dist(GetUnit(i).position, logic->map.bonus_places[i & 1]) >=
				 bonus_radius - human_velocity)
			state[i] = RushToBonus;
		else {
			state[i] = RandomAction;
			rep(j, 5) if (dist(GetEnemyUnit(j).position,
							   logic->map.bonus_places[i & 1]) <
							  meteor_delay * human_velocity + meteor_distance &&
						  dist(GetEnemyUnit(j).position,
							   logic->map.bonus_places[i & 1]) >
							  meteor_distance - explode_radius &&
						  GetEnemyUnit(j).flash_time < meteor_delay) {
				state[i] = WanderBouns;
				break;
			}
		}
	}

	// crystal
	if (logic->crystal[logic->faction ^ 1].belong == -1) {
		fx(i, 2, 4) { state[i] = RushToCrystal; }
	} else {
		if (dist(logic->crystal[logic->faction ^ 1].position,
				 logic->map.target_places[logic->faction]) <
			dist(logic->crystal[logic->faction ^ 1].position,
				 logic->map.target_places[logic->faction ^ 1]) *
				0.6) {
			fx(i, 2, 4) {
				if (node_retranslate(
						logic->faction,
						logic->crystal[logic->faction ^ 1].belong) == i) {
					state[i] = BackToOurTarget;
				} else {
					state[i] = RushToEnemyTarget;
				}
			}
		} else {
			fx(i, 2, 4) {
				if (node_retranslate(
						logic->faction,
						logic->crystal[logic->faction ^ 1].belong) == i) {
					state[i] = BackToOurTarget;
				} else {
					state[i] = ProtectCrystal;
				}
			}
		}
	}
}

// state evaluate
void Eval(int num) {
	if (num == 3 &&
		dist(GetUnit(3).position, GetUnit(2).position) < splash_radius)
		state[num] = RandomAction;
	if (num == 4 &&
		(dist(GetUnit(4).position, GetUnit(2).position) < splash_radius ||
		 dist(GetUnit(4).position, GetUnit(3).position) < splash_radius))
		state[num] = RandomAction;

	switch (state[num]) {
	case RushToCrystal:
		Forward(num, logic->crystal[logic->faction ^ 1].position);
		break;
	case RushToEnemyTarget:
		Forward(num, logic->map.target_places[logic->faction ^ 1]);
		break;
	case BackToOurTarget:
		Forward(num, logic->map.target_places[logic->faction]);
		break;
	case RushToBonus:
		Forward(num, logic->map.bonus_places[num & 1]);
		break;
	case ProtectCrystal:
		if (dist(GetUnit(num).position,
				 logic->crystal[logic->faction ^ 1].position) >
			flash_distance) {
			Forward(num, logic->crystal[logic->faction ^ 1].position);
		} else {
			Forward(num, logic->map.target_places[logic->faction], false);
		}
		break;
	case WanderBouns: {
		double vv;
		Point v;
		do {
			vv = RandDouble() * 2 * PI;
			v = Point(cos(vv), sin(vv)) * human_velocity;
		} while (isWall(GetUnit(num).position + v));
		Forward(num, GetUnit(num).position + v, false, -1);
		break;
	}
	case RandomAction: {
		double vv;
		Point v;
		do {
			vv = RandDouble() * 2 * PI;
			v = Point(cos(vv), sin(vv)) * human_velocity;
		} while (isWall(GetUnit(num).position + v));
		Forward(num, GetUnit(num).position + v, false, -1);
		break;
	}
	case FollowEnemy:
		// Forward(num, GetEnemyUnit(GetNearestEnemy(num)).position, true, -1);
		move_relative(num, GetEnemyUnit(GetNearestEnemy(num)).position -
							   GetUnit(num).position);
		break;
	}
}

Point target[5];
void Think() {
	for (int i = 0; i < 5; i++) {
		target[i] = Point(-1, -1);
		if (state[i] == WanderBouns)
			continue;
		Point mypos = GetUnit(i).position;
		int nearest = GetNearestEnemy(i);
		target[i] = ForecastFirePos(nearest, i);
	}
}

void playerAI() {
	stringstream ss;
	auto t0 = clock();
	auto t = t0;

	logic = Logic::Instance();
	rep(i, 5) past5frame[logic->frame % 5][i] = GetEnemyUnit(i);

	static bool ONCE = true;

	if (ONCE) {
		map = logic->map;
		thread th(initwaypoint);
		th.detach();
		ONCE = false;
		srand(time(NULL));
	}

	Decide();
	rep(i, 5) Eval(i);

	Think();
	rep(i, 5) {
		if (state[i] == WanderBouns)
			continue;
		double D = 3;
		logic->shoot(i, Point(target[i].x + RandDouble(-D, D),
							  target[i].y + RandDouble(-D, D)));
	}

	bool used[5] = {false, false, false, false, false};
	rep(i, 5) {
		rep(j, 5) {
			if (dist(GetUnit(i).position, GetEnemyUnit(j).position) <
				(meteor_delay * human_velocity) + meteor_distance -
					(i < 2 ? 2 : 0)) {
				Point target =
					ForecastPos(j, (state[i] == WanderBouns ? 1 : 0.8));
				target.x += RandDouble(-.5, .5);
				target.y += RandDouble(-.5, .5);
				if (!used[i] && target != Point(-1, -1) &&
					dist(target, GetUnit(i).position) < meteor_distance) {
					logic->meteor(i, target);
					used[i] = true;
				}
			}
		}
	}

	rep(i, 5) ss << i << ": " << STATEstr[state[i]] << "  ";
	logic->debug(ss.str());
}