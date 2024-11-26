#include "function.h"
#include "simlib.h"
#include"define.h"

extern int elevator_floor;
extern int elevator_direction;
extern int elevator_destination;
extern unsigned num_people;
extern unsigned not_get_in_num[9];


void arrive(){
    /*arrive()应该要能驱动空闲电梯
    例如，当HOME_BASE = 3时，电梯在3楼空闲，arrive()应该能驱动电梯下行*/

    //确定人员的目标楼层(2，3，4，5等概率分布)
    int destination = determine_destination(STREAM_NEXT_FLOOR);

    if(elevator_direction == IDLE){
        if(elevator_floor == 1){
            /*电梯空闲，且在1楼(即HOME_BASE = 1)*/
            transfer[1] = sim_time;
            transfer[2] = destination;
            list_file(INCREASING, LIST_ELEVATOR);//人员进入电梯
            /*更新电梯状态*/
            elevator_direction = UP;
            elevator_destination = destination;
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);

        }
        else{/*电梯不在1楼，HOME_BASE != 1*/
            //人员进入1楼的等待队列，LIST_QUEUE_UP_1
            transfer[1] = sim_time;
            transfer[2] = destination;
            list_file(LAST, LIST_QUEUE_UP_1);
            /*电梯前往1楼接客*/
            elevator_direction = DOWN;
            elevator_destination = 1;
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);

        }
    }
    else{/*电梯不空闲*/
        //人员进入1楼的等待队列，LIST_QUEUE_UP_1
        transfer[1] = sim_time;
        transfer[2] = destination;
        list_file(LAST, LIST_QUEUE_UP_1);
    }
}

void elevator_arrival(){
    
    //电梯步进一楼
    elevator_floor = (elevator_direction == UP) ? elevator_floor + 1 : elevator_floor - 1;

    //电梯开门后，先下后上
    if(elevator_direction == UP){
        elevator_arrival_UP();
    }
    else{//DOWN
        elevator_arrival_DOWN();
    }
    /*调度下一个电梯到达事件*/
    /*电梯到达目的地前，电梯中不可能为空——电梯要走到目的地时，才有可能改变 方向
    到达目的地时，电梯不一定为空(但电梯里面的人会先走完，然后再进人)，可能当前楼层又进入了同方向的人*/
    if(elevator_floor != elevator_destination){
        /*两种情况：1.电梯没到达目的地
        2. 电梯在目的地那层楼中又接到了与电梯方向相同的人(同时elevator_destination也更新了)
        注：电梯在目的地处不可能接到与其方向相反的人(见上面的逻辑)*/
        event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
        /*没到达目的地，就不用改变方向，朝方向步进一格*/  
    }
    else{/*电梯到达目的地
        电梯中没人了(如果电梯里有人，电梯目的地绝对不等于当前楼层)
        朝原运行方向(UP/DOWN)看去，若电梯之上(之下)的楼层有人想上楼(下楼)，
        则前往电梯之上(之下)的最近的楼层*/
        if(elevator_direction == UP){
            elevator_arrival_UP_2_destination();
        }

        else{//DOWN电梯下行
            elevator_arrival_DOWN_2_destination();
        }
    }


}

/// 电梯上行时的到达事件
void elevator_arrival_UP(){
    /*电梯内人员先下*/
    while(list_size[LIST_ELEVATOR] > 0 && (int)head[LIST_ELEVATOR]->value[2] == elevator_floor){
        //电梯中有人想下电梯
        list_remove(FIRST, LIST_ELEVATOR);
        //这人出电梯后，过一段时间后，又会回来坐电梯，把这个事件定义成他要改变楼层
        double stay_time = uniform(15.0 , 120.0 , STREAM_STAY_TIME);
        transfer[3] = elevator_floor;
        event_schedule(sim_time + stay_time, EVENT_FLOOR_CHANGE);
        
    }

    int queue = elevator_floor == FLOOR_NUM ? elevator_floor + 3 : elevator_floor;
    
    /*该楼层中LIST_QUEUE_UP队列中的人员进电梯*/
    /*LIST_QUEUE_UP_i就对应楼层数elevator_floor*/      
    while(list_size[queue] > 0 && list_size[LIST_ELEVATOR] < ELEVATOR_MAX_CAPACITY){
    
        list_remove(FIRST , queue);
        
        /*SAMPST_DELAY_UP_i也对应elevator_floor
        队列中人员开始排队的时间放入了transfer[1]*/
        sampst(sim_time - transfer[1], queue);
        sampst(sim_time - transfer[1], SAMPST_AVERAGE_DELAY);
        update_static();

        transfer[1] = sim_time;
        list_file(INCREASING, LIST_ELEVATOR);
    }

    update_not_get_in_num(queue);
    /*更新电梯的目的地,去往更高的楼层*/
    if(list_size[LIST_ELEVATOR] > 0){
        if(elevator_floor!= FLOOR_NUM){
            elevator_destination = tail[LIST_ELEVATOR]->value[2];
        }
        else{
            elevator_direction = DOWN;
            elevator_destination = head[LIST_ELEVATOR]->value[2];
        }
        
    }
}

/// 电梯下行到达事件
void elevator_arrival_DOWN(){
    /*电梯内人员先下*/
    while(list_size[LIST_ELEVATOR] > 0 && (int)tail[LIST_ELEVATOR]->value[2] == elevator_floor){
        //电梯中有人想下电梯
        list_remove(LAST, LIST_ELEVATOR);
        if(elevator_floor != 1){/*==1时，电梯中的人下电梯后，不会再回来*/
            //这人出电梯后，过一段时间后，又会回来坐电梯，把这个事件定义成他要改变楼层
            double stay_time = uniform(15.0 , 120.0 , STREAM_STAY_TIME);

            transfer[3] = elevator_floor;
            /*事件表中第三个属性，专用于记录EVENT_FLOOR_CHANGE事件中的人员的原楼层
            例如有人想在transfer[3]楼去其他楼*/
            event_schedule(sim_time + stay_time, EVENT_FLOOR_CHANGE);
        }
    }
    /*该楼层中LIST_QUEUE_DOWN队列中的人员进电梯*/
    /*LIST_QUEUE_DOWN_i就对应楼层数elevator_floor + 3*/
    /*(has fixed)ToFix:判断elevator_floor == 1 , 即一楼不会有人下行*/
    /*电梯下行到1楼，处理LIST_QUEUE_UP_1,否则处理下行队列elevator_floor + 3*/
    int queue = elevator_floor == 1 ? LIST_QUEUE_UP_1 : elevator_floor + 3;
    
    while(list_size[queue] > 0 && list_size[LIST_ELEVATOR] < ELEVATOR_MAX_CAPACITY){
        
        list_remove(FIRST , queue);

        sampst(sim_time - transfer[1], queue);
        sampst(sim_time - transfer[1], SAMPST_AVERAGE_DELAY);
        update_static();

        transfer[1] = sim_time;
        list_file(INCREASING , LIST_ELEVATOR);    
    }

    update_not_get_in_num(queue);
    /*更新电梯的目的地,去往更低的楼层*/
    if(list_size[LIST_ELEVATOR] > 0){
        if(elevator_floor != 1){
            elevator_destination = head[LIST_ELEVATOR]->value[2];
        }
        else{
            elevator_direction = UP;
            elevator_destination = tail[LIST_ELEVATOR]->value[2]; 
        }
    }
}

/// @brief  当电梯即将返回驻点时，将电梯当前所在楼层的乘客接走
void transport_people_current_floor(){
    if(elevator_direction == DOWN){
        while(list_size[elevator_floor + 3] > 0 && list_size[LIST_ELEVATOR] < ELEVATOR_MAX_CAPACITY){
            list_remove(FIRST , elevator_floor + 3);

            sampst(sim_time - transfer[1], elevator_floor + 3);
            sampst(sim_time - transfer[1], SAMPST_AVERAGE_DELAY);
            update_static();

            transfer[1] = sim_time;
            list_file(INCREASING , LIST_ELEVATOR);
        }
        update_not_get_in_num(elevator_floor + 3);
        /*更新目的地*/
        elevator_destination = list_size[LIST_ELEVATOR] > 0 ? 
                                head[LIST_ELEVATOR]->value[2] : HOME_BASE;
        
    }
    else{
        while(list_size[elevator_floor] > 0 && list_size[LIST_ELEVATOR] < ELEVATOR_MAX_CAPACITY){
            list_remove(FIRST , elevator_floor);
            sampst(sim_time - transfer[1], elevator_floor);
            sampst(sim_time - transfer[1], SAMPST_AVERAGE_DELAY);
            update_static();

            transfer[1] = sim_time;
            list_file(INCREASING , LIST_ELEVATOR);
        }
        update_not_get_in_num(elevator_floor);
        /*更新目的地*/
        elevator_destination = list_size[LIST_ELEVATOR] > 0 ? 
                                tail[LIST_ELEVATOR]->value[2] : HOME_BASE;
    }
}



/// 电梯上升到达目的地
void elevator_arrival_UP_2_destination(){
    if(elevator_floor != FLOOR_NUM){//没到顶层
        elevator_destination = has_up_passenger_above(elevator_floor);
        if(elevator_destination == elevator_floor){//上面楼层没有人要上楼了
            /*以上楼层有下楼的吗*/
            elevator_destination = has_down_passenger_above(elevator_floor);
            if(elevator_destination != elevator_floor){//上面楼层有人要下楼，电梯上行去接Ta
                event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
            }
            else{
                /*上面楼层既没有人要上楼，也没人要下楼，电梯即将下行*/
                /*当前楼层有下楼的人吗*/
                if(list_size[elevator_floor + 3] != 0){
                    elevator_direction = DOWN;
                    transport_people_current_floor();
                    event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
                }
                else{
                    /*电梯返回驻点，注：返回的途中可以搭乘同方向的人*/
                    if(elevator_floor != HOME_BASE){
                        elevator_direction = HOME_BASE > elevator_floor ? UP : DOWN;
                        elevator_destination = HOME_BASE;
                        event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
                    }
                    else{//当前就是驻点，且该驻点 != 1,因为当前电梯是上行到目的地 
                        elevator_direction = IDLE;  
                    }
                }
            }
            
            
        }
        else{
            /*电梯继续向上接客
            elevator_destination已经更新了*/
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
        }
    }
    else{//到顶层了
        if(list_size[elevator_floor + 3] != 0){/*有人想下楼*/
            elevator_direction = DOWN;
            transport_people_current_floor();
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
        }
        else{/*5楼没人想下楼,回驻点*/
            if(elevator_floor != HOME_BASE){

                elevator_direction = DOWN;
                elevator_destination = HOME_BASE;
                event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
            }
            else{
                elevator_direction = IDLE;
            }
        }
    }
}

void elevator_arrival_DOWN_2_destination(){
    if(elevator_floor != 1){//没到一楼
        elevator_destination = has_down_passenger_below(elevator_floor);
        if(elevator_destination == elevator_floor){//下面楼层没有人要下楼了
        /*下面楼层是否有上楼的人*/
            elevator_destination = has_up_passenger_below(elevator_floor);
            if(elevator_destination != elevator_floor){//下面楼层有人要上楼，电梯下行去接Ta
                event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
            }
            else{
                /*下面楼层既没有人要上楼，也没人要下楼*/
                /*当前楼层有上楼的人吗*/
                if(list_size[elevator_floor] != 0){
                    elevator_direction = UP;
                    transport_people_current_floor();
                    event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
                }
                else{/*没上楼的人，返回驻点*/
                    if(elevator_floor != HOME_BASE){
                        elevator_direction = HOME_BASE > elevator_floor ? UP : DOWN;
                        elevator_destination = HOME_BASE;
                        event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
                    }
                    else{
                        elevator_direction = IDLE;
                    }
                }
            }
            
        }
        else{//下面楼层有人要下楼，继续向下接客
            event_schedule(sim_time + 0.25 , EVENT_ELEVATOR_ARRIVAL);
        }
    }
    else{//到一楼了
        if(list_size[elevator_floor] != 0){/*有人想上楼*/
            elevator_direction = UP;
            transport_people_current_floor();
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
        }
        else{/*没人想上楼，返回驻点*/
            if(elevator_floor != HOME_BASE){
                elevator_direction = UP;
                elevator_destination = HOME_BASE;
                event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
            }
            else{
                elevator_direction = IDLE;
            }
        }
    }
}

void floor_change(){
    /*HasFix:floor_change()应该能够驱动电梯，例如驱动空闲的电梯*/
    //电梯中有人想从transfer[3]楼去其他楼
    int _floor = transfer[3];//这个人当前所在楼层
    int destination = get_floor_change_destination(_floor);/*得到其目的楼层*/
    
    if(elevator_direction == IDLE){
        //电梯空闲，驱动电梯
        if(elevator_floor == _floor){
            //电梯在_floor楼层，电梯开门接客
            transfer[1] = sim_time;
            transfer[2] = destination;
            list_file(INCREASING, LIST_ELEVATOR);
            elevator_direction = destination > _floor ? UP : DOWN;
            elevator_destination = destination;
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
        }
        else{
            //电梯不在_floor楼层，这个人排队，同时电梯前往_floor楼层接客
            transfer[1] = sim_time;
            transfer[2] = destination;

            if(destination > _floor){
                list_file(LAST, _floor);
            }
            else{
                list_file(LAST, _floor + 3);
            }

            elevator_direction = elevator_floor > _floor ? DOWN : UP;
            elevator_destination = _floor;
            event_schedule(sim_time + 0.25, EVENT_ELEVATOR_ARRIVAL);
        }
    }
    else{/*ToFix:有没有一种可能，电梯不空闲，恰好电梯在_floor楼层，电梯是否需要开门接客
           可能不需要？因为这意味着电梯到达事件与floor_change事件同时发生*/
        //电梯不空闲，排队
        transfer[1] = sim_time;
        transfer[2] = destination;

        if(destination > _floor){//这个人要上楼
            list_file(LAST, _floor);
        }
        else{
            list_file(LAST, _floor + 3);/*这个人要下楼，_floor + 3 对应LIST_QUEUE_DOWN_i*/
        }
    }
    
}

void report(){


    FILE *outfile;
    outfile = fopen("esm.out" , "w");

    fprintf(outfile , "Elevator-system-model\n\n\n\n");

    fprintf(outfile , "-----------------------------------\n");
    fprintf(outfile , "Average delay in queue in each direction\nHOME_BASE : %d\n" , HOME_BASE);
    fprintf(outfile , "LIST_QUEUE_UP_1:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_UP_1) );
    fprintf(outfile , "LIST_QUEUE_UP_2:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_UP_2) );
    fprintf(outfile , "LIST_QUEUE_UP_3:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_UP_3) );
    fprintf(outfile , "LIST_QUEUE_UP_4:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_UP_4) );
    fprintf(outfile , "LIST_QUEUE_DOWN_2:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_DOWN_2) );
    fprintf(outfile , "LIST_QUEUE_DOWN_3:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_DOWN_3) );
    fprintf(outfile , "LIST_QUEUE_DOWN_4:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_DOWN_4) );
    fprintf(outfile , "LIST_QUEUE_DOWN_5:\t%10.5f\n" , sampst(0.0 , -SAMPST_DELAY_DOWN_5) );

    fprintf(outfile , "-----------------------------------\n");
    fprintf(outfile , "total number of people : %d\n" , num_people);
    fprintf(outfile , "Average delay of all passengers:\t%10.5f\n" , sampst(0.0 , -SAMPST_AVERAGE_DELAY) );

    
    // timest(0.0 , -TIMEST_ELEVATOR_PEOPLE_NUM);
    filest(LIST_ELEVATOR);

    fprintf(outfile , "AVERAGE NUMBER OF PEOPLE IN ELEVATOR:\t%10.5f\n" , transfer[1]);
    fprintf(outfile , "MAXIMUM NUMBER OF PEOPLE IN ELEVATOR:\t%10.5f\n" , transfer[2]);
    



    fprintf(outfile , "LIST_QUEUE_UP_1_not_get_in_num : %d\n" , not_get_in_num[1]);
    fprintf(outfile , "LIST_QUEUE_UP_2_not_get_in_num : %d\n" , not_get_in_num[2]);
    fprintf(outfile , "LIST_QUEUE_UP_3_not_get_in_num : %d\n" , not_get_in_num[3]);
    fprintf(outfile , "LIST_QUEUE_UP_4_not_get_in_num : %d\n" , not_get_in_num[4]);
    fprintf(outfile , "LIST_QUEUE_DOWN_2_not_get_in_num : %d\n" , not_get_in_num[5]);
    fprintf(outfile , "LIST_QUEUE_DOWN_3_not_get_in_num : %d\n" , not_get_in_num[6]);
    fprintf(outfile , "LIST_QUEUE_DOWN_4_not_get_in_num : %d\n" , not_get_in_num[7]);
    fprintf(outfile , "LIST_QUEUE_DOWN_5_not_get_in_num : %d\n" , not_get_in_num[8]);

    timest(0.0 , -TIMEST_ELEVATOR_IDLE);
    double T_IDLE = transfer[4];
    timest(0.0 , -TIMEST_ELEVATOR_MOVING_EMPTY);
    double T_EMPTY = transfer[4];
    timest(0.0 , -TIMEST_ELEVATOR_MOVING_WITH_PEOPLE);
    double T_FILL = transfer[4];

    double T_SUM = T_IDLE + T_EMPTY + T_FILL;
    T_IDLE /= T_SUM;
    T_EMPTY /= T_SUM;
    T_FILL /= T_SUM;
    fprintf(outfile , "Proportion of time that the elevator is moving with people,moving empty,idle%10.5f  %10.5f  %10.5f\n" , T_FILL , T_EMPTY , T_IDLE);



    fclose(outfile);
}

void update_static(){
    if(elevator_direction == IDLE){
        timest(1.0 , TIMEST_ELEVATOR_IDLE);
        timest(0.0 , TIMEST_ELEVATOR_MOVING_EMPTY);
        timest(0.0 , TIMEST_ELEVATOR_MOVING_WITH_PEOPLE);
    }
    else if(list_size[LIST_ELEVATOR] == 0){
        timest(0.0 , TIMEST_ELEVATOR_IDLE);
        timest(1.0 , TIMEST_ELEVATOR_MOVING_EMPTY);
        timest(0.0 , TIMEST_ELEVATOR_MOVING_WITH_PEOPLE);
    }
    else{
        timest(0.0 , TIMEST_ELEVATOR_IDLE);
        timest(0.0 , TIMEST_ELEVATOR_MOVING_EMPTY);
        timest(1.0 , TIMEST_ELEVATOR_MOVING_WITH_PEOPLE);
    }
}

void update_not_get_in_num(int queue){
    if(list_size[queue] != 0){
        not_get_in_num[queue] += list_size[queue]; 
    }
}

int determine_destination(int stream){
    double r = lcgrand(stream);
    if (r < 0.25) return 2;
    if (r < 0.50) return 3;
    if (r < 0.75) return 4;
    return 5;
}

int get_floor_change_destination(int _floor){
    //电梯中有人想从_floor楼去其他楼,得到其目的楼层
    //0.7的概率去一楼，0.3的概率去其他楼层
    double r = lcgrand(STREAM_NEXT_FLOOR);
    if(r < 0.7) return 1;
    
    // 剩下0.3的概率平分给除了_floor和1之外的楼层
    r = lcgrand(STREAM_NEXT_FLOOR); // 重新生成随机数

    int floor[3];
    for(int i = 2 , j = 0; i <= 5 ; i++){
        if(i != _floor){
            floor[j++] = i;
        }
    }
    if(r < (double)1/3) return floor[0];
    if(r < (double)2/3) return floor[1];
    return floor[2];
}

int has_up_passenger_above(int cur_floor){
    /*当电梯上行，到达目的地时，电梯中没人，调用该函数获取新的目的地*/
    /*电梯上行时，当前楼层cur_floor(cur_floor < FLOOR_NUM)以上的楼层是否有人想上楼
    若有，则电梯的目的地更新为最近想上楼的人所在的楼层*/
    int elevator_destination_update = cur_floor;
    for(int i = cur_floor ; i < FLOOR_NUM ; i++){
        if(list_size[i] > 0){
            elevator_destination_update = i;
            break;
        }
    }
    return elevator_destination_update;
}

int has_down_passenger_above(int cur_floor){
    /*当电梯上行，到达目的地时，电梯中没人，调用该函数获取新的目的地*/
    /*电梯上行时，当前楼层cur_floor(cur_floor > 1)以上的楼层是否有人想下楼
    若有，则电梯的目的地更新为最近想下楼的人所在的楼层*/
    int elevator_destination_update = cur_floor;
    for(int i = FLOOR_NUM ; i > cur_floor ; i--){
        if(list_size[i + 3] > 0){
            elevator_destination_update = i;
            break;
        }
    }
    return elevator_destination_update;
}

int has_down_passenger_below(int cur_floor){
    /*当电梯下行，到达目的地时，电梯中没人，调用该函数获取新的目的地*/
    /*电梯下行时，当前楼层cur_floor(cur_floor > 1)以下的楼层是否有人想下楼
    若有，则电梯的目的地更新为最近想下楼的人所在的楼层*/
    int elevator_destination_update = cur_floor;
    for(int i = cur_floor; i > 1 ; i--){
        if(list_size[i + 3] > 0){
            elevator_destination_update = i;
            break;
        }
    }
    return elevator_destination_update;
}

int has_up_passenger_below(int cur_floor){
    /*当电梯下行，到达目的地时，电梯中没人，调用该函数获取新的目的地*/
    /*电梯下行时，当前楼层cur_floor(cur_floor < FLOOR_NUM)以下的楼层是否有人想上楼
    若有，则电梯的目的地更新为最近想上楼的人所在的楼层*/
    int elevator_destination_update = cur_floor;

    for(int i = 1 ; i < cur_floor  ; i++){
        if(list_size[i] > 0){
            elevator_destination_update = i;
            break;
        }
    }
    return elevator_destination_update;
}
///for debug : print the content of the list

void print_list(int list){
    if(list_size[list] == 0){
        printf("the list is empty\n");
    }

    struct master* cur = head[list];
    while(cur != NULL){
        printf("%f\t%f\n" , cur->value[1] , cur->value[2]);

        cur = cur->sr;
    }
}

std::string get_list(int list){
    if(list_size[list] == 0){
        return "";
    }

    struct master* cur = head[list];
    std::string res , _tmp;
    while(cur != NULL){
        _tmp = "(" + std::to_string(cur->value[1]) + "," + std::to_string(int(cur->value[2])) + "), ";
        res += _tmp;
        cur = cur->sr;
    }

    return res;
}

void output_log(std::ofstream& log){
    log << "---------------------------------------------\n";
    log << "sim_time : " << sim_time << "\n" ;
    log << "elevator_floor : " << elevator_floor << "\t\t elevator_direction : " << elevator_direction << "\t\televator_destination : " << elevator_destination << "\n";
    log << "elevator_people : " << get_list(LIST_ELEVATOR) << "\n";
    log << "LIST_QUEUE_UP_1 : " << get_list(LIST_QUEUE_UP_1) << "\n";
    log << "LIST_QUEUE_UP_2 : " << get_list(LIST_QUEUE_UP_2) << "\n";
    log << "LIST_QUEUE_UP_3 : " << get_list(LIST_QUEUE_UP_3) << "\n";
    log << "LIST_QUEUE_UP_4 : " << get_list(LIST_QUEUE_UP_4) << "\n";
    log << "LIST_QUEUE_DOWN_2 : " << get_list(LIST_QUEUE_DOWN_2) << "\n";
    log << "LIST_QUEUE_DOWN_3 : " << get_list(LIST_QUEUE_DOWN_3) << "\n";
    log << "LIST_QUEUE_DOWN_4 : " << get_list(LIST_QUEUE_DOWN_4) << "\n";
    log << "LIST_QUEUE_DOWN_5 : " << get_list(LIST_QUEUE_DOWN_5) << "\n";

}