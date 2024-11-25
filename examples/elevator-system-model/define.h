#define EVENT_ARRIVAL 1 // 事件1 ： 人员到达1层
#define EVENT_FLOOR_CHANGE 2 // 事件2 ： 人员在某层改变目的地
#define EVENT_ELEVATOR_ARRIVAL 3 // 事件3 ： 电梯到达某层
#define EVENT_END_SIMULATION 4 // 事件4 ： 模拟结束

#define ELEVATOR_MAX_CAPACITY 6 // 电梯最大容量

// 每层楼上行下行各需要一个队列
// attributes : 1. 到达时间 2. 目标楼层
#define LIST_QUEUE_UP_1    1  // 1楼上行队列
#define LIST_QUEUE_UP_2    2  // 2楼上行队列
#define LIST_QUEUE_UP_3    3
#define LIST_QUEUE_UP_4    4
#define LIST_QUEUE_DOWN_2  5  // 2楼下行队列
#define LIST_QUEUE_DOWN_3  6
#define LIST_QUEUE_DOWN_4  7
#define LIST_QUEUE_DOWN_5  8

#define LIST_ELEVATOR 9 // 电梯当前搭载的乘客
//attributes : 1. 进入时间 2. 目标楼层

//电梯运行方向
#define IDLE 0 // 空闲
#define UP 1 // 上行
#define DOWN -1 // 下行


#define STREAM_INTERARRIVAL 1 // 随机数流1 ： 人员到达时间间隔
#define STREAM_NEXT_FLOOR 2 // 随机数流2 ： 人员目标楼层
#define STREAM_STAY_TIME 3 // 随机数流3 ： 人员在当前楼层停留时间

//统计变量
#define SAMPST_DELAY_UP_1 1 // 1楼上行平均等待时间
#define SAMPST_DELAY_UP_2 2
#define SAMPST_DELAY_UP_3 3
#define SAMPST_DELAY_UP_4 4

#define SAMPST_DELAY_DOWN_2 5
#define SAMPST_DELAY_DOWN_3 6
#define SAMPST_DELAY_DOWN_4 7
#define SAMPST_DELAY_DOWN_5 8

#define SAMPST_AVERAGE_DELAY 9 // 每个人平均等待时间

#define HOME_BASE 1 // 电梯基地楼层
#define FLOOR_NUM 5 // 楼层数

#define TIMEST_ELEVATOR_MOVING_WITH_PEOPLE  1  // 载人运行
#define TIMEST_ELEVATOR_MOVING_EMPTY        2  // 空载运行
#define TIMEST_ELEVATOR_IDLE                3  // 空闲停止
#define TIMEST_ELEVATOR_PEOPLE_NUM      4  //电梯中人数