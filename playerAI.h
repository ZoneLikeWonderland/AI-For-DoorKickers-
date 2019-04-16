#include "const.h"
#include "geometry.h"
#include "logic.h"

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

void playerAI();