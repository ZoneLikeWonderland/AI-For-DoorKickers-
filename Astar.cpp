#include "playerAI.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>

#define fx(i, a, b) for (register int i = a; i <= (int)b; i++)
#define rep(i, N) for (register int i = 0; i < (int)N; i++)


// #include "logic.cpp"
// #include "geometry.cpp"
// #include "playerAI.cpp"

using namespace std;

extern void move_relative(int num, Point v);
extern Human GetUnit(int num);

namespace OriginAstar {

const double D = 1E-4;
const int MAXN = 330;
Map map;
Point DEST;

void print(Point a) { cout << a.x << "," << a.y << '\n'; }

// points with integer coordinates
class IntPoint {
  public:
	int x, y;
	IntPoint() {}
	IntPoint(Point a) : x(int(a.x)), y(int(a.y)) {}
	IntPoint(int a, int b) : x(a), y(b) {}
	friend IntPoint operator+(IntPoint a, IntPoint b) {
		return IntPoint(a.x + b.x, a.y + b.y);
	}
	friend IntPoint operator-(IntPoint a, IntPoint b) {
		return IntPoint(a.x - b.x, a.y - b.y);
	}
};

// return H dist of a point
double Hdist(IntPoint s) {
	return sqrt(((double)s.x + 0.5 - DEST.x) * ((double)s.x + 0.5 - DEST.x) +
				((double)s.y + 0.5 - DEST.y) * ((double)s.y + 0.5 - DEST.y));
}

// direction vectors
IntPoint DIR[] = {IntPoint(-1, 0), IntPoint(0, 1), IntPoint(1, 0),
				  IntPoint(0, -1)};

Point GetPointByDirection(IntPoint P, int dir) {
	// 0        1
	//   P----+
	//   |    |
	//   +----+
	// 3        2
	switch (dir % 4) {
	case 0:
		return Point(P.x, P.y);
	case 1:
		return Point(P.x, P.y + 1);
	case 2:
		return Point(P.x + 1, P.y + 1);
	case 3:
		return Point(P.x + 1, P.y);
	}
	return Point(-1, -1);
}

// current cost
vector<vector<double>> Gdist;

// is visited
vector<vector<bool>> VST;

// previous direction
vector<vector<int>> DAD;

// function object of comprasion of Astar points
class AstarCMP {
  public:
	bool operator()(IntPoint a, IntPoint b) {
		return Hdist(a) + Gdist[a.x][a.y] > Hdist(b) + Gdist[b.x][b.y];
	}
};

// judge the boundary
bool inbound(IntPoint P) {
	if (P.x >= 0 && P.x < map.width && P.y >= 0 && P.y < map.height)
		return true;
	return false;
}

Point StayAwayFromWalls(Point P) {
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

// int temp[MAXN][MAXN];
// int scale = 1;
// void ReduceMap() {
// 	rep(i, map.width) {
// 		rep(j, map.height) {
// 			if (map.pixels[i][j])
// 				temp[i / scale][j / scale] = 1;
// 		}
// 	}
// 	map.width /= scale;
// 	map.height /= scale;
// 	rep(i, map.width) rep(j, map.height) map.pixels[i][j] = temp[i][j];
// }
// Point bigger(Point a) {
// 	return Point(a.x * (double)scale, a.y * (double)scale);
// }
// Point smaller(Point a) {
// 	return Point(a.x / (double)scale, a.y / (double)scale);
// }

Point FindFirstNode(Point Start, Point Final) {
	// Start = smaller(Start);
	// Final = smaller(Final);

	Point t = Start;
	Point s = Final;

	static bool FIRST = true;
	if (FIRST) {
		FIRST = false;
		map = Logic::Instance()->map;
		// ReduceMap();
	}
	Gdist.clear();
	Gdist.resize(map.width);
	rep(i, map.width) { Gdist[i].resize(map.height, INF); }
	VST.clear();
	VST.resize(map.width);
	rep(i, map.width) { VST[i].resize(map.height, false); }
	DAD.clear();
	DAD.resize(map.width);
	rep(i, map.width) { DAD[i].resize(map.height, -1); }

	Gdist[int(s.x)][int(s.y)] = 0;
	DEST = t;
	// cout<<DEST.x<<','<<DEST.y<<'\n';
	priority_queue<IntPoint, vector<IntPoint>, AstarCMP> Q;
	Q.push(IntPoint(int(s.x), int(s.y)));
	while (!Q.empty()) {
		IntPoint P = Q.top();
		// cout<<P.x<<P.y<<'\n';
		if (P.x == int(DEST.x) && P.y == int(DEST.y)) {
			// cout<<"find!\n";
			break;
		}
		Q.pop();
		rep(d, 4) {
			auto T = P + DIR[d];
			if (inbound(T) && !map.pixels[T.x][T.y]) {
				// cout<<"try "<<T.x<<","<<T.y<<":"<<Gdist[T.x][T.y]<<'\n';
				if (Gdist[T.x][T.y] > Gdist[P.x][P.y] + 1) {
					Gdist[T.x][T.y] = Gdist[P.x][P.y] + 1;
					DAD[T.x][T.y] = d;
					if (!VST[T.x][T.y]) {
						VST[T.x][T.y] = true;
						// cout<<"add
						// "<<T.x<<','<<T.y<<':'<<Hdist(T)+Gdist[T.x][T.y]<<'\n';
						Q.push(T);
					}
				}
			}
		}
	}
	IntPoint X = DEST;

	deque<IntPoint> ROP;
	deque<int> NEXT;

	while (!(X.x == int(s.x) && X.y == int(s.y))) {
		// cout<<X.x<<","<<X.y<<'\n';
		NEXT.push_back((DAD[X.x][X.y] + 2) % 4);
		ROP.push_back(X);
		X = X - DIR[DAD[X.x][X.y]];
	}

	// print the way
	// for(auto x:ROP)
	// cout<<x.x<<','<<x.y<<'\t';
	// cout<<'\n';
	// for(auto x:NEXT)
	// cout<<x<<'\t';
	// cout<<'\n';

	// trace turning cross
	Point op(Start);
	Point left(GetPointByDirection(IntPoint(Start), NEXT[0]));
	Point right(GetPointByDirection(IntPoint(Start), NEXT[0] + 1));
	Point ANS;
	bool straight = true;
	fx(i, 1, ROP.size() - 1) {
		Point left2(GetPointByDirection(ROP[i], NEXT[i]));
		Point right2(GetPointByDirection(ROP[i], NEXT[i] + 1));

		if (multiply(left2, left, op) > 0) {
			if (multiply(left2, right, op) > 0) {
				ANS = right;
				// print(right);
				straight = false;
				break;
			} else {
				left = left2;
			}
		}
		if (multiply(right2, right, op) < 0) {
			if (multiply(right2, left, op) < 0) {
				ANS = left;
				// print(left);
				straight = false;
				break;
			} else {
				right = right2;
			}
		}
	}
	if (straight) {
		if (multiply(Final, right, op) > 0)
			ANS = right;
		else if (multiply(Final, left, op) < 0)
			ANS = left;
		else
			// return bigger(Final);
			return Final;
	}

	// return bigger(StayAwayFromWalls(ANS));
	return StayAwayFromWalls(ANS);
}

void move_smart(int num, Point v, Point &step) {
	move_relative(num, (step = FindFirstNode(GetUnit(num).position, v)) -
						   GetUnit(num).position);
}

} // namespace OriginAstar
  // #ifndef _ALL_
  // int main(){
  //     freopen("1.txt","r",stdin);
  //     int n=5;
  //     map.width=map.height=n;
  //     map.pixels.resize(n);
  //     rep(i,n)
  //     rep(j,n){
  //         int a;
  //         cin>>a;
  //         map.pixels[i].push_back(a);
  //     }

//     print(FindFirstNode(Point(2.5,0.5),Point(2,4.99)));
//     // print(FindFirstNode(Point(0.99,4.01),Point(4,3)));

//     return 0;
// }
// #endif