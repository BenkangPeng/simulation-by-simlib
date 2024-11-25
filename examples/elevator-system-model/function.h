#include<fstream>
void arrive();
void elevator_arrival();
void floor_change();
void report();
int determine_destination(int stream);
int get_floor_change_destination(int _floor);
int has_up_passenger(int cur_floor);
int has_down_passenger(int cur_floor);
void elevator_arrival_UP();
void elevator_arrival_DOWN();
void elevator_arrival_UP_2_destination();
void elevator_arrival_DOWN_2_destination();
void update_static();
void update_not_get_in_num(int queue);
void output_log(std::ofstream& log);
std::string get_list(int list);

void transport_people_current_floor();
