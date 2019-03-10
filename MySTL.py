#                             _ooOoo_
#                            o8888888o
#                            88" . "88
#                            (| -_- |)
#                            O\  =  /O
#                         ____/`---'\____
#                       .'  \\|     |//  `.
#                      /  \\|||  :  |||//  \
#                     /  _||||| -:- |||||-  \
#                     |   | \\\  -  /// |   |
#                     | \_|  ''\---/''  |   |
#                     \  .-\__  `-`  ___/-. /
#                   ___`. .'  /--.--\  `. . __
#                ."" '<  `.___\_<|>_/___.'  >'"".
#               | | :  `- \`.;`\ _ /`;.`/ - ` : | |
#               \  \ `-.   \_ __\ /__ _/   .-` /  /
#          ======`-.____`-.___\_____/___.-`____.-'======
#                             `=---='
#          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#                      God Bless Me, No Bugs.

import math
from math import sqrt,fabs,atan2
from BaseClass import *

eps = 1e-7


def L2Distance(p1,p2):
    return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y))

def PointOnLine(p,l):
    if l.p1.x==l.p2.x:
        if p.x!=l.p1.x:
            return False
        else:
            if min(l.p1.y,l.p2.y)<=p.y and p.y<=max(l.p1.y, l.p2.y):
                return True
            else:
                return False

    if fabs((l.p1.x-p.x)*(l.p2.y-p.y)-(l.p2.x-p.x)*(l.p1.y-p.y))<eps:
        if min(l.p1.x, l.p2.x)<=p.x and p.x<=max(l.p1.x,l.p2.x):
            return True
        else:
            return False
    else:
        return False



def LineIntersection(l1,l2):
    if PointOnLine(l1.p1, l2) or PointOnLine(l1.p2, l2) or PointOnLine(l2.p1, l1) or PointOnLine(l2.p2, l1):
        return True

    def Rect(line):
        return Rectangle(min(line.p1.x,line.p2.x), max(line.p1.x,line.p2.x), min(line.p1.y,line.p2.y), max(line.p1.y,line.p2.y))

    if (l1.p1.x!=l1.p2.x and l1.p1.y!=l1.p2.y) and (l2.p1.x!=l2.p2.x and l2.p1.y!=l2.p2.y):
        if not RectangleIntersection(Rect(l1), Rect(l2)):
            return False



    p1 = l1.p2-l1.p1

    if ((l2.p1-l1.p1)*p1)*((l2.p2-l1.p1)*p1)>=0:
        return False

    p2 = l2.p2-l2.p1

    if ((l1.p1-l2.p1)*p2)*((l1.p2-l2.p1)*p2)>=0:
        return False

    return True

    #This Implementation is slow, maybe bugs exist

def Angle(p1,p2):
    if p1==p2:
        raise Exception("Try to get Angle with illegal Argument")
    p = p2-p1
    angle = atan2(p.y, p.x)

def LineIntersectRect(l,rect):
    '''
    Line l
    Rectangle rect
    return True if l intersect with rect(contain no edge)
    '''
    if PointInRectangle(l.p1, rect) or PointInRectangle(l.p2,rect):
        return True

    smallrect = rect.expand(-eps)
    l1,l2,l3,l4 = smallrect.Lines()

    return LineIntersection(l, l1) or LineIntersection(l, l2) or LineIntersection(l, l3) or LineIntersection(l, l4)


def DisLinePoint(l,p):
    '''
    Line l
    Point p
    return p to line dis
    '''
    dot = (l.p2.x-l.p1.x)*(p.x-l.p1.x)+(l.p2.y-l.p1.y)*(p.y-l.p1.y)
    d2 = (l.p2.x-l.p1.x)*(l.p2.x-l.p1.x)+(l.p2.y-l.p1.y)*(l.p2.y-l.p1.y)
    if dot<=0:
        return L2Distance(p, l.p1)
    if dot>=d2:
        return L2Distance(p, l.p2)

    r = dot/d2
    px = l.p1.x + ( l.p2.x - l.p1.x ) *r
    py = l.p1.y + ( l.p2.y - l.p1.y ) *r
    return L2Distance(p, Point(px, py))


def LineIntersectCircle(l,c):
    '''
    Circle c
    Line l
    return True if c intersect with l (more than one intersection)
    Caution: if l in c return False
    '''
    if DisLinePoint(l,c.centre)<c.radius-eps:
        if L2Distance(c.centre, l.p1)<c.radius-eps and L2Distance(c.centre, l.p2)<c.radius-eps:
            return False
        else:
            return True
    else:
        return False

def MoveAlongAngle(nowPos,angle,dis):
    '''
    Point nowPos
    float angle
    float dis
    return pos where nowPos move by dis along angle
    '''
    angle = 1.0*angle/180*math.pi
    dx = 1.0*dis*math.cos(angle)
    dy = 1.0*dis*math.sin(angle)

    pos = nowPos+Point(dx, dy)
    return pos


def LineIntersectionPoint(l1,l2):
    '''
    line l1
    line l2
    return where l1 intersect with l2
    '''
    if not LineIntersection(l1, l2):
        return None

    if PointOnLine(l1.p1,l2):
        delta = l1.p2-l1.p1
        delta.x*=eps
        delta.y*=eps
        if PointOnLine(l1.p1+delta, l2):
            return None,None
        else:
            return l1.p1

    if PointOnLine(l1.p2,l2):
        delta = l1.p1-l1.p2
        delta.x*=eps
        delta.y*=eps
        if PointOnLine(l1.p2+delta, l2):
            return None,None
        else:
            return l1.p2


    tmpLeft = (l2.p2.x-l2.p1.x)*(l1.p1.y-l1.p2.y)-(l1.p2.x-l1.p1.x)*(l2.p1.y-l2.p2.y)
    tmpRight = (l1.p1.y-l2.p1.y)*(l1.p2.x-l1.p1.x)*(l2.p2.x-l2.p1.x)+l2.p1.x*(l2.p2.y-l2.p1.y)*(l1.p2.x-l1.p1.x)-l1.p1.x*(l1.p2.y-l1.p1.y)*(l2.p2.x-l2.p1.x)
    x = tmpRight/tmpLeft
    tmpLeft = (l1.p1.x-l1.p2.x)*(l2.p2.y-l2.p1.y)-(l1.p2.y-l1.p1.y)*(l2.p1.x-l2.p2.x)
    tmpRight = l1.p2.y*(l1.p1.x-l1.p2.x)*(l2.p2.y-l2.p1.y)+(l2.p2.x-l1.p2.x)*(l2.p2.y-l2.p1.y)*(l1.p1.y-l1.p2.y)-l2.p2.y*(l2.p1.x-l2.p2.x)*(l1.p2.y-l1.p1.y)
    y = tmpRight/tmpLeft;
    return Point(x, y)

def CircleIntersection(c1,c2):
    '''
    Circle c1
    Circle c2
    return True if c1 intersect with c2
    '''
    dis = L2Distance(c1.centre, c2.centre)
    return dis<c1.radius+c2.radius-eps



def LineIntersectCirclePoints(l,c):
    '''
    Line l
    Circle c
    return list of Points
    '''
    if not LineIntersectCircle(l, c):
        return []

    def disLine(A,B,P):
        return fabs((B-A)*(P-A))/L2Distance(A,B)
    O = c.centre
    A = l.p1
    B = l.p2
    r = c.radius
    AO = O-A
    AB = B-A
    dis_AB = L2Distance(A, B)
    mu = (AO.x*AB.x+AO.y*AB.y)/dis_AB
    PR = Point(A.x+mu*AB.x/dis_AB, A.y+mu*AB.y/dis_AB)
    base = sqrt(r*r-disLine(A,B,O)*disLine(A, B, O))
    Base = Point(base*AB.x/dis_AB, base*AB.y/dis_AB)
    ans1,ans2 = Base+PR,PR-Base
    ans = []
    for p in ans1,ans2:
        if l.p1.x==l.p2.x:
            if min(l.p1.y,l.p2.y)<=p.y and p.y<=max(l.p1.y,l.p2.y):
                ans.append(p)
        else:
            if min(l.p1.x,l.p2.x)<=p.x and p.x<=max(l.p1.x,l.p2.x):
                ans.append(p)
    return ans


def LegalPos(pos,walls):
    if pos.x<0 or pos.x>width or pos.y<0 or pos.y>height:
        return False
    for wall in walls:
        if wall.left<=pos.x and pos.x<=wall.right and wall.bottom<=pos.y and pos.y<=wall.top:
            return False
    return True


def FutureFireballPos(fireball,walls,future):
    '''
    Fireball fireball
    list of Wall walls
    int future
    return where the fireball is after future frames,if collision happens return None

    '''
    if future == 0:
        return fireball.circle.centre
    pos1 = fireball.circle.centre
    angle = fireball.rotation
    angle = 1.0*angle/180*math.pi
    dx = 1.0*Fireball.velocity*future*math.cos(angle)
    dy = 1.0*Fireball.velocity*future*math.sin(angle)
    pos2 = pos1+Point(dx, dy)
    r = fireball.circle.radius
    if pos2.x-r<map_lbx or pos2.x+r>map_ubx or pos2.y-r<map_lby or pos2.y+r>map_uby:
        return None

    line = Line(pos1, pos2)
    for wall in walls:
        ps = wall.rectangle.Points()
        for p in ps:
            dis = DisLinePoint(line, p)
            if dis < r-eps:
                return None
        rect = wall.rectangle.expand(-eps)
        l1,l2,l3,l4 = wall.rectangle.Lines()
        if  LineIntersection(l1, line) or LineIntersection(l2, line) or LineIntersection(l3, line) or LineIntersection(l4, line):
            return None
        if DisLinePoint(l1, pos2)<r or DisLinePoint(l2, pos2)<r or DisLinePoint(l3, pos2)<r or DisLinePoint(l4, pos2)<r:
            return None
    return pos2


def HumanCanGotoPos(human,walls,pos):
    '''
    Human human
    Point pos
    list of wall walls
    return True if human can go straight to pos from human.position
    without collision(with wall)
    '''
    pos1 = human.circle.centre
    if pos1.x==pos.x and pos1.y==pos.y:
        return True
    r = human.circle.radius
    if pos.x-r<map_lbx or pos.x+r>map_ubx or pos.y-r<map_lby or pos.y+r>map_uby:
        return False

    line = Line(pos1, pos)
    for wall in walls:
        ps = wall.rectangle.Points()
        for p in ps:
            dis = DisLinePoint(line, p)
            if dis < r-eps:
                return False
        rect = wall.rectangle.expand(-eps)
        l1,l2,l3,l4 = wall.rectangle.Lines()
        if  LineIntersection(l1, line) or LineIntersection(l2, line) or LineIntersection(l3, line) or LineIntersection(l4, line):
            return False
        if DisLinePoint(l1, pos)<r or DisLinePoint(l2, pos)<r or DisLinePoint(l3, pos)<r or DisLinePoint(l4, pos)<r:
            return False
    return True


def UpdateWeaponMap(walls,fireballs,meteors,future,eventlist):
    '''
    list of Wall: walls
    list of Fireball : fireballs
    list of Meteor : meteors
    int : future

    return update fireballs and meteors after future frames

    e.g. future = 1 : next turn


    Warning: meteor.time>=0
    Warning: assume human has no influence , so does ball
    '''
    if future==0:
        return
    delFireballs = []
    delMeteors = []

    for fireball in fireballs:
        pos = FutureFireballPos(fireball, walls, future)
        if pos != None:
            fireball.circle.centre = pos
            fireball.attack_range.centre = pos
        else:
            delFireballs.append(fireball)
    for fireball in delFireballs:
        eventlist.append([9,fireball.circle.centre.x,fireball.circle.centre.y])
        fireballs.remove(fireball)

    for meteor in meteors:
        if meteor.time>=future:
            meteor.time-=future
        else:
            delMeteors.append(meteor)

    for meteor in delMeteors:
        meteors.remove(meteor)



# Now, God only knows

import time
import random

if __name__ == '__main__':
    import pygame
    pygame.init()
    screen = pygame.display.set_mode((width_of_screen,height_of_screen))
    pygame.display.set_caption('Debug Mode')
    screen.fill(white)
    def drawPoint(pos):
        pygame.draw.circle(screen, green, (int(pos.x),int(pos.y)), 4)

    def drawLine(line):
        pygame.draw.line(screen, black, (int(line.p1.x),int(line.p1.y)), (int(line.p2.x),int(line.p2.y)), 2)

    def drawCircle(circ):
        pygame.draw.circle(screen, yellow, (int(circ.centre.x),int(circ.centre.y)), int(circ.radius))

    def drawRect(rect):
        pygame.draw.rect(screen, red, pygame.Rect(int(rect.left), int(rect.bottom), int(rect.right-rect.left), int(rect.top-rect.bottom)))

    def RandomPoint():
        return Point(random.randint(map_lbx,map_ubx), random.randint(map_lby, map_uby))

    def RandomLine():
        A = RandomPoint()
        B = RandomPoint()
        while A==B:
            A = RandomPoint()
            B = RandomPoint()
        return Line(A, B)

    def RandomRect():
        A = RandomPoint()
        B = RandomPoint()
        while A.x==B.x or A.y==B.y:
            A = RandomPoint()
            B = RandomPoint()
        return Rectangle(min(A.x,B.x), max(A.x,B.x), min(A.y,B.y), max(A.y,B.y))
    def RandomCirc():
        return Circle(RandomPoint(), random.randint(1, 100))




    #rect1 = Rectangle(100, 200, 500, 800)
    #p1 = Point(150, 600)
    #print(PointInRectangle(p1, rect1))
    #drawRect(rect1)
    #drawPoint(p1)

    #rect2 = Rectangle(105, 110, 550, 600)
    #print(RectangleIntersection(rect1, rect2))

    #p = Point(10, 10)
    #line = Line(Point(5,9), Point(15, 11))
    #print(PointOnLine(p,line))

    while True:

        screen.fill(white)
        #l = Line(Point(200,200),Point(200, 400))
        #l2 = RandomLine()
        #while(not LineIntersection(l1, l2)):
        #    l1 = RandomLine()
        #    l2 = RandomLine()
        #l1 = Line(Point(99,99), Point(100, 100))
        #l2 = Line(Point(99,99), Point(200, 200))
        #print(LineIntersection(l1,l2))
        #p = LineIntersectionPoint(l1, l2)
        #drawLine(l1)
        #drawLine(l2)
        #drawPoint(p)
        #p = RandomPoint()
        #c = RandomCirc()
        #rects = [Rectangle(217, 483, 232, 247),
        #         Rectangle(42, 590, 371, 508),
        #         Rectangle(303, 307, 12, 74),
        #         Rectangle(429, 570, 336, 416),
        #         Rectangle(327, 380, 432, 456)]
        walls = []

        for i in range(5):
            rect = RandomRect()
        #for rect in rects:
            wall = Wall(rect.left, rect.right, rect.bottom, rect.top)
            walls.append(wall)
            drawRect(wall.rectangle)
        pos = RandomPoint()
        #pos = Point(109, 283)
        while not LegalPos(Circle(pos,Meteor_radius), walls):
            pos = RandomPoint()


        rot = Angle(pos, Point(width_of_screen/2,height_of_screen/2))+random.randint(-10,10)
        if rot>360:
            rot-=360
        if rot<0:
            rot+=360
        #rot = random.randint(0, 360)
        #rot = 48
        #u = LineIntersectCirclePoints(l, c)
        #while u==[]:
        #    l = Line(Point(200,200),Point(200, 400))
        #    c = RandomCirc()
        #    u = LineIntersectCirclePoints(l, c)

        #dis = DisLinePoint(l, p)
        #drawCircle(c)
        #drawRect(rect)
        #print(LegalPos(c, walls))
        #for p in u:
        #    drawPoint(p)

    #    drawLine(l2)
    #    print(LineIntersectCircle(c,l))
        pygame.display.flip()
        time.sleep(2)
    #    break



