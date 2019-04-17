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

extern Human GetUnit(int num);
extern Logic *logic;
extern bool isWall(Point v);

const int DangerDist = 18;
const int DangerFrames = 6;
const int nD = 10;
vector<vector<Fireball>> coming(5);

void findthreat(int num) {
	Point manpos = GetUnit(num).position;
	coming[num].clear();
	for (auto x : logic->fireballs) {
		if (x.from_number % logic->map.faction_number == logic->faction)
			continue;
		if (abs(angle(x.position, manpos,
					  x.position + Point(cos(x.rotation), sin(x.rotation)))) <
			PI / 2)
			if (dist(manpos, x.position) < DangerDist) {
				// when vt+3>d,should u(t-1)>length.(length^2+d^2==dist^2)
				double Dist = dist(manpos, x.position);
				double d = ptoldist(
					manpos,
					Lineseg(x.position, x.position + Point(cos(x.rotation),
														   sin(x.rotation))));
				double length = sqrt(Dist * Dist - d * d);
				if (fireball_velocity * ((d - 3) / human_velocity - 1) <
					length * 2) {
					coming[num].push_back(x);
					// cout<<"is\n";
					// cout<<x.position.x<<','<<x.position.y<<endl;
				}
			}
	}
}

// give relative vector v
vector<pair<Point, int>> LR;
vector<bool> lforbid, rforbid;
vector<Point> lsecond, rsecond;
vector<Point> lfirst, rfirst;
const Point p0(-1, -1);
Point center, dir;

bool anglesmall(pair<Point, int> a, pair<Point, int> b) {
	return angle(center, center + dir, a.first) >
		   angle(center, center + dir, b.first);
}

// pass a vector and return a vector
Point dealthreat(int num, Point v) {
	if (coming[num].empty())
		return v;
	// if (num == 4) {
	// 	fstream f("1.txt", ios::app);
	// 	f << "\npos: " << GetUnit(num).position.x << ','
	// 	  << GetUnit(num).position.y << '\n';
	// 	f << "old: " << v.x << ',' << v.y << '\n';
	// 	f << "find\n";
	// 	for (auto x : coming[num]) {
	// 		f << x.position.x - GetUnit(num).position.x << ','
	// 		  << x.position.y - GetUnit(num).position.y << "," << x.rotation
	// 		  << "  ";
	// 		f << x.position.x << ',' << x.position.y << "," << x.rotation
	// 		  << '\n';
	// 	}
	// }

	Point manpos = GetUnit(num).position;
	LR.clear();
	lforbid.clear();
	rforbid.clear();
	lsecond.clear();
	rsecond.clear();
	lfirst.clear();
	rfirst.clear();
	lforbid.resize(coming[num].size(), true);
	rforbid.resize(coming[num].size(), true);
	lsecond.resize(coming[num].size(), p0);
	rsecond.resize(coming[num].size(), p0);
	lfirst.resize(coming[num].size(), p0);
	rfirst.resize(coming[num].size(), p0);
	bool lf = false, rf = false;
	Point temp;
	rep(t, DangerFrames) {
		rep(i, coming[num].size()) {
			// for(auto &f:coming[num]){
			Fireball &f = coming[num][i];
			double ro = f.rotation;
			Point p = f.position;
			Point q = p + Point(cos(ro) * fireball_velocity,
								sin(ro) * fireball_velocity);

			Point l = Point(cos(ro - PI / 2) * (fireball_radius + D),
							sin(ro - PI / 2) * (fireball_radius + D));
			Point r = Point(cos(ro + PI / 2) * (fireball_radius + D),
							sin(ro + PI / 2) * (fireball_radius + D));
			Point lp = p + l;
			Point lq = q + l;
			Point rp = p + r;
			Point rq = q + r;

			if (ptoLinesegdist(manpos, Lineseg(lp, lq), temp) <=
				t * human_velocity - nD * D) {
				if (lforbid[i]) {
					lfirst[i] = firstcrosscircle(
						manpos, t * human_velocity - nD * D, Lineseg(lp, lq));
				}
				lforbid[i] = false;

				lsecond[i] = firstcrosscircle(
					manpos, t * human_velocity - nD * D, Lineseg(lq, lp));
			}
			if (ptoLinesegdist(manpos, Lineseg(rp, rq), temp) <=
				t * human_velocity - nD * D) {
				if (rforbid[i]) {
					rfirst[i] = firstcrosscircle(
						manpos, t * human_velocity - nD * D, Lineseg(rp, rq));
				}
				rforbid[i] = false;

				rsecond[i] = firstcrosscircle(
					manpos, t * human_velocity - nD * D, Lineseg(rq, rp));
			}

			f.position = q;
		}
	}

	int level = 0;
	Point base =
		firstcrosscircle(manpos, human_velocity, Lineseg(manpos, manpos + v));

	// print(base);
	rep(i, coming[num].size()) {
		if (lfirst[i] != p0 && rfirst[i] != p0) {
			if (ptoldist(base, Lineseg(lfirst[i], lsecond[i])) +
					ptoldist(base, Lineseg(rfirst[i], rsecond[i])) <=
				fireball_radius + fireball_radius + 4 * D)
				level++;
		} else {
			if (rfirst[i] != p0) {
				if (multiply(rfirst[i], base, rsecond[i]) > 0)
					level++;
			}
			if (lfirst[i] != p0) {
				if (multiply(lfirst[i], base, lsecond[i]) < 0)
					level++;
			}
		}

		if (lfirst[i] != p0)
			LR.push_back(make_pair(lfirst[i], 0));
		if (rfirst[i] != p0)
			LR.push_back(make_pair(rfirst[i], 1));
		if (lsecond[i] != p0)
			LR.push_back(make_pair(lsecond[i], 1));
		if (rsecond[i] != p0)
			LR.push_back(make_pair(rsecond[i], 0));
	}
	// cout << level << '\n';
	for (auto f : lforbid)
		lf = lf || f;
	for (auto f : rforbid)
		rf = rf || f;

	// if (lf)
	// 	cout << "no left\n";
	// if (rf)
	// 	cout << "no right\n";

	center = manpos;
	dir = v;
	Point w = -v; // vector
	LR.push_back(make_pair(manpos + v, -1));
	sort(LR.begin(), LR.end(), anglesmall);

	// for (auto x : LR) {
	// 	cout << x.first.x - manpos.x << ',' << x.first.y - manpos.y << ' ';
	// 	cout << x.second << '\n';
	// }
	// if (num == 4) {
	// 	fstream f("1.txt", ios::app);
	// 	f << "LR:\n";
	// 	for (auto x : LR) {
	// 		// print(x.first);
	// 		f << x.first.x - manpos.x << ',' << x.first.y - manpos.y << ' ';
	// 		f << x.second << '\n';
	// 		// cout<<angle(center, center + dir, x.first)<<endl;
	// 	}
	// }

	for (auto x : LR) {
		if (x.second == -1 && level == 0) {
			// if (num == 4) {
			// 	fstream f("1.txt", ios::app);
			// 	f << "No alert\n";
			// }
			return v;
		}
		// print(x.first);
		// cout<<level<<'\n';
		if (x.second == 0) {
			if (level == 0) {
				if (!isWall(x.first)) {
					if (abs(angle(manpos, manpos + v, x.first)) <
						abs(angle(manpos, manpos + v, manpos + w))) {
						w = x.first - manpos;
					}
				}
			}
			level++;
		} else if (x.second == 1) {
			level--;
			if (level == 0) {
				if (!isWall(x.first)) {
					if (abs(angle(manpos, manpos + v, x.first)) <
						abs(angle(manpos, manpos + v, manpos + w))) {
						w = x.first - manpos;
					}
				}
			}
		}
	}
	// print(w);
	if (abs(angle(manpos, manpos + v, manpos + w)) > 0.95 * PI)
		return v;
	// if (num == 4) {
	// 	fstream f("1.txt", ios::app);
	// 	f << "new: " << w.x << ',' << w.y << '\n';
	// }
	return w;
}

// int main() {
// 	logic = Logic::Instance();
// 	man = Human(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
// 	logic->map.faction_number = 2;
// 	// logic->faction = 0;
// 	logic->fireballs.push_back(Fireball(0, 16, 1.5 * PI, -1));
// 	// logic->fireballs.push_back(Fireball(4, 18, 1.5 * PI, -1));
// 	// logic->fireballs.push_back(Fireball(18, 0, 1 * PI, -1));
// 	// logic->fireballs.push_back(Fireball(18, -1, 1 * PI, -1));
// 	findthreat(4);
// 	print(dealthreat(4, Point(0, -1)));

// 	return 0;
// }