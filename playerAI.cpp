#include "playerAI.h"
#include<queue>
#include<cstring>
#include<algorithm>
#include<ctime>
#include<fstream>
#include<thread>

#include "Astar.cpp"

#define fx(i, a, b) for (register int i = a; i <= (int)b; i++)
#define rep(i, N) for (register int i = 0; i < (int)N; i++)


using namespace std;

Logic * logic;

int node_translate(int faction,int num)
{
    return num*logic->map.faction_number+faction;
}
Human GetUnit(int num)
{
    return logic->humans[node_translate(logic->faction,num)];
}
Human GetEnemyUnit(int num)
{
    return logic->humans[node_translate(logic->faction^1,num)];
}
Human GetUnit(int faction,int num)
{
    return logic->humans[node_translate(faction,num)];
}
bool isWall(int x,int y)
{
    if(x<0||y<0||x>=logic->map.width||y>=logic->map.height)return true;
    return logic->map.pixels[x][y];
}
bool isWall(Point v)
{
    return isWall(v.x,v.y);
}

void move(int num,Point v)
{
    if(!isWall(v))logic->move(num,v);
}

// move with vector v
void move_relative(int num,Point v)
{
    double mod2=v.x*v.x+v.y*v.y;
    if(mod2>(CONST::human_velocity-0.0001)*(CONST::human_velocity-0.0001))
    {
        mod2=sqrt(mod2);
        mod2=(CONST::human_velocity-0.0001)/mod2;
        v.x*=mod2;
        v.y*=mod2;
    }
    Point pos=GetUnit(num).position;
    v.x+=pos.x;
    v.y+=pos.y;
    move(num,v);
}
void move_s(int num, Point v)
{
    Point pos=GetUnit(num).position;
    v.x-=pos.x;
    v.y-=pos.y;
    move_relative(num,v);
}

bool canflash(int num)
{
    return GetUnit(num).flash_time<=0&&logic->crystal[logic->faction^1].belong!=node_translate(logic->faction,num);
}
void flash(int num,Point v)
{
    if(!canflash(num))return;
    if(isWall(v))return;
    logic->flash(num);
    logic->move(num,v);
}
void flash_relative(int num,Point v)
{
    double mod2=v.x*v.x+v.y*v.y;
    if(mod2>(CONST::flash_distance-0.0001)*(CONST::flash_distance-0.0001))
    {
        mod2=sqrt(mod2);
        mod2=(CONST::flash_distance-0.0001)/mod2;
        v.x*=mod2;
        v.y*=mod2;
    }
    Point pos=GetUnit(num).position;
    v.x+=pos.x;
    v.y+=pos.y;
    flash(num,v);
}
void flash_s(int num, Point v)
{
    Point pos=GetUnit(num).position;
    v.x-=pos.x;
    v.y-=pos.y;
    flash_relative(num,v);
}
double RandDouble(){return rand()/(double)RAND_MAX;}
double RandDouble(double L,double R){return RandDouble()*(R-L)+L;}


// Way search
using OriginAstar::move_smart;

extern void initwaypoint();
extern bool DONE;
extern Map map;
extern int Astar(Point s, Point t, Point ROP[]);

Point ROP[MAXM];

// avoid fireball
extern void findthreat(int num);
extern Point dealthreat(int num, Point v);

void playerAI() {
    auto t=clock();
    logic = Logic::Instance();
    
    static bool ONCE=true;
    static int stamp=0;

    if(ONCE){
        map=logic->map;
        thread th(initwaypoint);
        th.detach();
        ONCE=false;
    }

    bool dontflash=false;
    if(logic->frame==1)srand(time(NULL));
    Point rush=logic->crystal[logic->faction^1].position;
    if(~logic->crystal[logic->faction^1].belong){
        rush=logic->map.target_places[logic->faction];
        dontflash=true;
    }
    Point dest[5]={logic->map.bonus_places[0],logic->map.bonus_places[1],rush,rush,rush};

    if(!DONE){
        for(int i=0;i<min(stamp/5+3,5);i++){
            Point step;
            move_smart(i,dest[i],step);
            flash_s(i,step);
        }
    }
    else{
        rep(i,min(stamp/5+3,5)){
            Point pos=GetUnit(i).position;
            int roplen=Astar(pos,dest[i],ROP);
            if(canflash(i)&&!dontflash)
            flash_s(i,bestjump(roplen,ROP,pos,human_velocity));
            else
            {
                auto T=clock();
                findthreat(i);
                Point u=ROP[0]-pos;
                Point v=dealthreat(i,u);
                move_relative(i,v);
            }
        }
    }
    for(int i=0;i<5;i++)
    {
        Point mypos=GetUnit(i).position;
        Point targ=logic->map.birth_places[logic->faction^1][0];
        for(int j=0;j<5;j++)
        {
            Human unit=GetEnemyUnit(j);
            if(unit.hp<=0)continue;
            if(dist(unit.position,mypos)<dist(targ,mypos))targ=unit.position;
        }
        double D=dist(targ,mypos)*0.05;
        logic->shoot(i,Point(targ.x+RandDouble(-D,D),targ.y+RandDouble(-D,D)));
        logic->meteor(i,Point(targ.x,targ.y));
    }
    stamp++;
    // ofstream f("1.txt",ios::app);
    // f<<"sum:"<<clock()-t<<'\n';
}