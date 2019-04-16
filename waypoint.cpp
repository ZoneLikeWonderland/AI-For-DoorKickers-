#include "playerAI.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#define fx(i, a, b) for (register int i = a; i <= (int)b; i++)
#define rep(i, N) for (register int i = 0; i < (int)N; i++)

// #include "geometry.cpp"
// #include "logic.cpp"


using namespace std;

int waypoint[MAXN][MAXN];

size_t HashIntPoint(IntPoint a) { return a.x * MAXN + a.y; }

// way-point (id -> point)
vector<IntPoint> keypoint;
// way-point (point -> id)
unordered_map<int, int> invkeypoint;
// way-point connection
vector<vector<int>> keyway(MAXM);

int getkeypointid(IntPoint a) { return invkeypoint[HashIntPoint(a)]; }

double Gdistdata[MAXM];
bool visit[MAXM];
int dad[MAXM];
int TEMP[MAXM];

Map map;
// if the init is done
bool DONE = false;
// current destination
Point DEST;

void print(Point a) { cerr << a.x << "," << a.y << '\n'; }
inline bool inbound(IntPoint P) {
	if (P.x >= 0 && P.x < map.width && P.y >= 0 && P.y < map.height)
		return true;
	return false;
}

inline Point StayAwayFromWalls(Point P) {
	int x = round(P.x);
	int y = round(P.y);
	double dx = 0, dy = 0;
	if (inbound(IntPoint(x, y)) && map.pixels[x][y]) {
		dx -= D;
		dy -= D;
	}
	if (inbound(IntPoint(x - 1, y)) && map.pixels[x - 1][y]) {
		dx += D;
		dy -= D;
	}
	if (inbound(IntPoint(x, y - 1)) && map.pixels[x][y - 1]) {
		dx -= D;
		dy += D;
	}
	if (inbound(IntPoint(x - 1, y - 1)) && map.pixels[x - 1][y - 1]) {
		dx += D;
		dy += D;
	}
	return Point(P.x + dx, P.y + dy);
}

inline bool havewalls(Point a, Point b) {
	a = StayAwayFromWalls(a);
	b = StayAwayFromWalls(b);
	if (a.x > b.x)
		swap(a, b);
	int j0 = a.y, j;
	fx(i, a.x + 1, b.x + 1) {
		if (a.x == b.x) {
			j = b.y;
		} else {
			j = (i - a.x) * (a.y - b.y) / (a.x - b.x) + a.y;
			if (j < min(a.y, b.y))
				j = min(a.y, b.y);
			if (j > max(a.y, b.y))
				j = max(a.y, b.y);
		}
		fx(k, min(j0, j), max(j0, j)) {
			if (map.pixels[i - 1][k])
				return true;
		}
		j0 = j;
	}
	return false;
}

const int reducedist=5;
inline IntPoint StayFarAwayFromWalls(IntPoint P) {
	int x = round(P.x);
	int y = round(P.y);
	int dx = 0, dy = 0;
	if (inbound(IntPoint(x, y)) && map.pixels[x][y]) {
		dx -= reducedist;
		dy -= reducedist;
	}
	if (inbound(IntPoint(x - 1, y)) && map.pixels[x - 1][y]) {
		dx += reducedist;
		dy -= reducedist;
	}
	if (inbound(IntPoint(x, y - 1)) && map.pixels[x][y - 1]) {
		dx -= reducedist;
		dy += reducedist;
	}
	if (inbound(IntPoint(x - 1, y - 1)) && map.pixels[x - 1][y - 1]) {
		dx += reducedist;
		dy += reducedist;
	}
	return IntPoint(P.x + dx, P.y + dy);
}

void initwaypoint() {
	freopen("err.txt", "w", stderr);
    
	auto t = clock();
	rep(i, map.width) rep(j, map.height) if (map.pixels[i][j]) {
		waypoint[i][j]++;
		waypoint[i + 1][j]++;
		waypoint[i][j + 1]++;
		waypoint[i + 1][j + 1]++;
	}
	keypoint.reserve(MAXM);
	invkeypoint.reserve(MAXM);
	int k = 0;
	fx(i, 1, map.width - 1) fx(j, 1, map.height - 1) if (waypoint[i][j] == 1) {
		IntPoint temp=StayFarAwayFromWalls(IntPoint(i,j));
		keypoint.push_back(temp);
		invkeypoint[HashIntPoint(temp)] = k++;
	}
	fx(i, 0, keypoint.size() - 2) fx(j, i + 1, keypoint.size() - 1) {
		if (!havewalls(keypoint[i], keypoint[j])) {
			keyway[i].push_back(j);
			keyway[j].push_back(i);
		}
	}
	// cerr << "init time: " << clock() - t << ' ' << keypoint.size() << endl;
	DONE = true;
}

double Hdist(Point s) {
	return sqrt((s.x - DEST.x) * (s.x - DEST.x) +
				(s.y - DEST.y) * (s.y - DEST.y));
}

// only for KeyPoints
double &Gdist(IntPoint s) { return Gdistdata[getkeypointid(s)]; }

// function object of comprasion of Astar points
class AstarCMP {
  public:
	bool operator()(IntPoint a, IntPoint b) const {
		return Hdist(a) + Gdist(a) > Hdist(b) + Gdist(b);
	}
};

// return number of points and a rop
int Astar(Point s, Point t, Point ROP[]) {
	auto T = clock();

	if (!havewalls(s, t)) {
		ROP[0] = t;
		return 1;
	}

	rep(i, keypoint.size()) Gdistdata[i] = INF;
	rep(i, keypoint.size()) visit[i] = false;
	DEST = t;
	priority_queue<IntPoint, vector<IntPoint>, AstarCMP> Q;
	IntPoint P;
	while (true) {
		if (Q.empty()) {
			rep(i, keypoint.size()) {
				if (!havewalls(s, keypoint[i])) {
					Gdist(keypoint[i]) = dist(s, keypoint[i]);
					dad[i] = -1;
					Q.push(keypoint[i]);
					visit[i] = true;
				}
			}
		} else {
			P = Q.top();
			// print(P);
			Q.pop();
			if (!havewalls(P, t))
				break;
			int id = getkeypointid(P);
			rep(i, keyway[id].size()) {
				IntPoint R = keypoint[keyway[id][i]];
				if (!havewalls(P, R)) {
					if (Gdist(R) > Gdist(P) + dist(P, R)) {
						Gdist(R) = Gdist(P) + dist(P, R);
						int sonid = getkeypointid(R);
						dad[sonid] = id;
						if (!visit[sonid]) {
							Q.push(R);
							visit[sonid] = true;
						}
					}
				}
			}
		}
	}
	// cout << "done\n";
	int ROPsize = 0;
	while (true) {
		TEMP[ROPsize++] = getkeypointid(P);
		// print(P);
		if (dad[TEMP[ROPsize - 1]] == -1)
			break;
		P = keypoint[dad[TEMP[ROPsize - 1]]];
	}
	rep(i, ROPsize) {
		ROP[i] = StayAwayFromWalls(keypoint[TEMP[ROPsize - i - 1]]);
	}
	ROP[ROPsize] = t;
	// cerr << "Astar time: " << clock() - T << '\n';
	return ROPsize;
}

// int main() {

// 	auto t = clock();
// 	freopen("map.txt", "r", stdin);
// 	int mapsize;
// 	cin >> mapsize;

// 	map.height = map.width = mapsize;
// 	map.pixels.resize(map.width);
// 	rep(i, map.width) map.pixels[i].resize(map.width);
// 	int a;
// 	rep(i, mapsize) rep(j, mapsize) {
// 		cin >> a;
// 		map.pixels[i][j] = a;
// 	}
// 	// thread init(initwaypoint);
// 	// init.detach();

// 	// cout << clock() - t << endl;
// 	// cout << DONE << endl;

// 	// _sleep(1000);
// 	// cout << DONE << endl;
// 	// cout << "============\n";

// 	initwaypoint();

// 	// rep(i, keypoint.size()) {
// 	// 	cout << keypoint[i].x << ',' << keypoint[i].y << ':';
// 	// 	rep(j, keyway[i].size()) {
// 	// 		cout << keypoint[keyway[i][j]].x << ',' << keypoint[keyway[i][j]].y
// 	// 			 << ' ';
// 	// 	}
// 	// 	cout << '\n';
// 	// }

// 	Point p[50];
// 	int len = Astar(Point(51.7754, 33.4329), Point(54.999, 36.001), p);
// 	cerr << len << endl;
// 	rep(i, len) { print(p[i]); }
// 	return 0;
// }