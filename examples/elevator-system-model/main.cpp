#include "simlib.h"
#include "define.h"
#include "function.h"
#include<stdio.h>
#include<vector>
#include<string>
#include<iostream>
#include<fstream>


// ELEVATOR elevator;
int elevator_floor = HOME_BASE , elevator_direction = IDLE , elevator_destination = HOME_BASE;

unsigned num_people = 0;

std::vector<double> QUEUE_UP_DELAY_2;
unsigned not_get_in_num[9] = {0};// 因电梯满而无法进入的人数


int main(){
    init_simlib();
    maxatr = 4;
    list_rank[LIST_ELEVATOR] = 2;//电梯中的人员按目标楼层增量排序
    timest(0.0 , 0);
    

    //一楼到达事件
    event_schedule(sim_time + expon(1.0 , STREAM_INTERARRIVAL), EVENT_ARRIVAL);
    //模拟结束事件,20小时
    event_schedule(20 * 60, EVENT_END_SIMULATION);
    timest(1.0 , TIMEST_ELEVATOR_IDLE);

    std::ofstream log("debug.log" , std::ios::trunc);

    //主事件循环
    do{

        timing();

        switch(next_event_type){
            case EVENT_ARRIVAL://处理一楼的到达事件
                arrive();
                event_schedule(sim_time + expon(1.0 , STREAM_INTERARRIVAL), EVENT_ARRIVAL);
                num_people++;
                break;
            case EVENT_ELEVATOR_ARRIVAL://处理电梯到达事件
                elevator_arrival();
                break;
            case EVENT_FLOOR_CHANGE://处理电梯中人员改变楼层事件
                floor_change();
                break;
        }
        update_static();
        output_log(log);
    }while (next_event_type != EVENT_END_SIMULATION);
    report();
    log.close();
    return 0;

}
