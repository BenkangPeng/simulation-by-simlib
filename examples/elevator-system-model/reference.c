#include "../../simlib/simlib.h"

#define EVENT_ARRIVAL          1  // 人员到达1楼
#define EVENT_FLOOR_CHANGE     2  // 人员在某层改变目的地
#define EVENT_ELEVATOR_ARRIVAL 3  // 电梯到达某层
#define EVENT_END_SIMULATION   4  // 模拟结束

// 每层楼上行下行各需要一个队列
#define LIST_QUEUE_UP_1    1  // 1楼上行队列
#define LIST_QUEUE_UP_2    2  // 2楼上行队列
#define LIST_QUEUE_UP_3    3
#define LIST_QUEUE_UP_4    4
#define LIST_QUEUE_DOWN_2  5  // 2楼下行队列
#define LIST_QUEUE_DOWN_3  6
#define LIST_QUEUE_DOWN_4  7
#define LIST_QUEUE_DOWN_5  8

#define LIST_ELEVATOR      9  // 电梯当前搭载的乘客

#define DIR_UP    1
#define DIR_DOWN   -1
#define DIR_IDLE    0

// 随机数流
#define STREAM_INTERARRIVAL 1
#define STREAM_NEXT_FLOOR 2

// 统计变量
#define SAMPST_DELAY_UP_1   1  // 各层上行等待时间
#define SAMPST_DELAY_UP_2   2
// ... 其他统计变量

int elevator_floor;        // 电梯当前楼层
int elevator_direction;    // 电梯运行方向 (1=上行, -1=下行, 0=空闲)
int elevator_destination;  // 电梯目标楼层
float sim_time;           // 当前模拟时间

void arrive(void) {
    // 1. 调度下一个到达事件
    event_schedule(sim_time + expon(1.0, STREAM_INTERARRIVAL), EVENT_ARRIVAL);
    
    // 2. 确定目标楼层
    int destination = determine_destination(STREAM_NEXT_FLOOR);
    
    // 3. 如果电梯在1楼且空闲，直接进入电梯
    if (elevator_floor == 1 && elevator_direction == 0 && 
        list_size[LIST_ELEVATOR] < 6) {
        add_to_elevator(destination);
        start_elevator_movement();
    } else {
        // 4. 否则加入等待队列
        add_to_queue(1, destination);
    }
}

void elevator_arrival(void) {
    // 1. 让到达目标楼层的乘客下电梯
    remove_arrived_passengers();
    
    // 2. 根据控制逻辑决定是否装载等待的乘客
    if (can_load_passengers()) {
        load_waiting_passengers();
    }
    
    // 3. 确定下一个目标楼层
    determine_next_destination();
    
    // 4. 调度下一个电梯到达事件
    if (elevator_destination != elevator_floor) {
        event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);  // 15秒=0.25分钟
    } else if (elevator_floor == home_base) {
        elevator_direction = 0;  // 电梯空闲
    }
}

int determine_destination(int stream) {
    double r = lcgrand(stream);
    if (r < 0.25) return 2;
    if (r < 0.50) return 3;
    if (r < 0.75) return 4;
    return 5;
}

void add_to_elevator(int destination) {
    transfer[1] = sim_time;         // 进入时间
    transfer[2] = destination;      // 目标楼层
    list_file(LAST, LIST_ELEVATOR);
}

void add_to_queue(int floor, int destination) {
    transfer[1] = sim_time;         // 到达时间
    transfer[2] = destination;      // 目标楼层
    int queue = determine_queue(floor, destination);
    list_file(LAST, queue);
}

void collect_statistics() {
    // 1. 收集队列延迟统计
    for (int floor = 1; floor <= 5; floor++) {
        if (floor < 5) sampst(delay, SAMPST_DELAY_UP_1 + floor - 1);
        if (floor > 1) sampst(delay, SAMPST_DELAY_DOWN_1 + floor - 2);
    }
    
    // 2. 收集电梯使用统计
    timest((double)list_size[LIST_ELEVATOR], TIMEST_ELEVATOR_LOAD);
    
    // 3. 收集电梯状态统计
    if (elevator_direction != 0) {
        if (list_size[LIST_ELEVATOR] > 0)
            timest(1.0, TIMEST_MOVING_WITH_PASSENGERS);
        else
            timest(1.0, TIMEST_MOVING_EMPTY);
    } else {
        timest(1.0, TIMEST_IDLE);
    }
}



/* 电梯状态相关函数 */

void start_elevator_movement(void) {
    /* 如果电梯当前空闲，需要决定运行方向和目标楼层 */
    if (elevator_direction == DIR_IDLE) {
        /* 1. 检查电梯内是否有乘客 */
        if (list_size[LIST_ELEVATOR] > 0) {
            determine_next_destination();
        } 
        /* 2. 如果电梯为空，检查各层是否有等待的乘客 */
        else {
            int found_passenger = 0;
            
            /* 先检查当前楼层是否有等待的乘客 */
            if (elevator_floor < 5 && list_size[LIST_QUEUE_UP_1 + elevator_floor - 1] > 0) {
                elevator_direction = DIR_UP;
                found_passenger = 1;
            } else if (elevator_floor > 1 && 
                      list_size[LIST_QUEUE_DOWN_2 + elevator_floor - 2] > 0) {
                elevator_direction = DIR_DOWN;
                found_passenger = 1;
            }
            
            /* 如果当前楼层没有，则搜索其他楼层 */
            if (!found_passenger) {
                /* 向上搜索 */
                for (int floor = elevator_floor + 1; floor <= 5; floor++) {
                    if (list_size[LIST_QUEUE_UP_1 + floor - 1] > 0 ||
                        list_size[LIST_QUEUE_DOWN_2 + floor - 2] > 0) {
                        elevator_direction = DIR_UP;
                        elevator_destination = floor;
                        found_passenger = 1;
                        break;
                    }
                }
                
                /* 向下搜索 */
                if (!found_passenger) {
                    for (int floor = elevator_floor - 1; floor >= 1; floor--) {
                        if (floor < 5 && list_size[LIST_QUEUE_UP_1 + floor - 1] > 0 ||
                            floor > 1 && list_size[LIST_QUEUE_DOWN_2 + floor - 2] > 0) {
                            elevator_direction = DIR_DOWN;
                            elevator_destination = floor;
                            found_passenger = 1;
                            break;
                        }
                    }
                }
            }
            
            /* 如果没有找到等待的乘客，返回基地 */
            if (!found_passenger) {
                if (elevator_floor != home_base) {
                    elevator_direction = (home_base > elevator_floor) ? DIR_UP : DIR_DOWN;
                    elevator_destination = home_base;
                } else {
                    elevator_direction = DIR_IDLE;
                    return; // 不需要调度到达事件
                }
            }
        }
    }
    
    /* 如果电梯需要移动，调度到达下一层的事件 */
    if (elevator_direction != DIR_IDLE) {
        event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL); // 15秒 = 0.25分钟
    }
}

void remove_arrived_passengers(void) {
    int removed = 0;
    struct master *current, *next;
    
    /* 遍历电梯中的所有乘客 */
    current = head[LIST_ELEVATOR];
    while (current != NULL) {
        next = current->sr;  // 保存下一个乘客的指针
        
        if ((int)current->value[2] == elevator_floor) {
            /* 到达目标楼层的乘客 */
            
            /* 计算乘客的总旅行时间 */
            sampst(sim_time - current->value[1], SAMPST_TRAVEL_TIME);
            
            /* 如果是到达1楼，乘客离开大楼 */
            if (elevator_floor == 1) {
                list_remove(FIRST, LIST_ELEVATOR);
            } else {
                /* 在当前楼层停留一段时间后会再次出发 */
                double stay_time = uniform(15.0, 120.0, STREAM_STAY_TIME);
                
                /* 调度该乘客的楼层改变事件 */
                transfer[1] = elevator_floor;  // 当前楼层
                transfer[2] = sim_time + stay_time;  // 改变时间
                list_file(INCREASING, LIST_FLOOR_CHANGE);
                
                /* 从电梯中移除乘客 */
                list_remove(FIRST, LIST_ELEVATOR);
            }
            removed++;
        }
        current = next;
    }
    
    /* 更新电梯载客统计 */
    if (removed > 0) {
        timest((double)list_size[LIST_ELEVATOR], TIMEST_ELEVATOR_LOAD);
    }
}

void load_waiting_passengers(void) {
    int queue_number;
    
    /* 确定当前楼层的队列编号 */
    if (elevator_direction == DIR_UP) {
        queue_number = LIST_QUEUE_UP_1 + elevator_floor - 1;
    } else {
        queue_number = LIST_QUEUE_DOWN_2 + elevator_floor - 2;
    }
    
    /* 尝试装载等待的乘客，直到电梯满载或队列为空 */
    while (list_size[LIST_ELEVATOR] < 6 && list_size[queue_number] > 0) {
        /* 检查队首乘客的目标楼层是否符合当前运行方向 */
        struct master *first = head[queue_number];
        int destination = (int)first->value[2];
        
        if ((elevator_direction == DIR_UP && destination > elevator_floor) ||
            (elevator_direction == DIR_DOWN && destination < elevator_floor)) {
            
            /* 移除队列中的乘客 */
            list_remove(FIRST, queue_number);
            
            /* 计算等待时间 */
            sampst(sim_time - transfer[1], SAMPST_DELAY_UP_1 + elevator_floor - 1);
            
            /* 将乘客加入电梯 */
            transfer[1] = sim_time;  // 进入电梯时间
            transfer[2] = destination;  // 目标楼层
            list_file(LAST, LIST_ELEVATOR);
            
        } else {
            /* 队列中的乘客目标方向与电梯运行方向不符 */
            break;
        }
    }
    
    /* 统计未能进入电梯的乘客数量（电梯满载情况） */
    if (list_size[queue_number] > 0 && list_size[LIST_ELEVATOR] == 6) {
        sampst(1.0, SAMPST_FULL_REJECT_1 + elevator_floor - 1);
    }
}

void determine_next_destination(void) {
    int i, has_passengers_above = 0, has_passengers_below = 0;
    int has_waiting_above = 0, has_waiting_below = 0;
    
    /* 检查电梯中是否有去上层/下层的乘客 */
    struct master *current = head[LIST_ELEVATOR];
    while (current != NULL) {
        int dest = (int)current->value[2];
        if (dest > elevator_floor) has_passengers_above = 1;
        if (dest < elevator_floor) has_passengers_below = 1;
        current = current->sr;
    }
    
    /* 检查各层是否有等待的乘客 */
    for (i = 1; i <= 5; i++) {
        if (i > elevator_floor) {
            if (list_size[LIST_QUEUE_UP_1 + i - 1] > 0) has_waiting_above = 1;
        }
        if (i < elevator_floor) {
            if (list_size[LIST_QUEUE_DOWN_2 + i - 2] > 0) has_waiting_below = 1;
        }
    }
    
    /* 根据电梯控制逻辑决定下一个目标楼层 */
    if (elevator_direction == DIR_UP) {
        if (has_passengers_above || has_waiting_above) {
            /* 继续向上 */
            elevator_destination = find_next_stop_up();
        } else if (has_passengers_below || has_waiting_below) {
            /* 改变方向向下 */
            elevator_direction = DIR_DOWN;
            elevator_destination = find_next_stop_down();
        } else {
            /* 返回基地（1楼或3楼） */
            elevator_direction = DIR_DOWN;
            elevator_destination = home_base;
        }
    } else if (elevator_direction == DIR_DOWN) {
        if (has_passengers_below || has_waiting_below) {
            /* 继续向下 */
            elevator_destination = find_next_stop_down();
        } else if (has_passengers_above || has_waiting_above) {
            /* 改变方向向上 */
            elevator_direction = DIR_UP;
            elevator_destination = find_next_stop_up();
        } else {
            /* 返回基地 */
            elevator_direction = (home_base > elevator_floor) ? DIR_UP : DIR_DOWN;
            elevator_destination = home_base;
        }
    }
}

/* 辅助函数：找到上行方向的下一个停靠楼层 */
int find_next_stop_up(void) {
    int floor;
    for (floor = elevator_floor + 1; floor <= 5; floor++) {
        /* 检查是否有乘客要在这层下电梯 */
        struct master *current = head[LIST_ELEVATOR];
        while (current != NULL) {
            if ((int)current->value[2] == floor) return floor;
            current = current->sr;
        }
        
        /* 检查这层是否有等待的乘客 */
        if (list_size[LIST_QUEUE_UP_1 + floor - 1] > 0) return floor;
    }
    return 5;  // 如果没找到，返回最顶层
}

/* 辅助函数：找到下行方向的下一个停靠楼层 */
int find_next_stop_down(void) {
    int floor;
    for (floor = elevator_floor - 1; floor >= 1; floor--) {
        /* 检查是否有乘客要在这层下电梯 */
        struct master *current = head[LIST_ELEVATOR];
        while (current != NULL) {
            if ((int)current->value[2] == floor) return floor;
            current = current->sr;
        }
        
        /* 检查这层是否有等待的乘客 */
        if (floor > 1 && list_size[LIST_QUEUE_DOWN_2 + floor - 2] > 0) 
            return floor;
    }
    return 1;  // 如果没找到，返回底层
}

int main() {
    // 1. 初始化
    init_simlib();
    initialize_elevator();
    
    // 2. 调度第一个到达事件
    event_schedule(expon(1.0, STREAM_INTERARRIVAL), EVENT_ARRIVAL);
    
    // 3. 调度结束事件
    event_schedule(20 * 60, EVENT_END_SIMULATION);  // 20小时
    
    // 4. 主事件循环
    while (next_event_type != EVENT_END_SIMULATION) {
        timing();
        switch (next_event_type) {
            case EVENT_ARRIVAL:
                arrive();
                break;
            case EVENT_FLOOR_CHANGE:
                floor_change();
                break;
            case EVENT_ELEVATOR_ARRIVAL:
                elevator_arrival();
                break;
        }
        collect_statistics();
    }
    
    // 5. 输出报告
    report();
}