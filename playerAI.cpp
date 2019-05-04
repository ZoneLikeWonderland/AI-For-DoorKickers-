#include "playerAI.h"
#include "playerbasic.cpp"
#include <algorithm>
#include <cmath>
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
	// WanderBouns,
	RandomAction,
	FollowEnemy,
	RandomButStayAway,
	AvoidMeteor
} state[5];

// const int DontShoot = 1 << 0;
// int ExtraState[5];

string STATEstr[] = {"RushToCrystal", "RushToEnemyTarget", "BackToOurTarget",
					 "RushToBonus",   "ProtectCrystal", //	"WanderBouns",
					 "RandomAction",  "FollowEnemy",	   "RandomButStayAway",
					 "AvoidMeteor"};

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
			dest = flash_s(num, bestjump(roplen, ROP, pos, flash_distance));
			for (auto x : logic->meteors) {
				if (dist(dest, x.position) < explode_radius &&
					x.last_time <= 7) {
					dest = firstcrosscircle(x.position, explode_radius + 1,
											Lineseg(pos, dest));
					flash_s(num, dest);
				}
			}
		} else {
			findthreat(num);
			Point u = ROP[0] - pos;
			Point v = dealthreat(num, u, consider);
			move_relative(num, v);
		}
	}
}

/* Character layer */
int MainRole = 2;
// state decide
void Decide() {
	// rep(i, 5) ExtraState[i] = 0;
	// bonus
	rep(i, 2) {
		int edid = GetNearestEnemy(i, true);
		int eid = GetNearestEnemy(i);
		if (dist(GetUnit(i).position, GetEnemyUnit(edid).position) <
				meteor_distance &&
			GetEnemyUnit(edid).meteor_time <
				human_meteor_interval - meteor_delay + 8 &&
			GetEnemyUnit(edid).meteor_time >=
				human_meteor_interval - meteor_delay - 3 &&
			dist(GetUnit(i).position, logic->map.bonus_places[i & 1]) <
				bonus_radius) {
			state[i] = RandomButStayAway;
		} else if (dist(GetUnit(i).position, GetEnemyUnit(eid).position) <
					   splash_radius &&
				   dist(GetEnemyUnit(eid).position,
						past5frame[(logic->frame + 1) % 5][eid].position) > D)
			state[i] = FollowEnemy;
		else if (dist(GetUnit(i).position, logic->map.bonus_places[i & 1]) >=
				 bonus_radius - human_velocity)
			state[i] = RushToBonus;
		else {
			state[i] = RandomAction;
		}
		// if (dist(GetEnemyUnit(eid).position, logic->map.bonus_places[i & 1])
		// < 		(meteor_delay * human_velocity + meteor_distance) *
		// (1.1
		// + human_velocity / fireball_velocity) &&
		// 	dist(GetEnemyUnit(eid).position, logic->map.bonus_places[i & 1]) >
		// 		meteor_distance - explode_radius &&
		// 	GetEnemyUnit(eid).flash_time < meteor_delay) {
		// 	ExtraState[i] += DontShoot;
		// }
	}

	// crystal
	if (logic->crystal[logic->faction ^ 1].belong == -1) {
		fx(i, 2, 4) { state[i] = RushToCrystal; }
	} else {
		MainRole = node_retranslate(logic->faction,
									logic->crystal[logic->faction ^ 1].belong);
		if (dist(logic->crystal[logic->faction ^ 1].position,
				 logic->map.target_places[logic->faction]) <
			dist(logic->crystal[logic->faction ^ 1].position,
				 logic->map.target_places[logic->faction ^ 1]) *
				0.6) {
			fx(i, 2, 4) {
				if (MainRole == i) {
					state[i] = BackToOurTarget;
				} else {
					state[i] = RushToEnemyTarget;
				}
			}
		} else {
			fx(i, 2, 4) {
				if (MainRole == i) {
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
	double times = 2;
	if (num >= 2 && MainRole != num && GetUnit(MainRole).hp > 0 &&
		dist(GetUnit(num).position, GetUnit(MainRole).position) <
			splash_radius * times)
		state[num] = RandomAction;

	int big, small;
	if (MainRole == 2) {
		big = 3;
		small = 4;
	} else if (MainRole == 3) {
		big = 2;
		small = 4;
	} else {
		big = 2;
		small = 3;
	}
	if (num == small && GetUnit(big).hp > 0 &&
		dist(GetUnit(num).position, GetUnit(big).position) <
			splash_radius * times)
		state[num] = RandomAction;

	for (auto x : logic->meteors) {
		if (dist(GetUnit(num).position, x.position) < explode_radius + 1 &&
			x.last_time <= 7) {
			state[num] = AvoidMeteor;
		}
	}

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
		move_relative(num, GetEnemyUnit(GetNearestEnemy(num)).position -
							   GetUnit(num).position);
		break;
	case RandomButStayAway: {
		if (dist(GetUnit(num).position, logic->map.bonus_places[num & 1]) <
			explode_radius * 1.1) {
			if (dist(GetUnit(num).position, logic->map.bonus_places[num & 1]) <
				D) {
				double vv;
				Point v;
				vv = RandDouble() * 2 * PI;
				v = Point(cos(vv), sin(vv)) * explode_radius * 1;
				move_relative(num, v);
				// Forward(num, GetUnit(num).position + v, true, -1);
				break;
			}
			move_relative(num, (GetUnit(num).position -
								logic->map.bonus_places[num & 1]) /
								   dist(GetUnit(num).position,
										logic->map.bonus_places[num & 1]) *
								   (explode_radius * 1.1 -
									dist(GetUnit(num).position,
										 logic->map.bonus_places[num & 1]) +
									D));
			break;
		}
		double vv;
		Point v;
		do {
			vv = RandDouble() * 2 * PI;
			v = Point(cos(vv), sin(vv)) * human_velocity;
		} while (
			isWall(GetUnit(num).position + v) ||
			dist(GetUnit(num).position + v, logic->map.bonus_places[num & 1]) <=
				explode_radius * 1.1 ||
			dist(GetUnit(num).position + v, logic->map.bonus_places[num & 1]) >=
				bonus_radius);
		move_relative(num, v);
		break;
	}
	case AvoidMeteor: {
		Point pos = GetUnit(num).position;
		for (auto x : logic->meteors) {
			if (dist(pos, x.position) < explode_radius && x.last_time <= 7) {
				if (dist(pos, x.position) < D) {
					double vv;
					Point v;
					vv = RandDouble() * 2 * PI;
					v = Point(cos(vv), sin(vv)) * explode_radius * 1;
					move_relative(num, v);
					break;
				}
				move_relative(num, (pos - x.position) / dist(pos, x.position) *
									   (explode_radius * 1.1 -
										dist(pos, x.position) + D));
				break;
			}
			if (dist(pos, x.position) < explode_radius + 1 &&
				x.last_time <= 7) {
				double vv;
				Point v;
				int t = 0;
				while (1) {
					t++;
					vv = RandDouble() * 2 * PI;
					v = Point(cos(vv), sin(vv)) * human_velocity;
					bool flag = true;
					for (auto y : logic->meteors) {
						if (dist(y.position, pos + v) <= explode_radius)
							flag = false;
					}
					if (!isWall(pos + v) && flag || t > 50) {
						Forward(num, pos + v, false);
						break;
					}
				}
				break;
			}
		}
	}
	}
}

Point target[10];
void Think() {
	for (int i = 0; i < 5; i++) {
		target[i] = Point(-1, -1);
		// if (ExtraState[i] & DontShoot)
		// 	continue;
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
	ss << logic->frame << "@";
	logic->debug(ss.str());

	rep(i, 5) past5frame[logic->frame % 5][i] = GetEnemyUnit(i);

	static bool ONCE = true;

	if (ONCE) {
		map = logic->map;
		thread th(initwaypoint);
		th.detach();
		ONCE = false;
		srand(time(NULL));
	}

	ss << " $init " << clock() - t << " ";
	t = clock();
	logic->debug(ss.str());

	Decide();

	ss << "$decide " << clock() - t << " ";
	t = clock();
	logic->debug(ss.str());

	rep(i, 5) Eval(i);

	ss << "$eval " << clock() - t << " ";
	t = clock();
	logic->debug(ss.str());

	Think();

	ss << "$think " << clock() - t << " ";
	t = clock();
	logic->debug(ss.str());

	rep(i, 5) {
		double E = dist(target[i], GetUnit(i).position) * 0.2;
		logic->shoot(i, Point(target[i].x + RandDouble(-D, D),
							  target[i].y + RandDouble(-D, D)));

		// logic->shoot(i, target[i]);
	}

	ss << "$shoot " << clock() - t << " ";
	t = clock();
	logic->debug(ss.str());

	bool used[5] = {false, false, false, false, false};
	rep(i, 5) {
		rep(j, 5) {
			if (dist(GetUnit(i).position, GetEnemyUnit(j).position) <
				(meteor_delay * human_velocity) + meteor_distance -
					(i < 2 ? 2 : 0)) {
				Point target = ForecastPos(j, 0.85);
				// ForecastPos(j, (ExtraState[i] & DontShoot ? 1 : 0.8));
				if (target == Point(-1, -1))
					continue;
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

	ss << "$meteor " << clock() - t << " ";
	t = clock();
	logic->debug(ss.str());

	rep(i, 5) ss << i << ": " << GetUnit(i).position.x << ","
				 << GetUnit(i).position.y << " " << STATEstr[state[i]] << "  ";
	logic->debug(ss.str());

	ss << '\n';
	ss << logic->meteors.size() << ":\n";
	for (auto x : logic->meteors)
		ss << x.position.x << ',' << x.position.y << ' ' << x.last_time << '\n';
	logic->debug(ss.str());

	// ofstream f("1.txt", ios::app);
	// f << logic->meteors.size() << '\n';
}