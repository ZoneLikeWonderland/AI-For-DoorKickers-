# AI-For-DoorKickers-
清华第23届DOTO智能体大赛

See details in **说明文档**



[即时战略游戏（比如 WAR3）的 AI 是怎样实现的？](https://www.zhihu.com/question/21090429)

## 分层状态机

- ### 基础层状态机

  - 向某方向移动
  - 向某方向闪现
  - 向某方向射击
  - 向某点施放陨石术

- ### 行为层状态机

  - 向某点前进并躲避弹道
  - ~~对某敌方单位预判施放陨石术~~

- ### 角色层状态机

  - 前往敌方水晶
  - 前往敌方目标点
  - 前往我方目标点
  - 前往符点
  - 保护运送水晶的人
  - ~~守护符点~~

