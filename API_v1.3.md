# API 文档

[TOC]

## BaseClass.py

### class Point(object)

| 接口                                        | 解释                |
| ------------------------------------------- | ------------------- |
| `__init__(self,x,y)`                        | 使用(x,y)进行初始化 |
| `__str__(self)`                             | 字符串表示          |
| `__add__(self,other)` `__sub__(self,other)` | 加减操作            |
| `__mul__(self,other)`                       | 叉积                |

| 成员 | 解释   |
| ---- | ------ |
| x    | 横坐标 |
| y    | 纵坐标 |

### class Line(object)

| 接口                   | 解释                  |
| ---------------------- | --------------------- |
| `__init__(self,p1,p2)` | 使用点p1,p2进行初始化 |
| `__str__(self)`        | 字符串表示            |
| `Points(self)`         | 返回线段的两个端点    |

| 成员 | 解释  |
| ---- | ----- |
| p1   | 端点1 |
| p2   | 端点2 |

### class Rectangle(object)

| 接口                                   | 解释                       |
| -------------------------------------- | -------------------------- |
| `__init__(self,left,right,bottom,top)` | 使用左右下上边界进行初始化 |
| `__str__(self)`                        | 字符串表示                 |
| `Points(self)`                         | 返回线段的四个端点         |
| `Lines(self)`                          | 返回矩形的四条边           |
| `expand(self,d)`                       | 返回向外扩展d的新矩形      |

| 成员                    | 解释             |
| ----------------------- | ---------------- |
| `left,right,bottom,top` | 左右下上四个实数 |

### class Circle(object)

| 接口                           | 解释                 |
| ------------------------------ | -------------------- |
| `__init__(self,centre,radius)` | 使用圆心与半径初始化 |
| `__str__(self)`                | 字符串表示           |

| 成员     | 解释              |
| -------- | ----------------- |
| `centre` | 圆心，`Point`类型 |
| `radius` | 半径，实数类型    |

### class Ball(object)

| 接口                      | 解释                        |
| ------------------------- | --------------------------- |
| `__init__(self,position)` | 使用Point类型初始化球的位置 |

| 成员     | 解释                                         |
| -------- | -------------------------------------------- |
| `centre` | 圆心，`Point`类型                            |
| `radius` | 半径，实数类型                               |
| `belong` | 持有者                                       |
| `circle` | 该类的形状对应的圆，与`centre`和`radius`一致 |

### class Bullet(object)

| 接口                               | 解释                                          |
| ---------------------------------- | --------------------------------------------- |
| `__init__(self,position,rotation)` | 使用Point类型和实数类型初始化子弹位置移动方向 |

| 成员       | 解释                                         |
| ---------- | -------------------------------------------- |
| `centre`   | 圆心，`Point`类型                            |
| `radius`   | 半径，实数类型                               |
| `rotation` | 角度，0~360                                  |
| `circle`   | 该类的形状对应的圆，与`centre`和`radius`一致 |
| `velocity` | 移动速率                                     |
| `hurt`     | 伤害                                         |

### class Grenade(object)

| 接口                               | 解释                                          |
| ---------------------------------- | --------------------------------------------- |
| `__init__(self,position,rotation)` | 使用Point类型和实数类型初始化手雷位置移动方向 |

| 成员          | 解释                                         |
| ------------- | -------------------------------------------- |
| `centre`      | 圆心，`Point`类型                            |
| `radius`      | 半径，实数类型                               |
| `rotation`    | 角度，0~360                                  |
| `circle`      | 该类的形状对应的圆，与`centre`和`radius`一致 |
| `velocity`    | 移动速率                                     |
| `hurt`        | 伤害                                         |
| `hurt_radius` | 爆炸半径                                     |
| `time`        | 剩余存在时间                                 |

### class Human(object)

| 接口                                      | 解释                                                         |
| ----------------------------------------- | ------------------------------------------------------------ |
| `__init__(self,position,rotation,number)` | 使用Point类型和实数类型初始化人的位置朝向，使用number表示这个人的编号 |

| 成员             | 解释                                         |
| ---------------- | -------------------------------------------- |
| `centre`         | 圆心，`Point`类型                            |
| `radius`         | 半径，实数类型                               |
| `rotation`       | 角度，0~360                                  |
| `circle`         | 该类的形状对应的圆，与`centre`和`radius`一致 |
| `velocity_max`   | 最大单帧移动速率                             |
| `rotation_max`   | 最大单帧旋转速率                             |
| `fire_interval`  | 每经过`fire_interval`帧发射一颗子弹          |
| `hp`             | 剩余血量                                     |
| `grenade_number` | 剩余手雷数                                   |
| `fire_time`      | 距离下一次的开火时间                         |
| `number`         | 编号                                         |

### class Wall(object)

| 接口                                   | 解释                   |
| -------------------------------------- | ---------------------- |
| `__init__(self,left,right,bottom,top)` | 使用左右下上坐标初始化 |

| 成员        | 解释     |
| ----------- | -------- |
| `rectangle` | 外围形状 |

## MySTL.py

里面包含各种可能使用到的计算几何的函数

## Arguments.py

里面包含游戏设定的各种常数

## AI_Player.py

实现AI的基类，定义了相关的接口

| 接口                                                    | 解释                                                         |
| ------------------------------------------------------- | ------------------------------------------------------------ |
| `__init__(self,num,ball,walls,bullets,humans,grenades)` | 初始时，将自己控制的人的编号，球，所有的墙，所有的子弹，所有的人，所有的炸弹的信息传给 |
| `refresh(self,num,ball,walls,bullets,humans,grenades)`  | 每一帧开始时，会调用refresh将上一帧的运行结果传给AI，进行地图信息的更新 |
| `analysis(self)`                                        | 你需要覆盖这个接口，给出自己的每一轮的决策结果，每一轮refresh之后调用这个函数 |

| 成员     | 解释               |
| -------- | ------------------ |
| number   | 自己控制的角色编号 |
| ball     | Ball类             |
| walls    | Wall类的list       |
| bullets  | Bullet类的list     |
| humans   | Human类的list      |
| grenades | Grenade类的list    |