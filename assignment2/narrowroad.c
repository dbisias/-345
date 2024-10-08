#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#define RED 0
#define BLUE 1
#define EAST 0
#define WEST 1

typedef struct person{
    int id;
    int color;
    int direction;
    int on_sidewalk;
} person_t;

typedef struct road_pos{
    person_t* road;
    person_t* sidewalk;
    int pos;
} position_t;

void red();
void blue();
void reset_color();
void color(int c);
void print_road();
person_t* get_person(int id);
int init_road();
void* move_people(void* arg);
int all_empty();


//global variables and semaphores
position_t *road_pos;
int moving_direction, moving_color;
int people;
int red_west = 0, red_east = 0, blue_west = 0, blue_east = 0;
int color_has_changed = 0, direction_has_changed = 0; // flags
sem_t move_person;

void main(int argc, char** argv) {
    srand(time(NULL));
    people = atoi(argv[2]);
    if(argc != 3 || strcmp(argv[1], "-p") != 0 || people <= 0 || people > 100) {
        printf("Usage: %s -p [0 < number of people <= 100]\n", argv[0]);
        return;
    }

    if(!init_road()){
        printf("Error while creating road. Exiting..\n");
        exit(1);
    }

    //find starting color
    if(red_east + red_west == blue_east + blue_west)
        moving_color = rand() % 2;
    else if(red_east + red_west > blue_east + blue_west) // if red > blue moving_color is red
        moving_color = RED;
    else // otherwise its blue
        moving_color = BLUE;
    
    //find starting direction
    if(moving_color == RED) {
        if(red_east == red_west)
            moving_direction = rand() % 2;
        else if(red_east > red_west) {
            moving_direction = EAST;
        }
        else 
            moving_direction = WEST;
    }
    else {
        if(blue_east == blue_west)
            moving_direction = rand() % 2;
        else if(blue_east > blue_west) {
            moving_direction = EAST;
        }
        else 
            moving_direction = WEST;
    }

    //init thread array and semaphores
    pthread_t threads[people];
    sem_init(&move_person, 0, 1);

    //create threads
    for(int i = 0; i < people; i++) {
        pthread_create(&threads[i], NULL, move_people, road_pos + i);
    }

    //wait for threads to finish
    for(int i = 0; i < people; i++) {
        pthread_join(threads[i], NULL);
    }

}

//print road and sidewalk
void print_road() {
    //for road
    printf("||");
    for(int i = 0; i < people; ++i) {
        if(road_pos[i].road == NULL) {
            printf("   ||");
        }
        else {
            if(road_pos[i].road->direction == WEST) {
                color(road_pos[i].road->color);
                printf("<%02d", road_pos[i].road->id);
                reset_color();
                printf("||");
            }
            else {
                color(road_pos[i].road->color);
                printf("%02d>", road_pos[i].road->id);
                reset_color();
                printf("||");
            }  
        }
    }

    //for sidewalk
    printf("\n||");
    for(int i = 0; i < people; ++i) {
        if(road_pos[i].sidewalk == NULL) {
            printf("   ||");
        }
        else {
            if(road_pos[i].sidewalk->direction == WEST) {
                color(road_pos[i].sidewalk->color);
                printf("<%02d", road_pos[i].sidewalk->id);
                reset_color();
                printf("||");
            }
            else {
                color(road_pos[i].sidewalk->color);
                printf("%02d>", road_pos[i].sidewalk->id);
                reset_color();
                printf("||");
            }  
        }
    }
    printf("\n\n");
}


void red() {
    printf("\033[1;31m");
}
void blue() {
    printf("\033[0;34m");
}
void reset_color() {
    printf("\033[0m");
}
void color(int c){
    c ? blue():red();
}

//return a new randomised person with id i
person_t* get_person(int id) {
    person_t* person = (person_t*)malloc(sizeof(person_t));
    person->id = id;
    person->color = rand() % 2; // 0 or 1
    person->direction = rand() % 2;
    person->on_sidewalk = 0;
    return person;
}

//initialise road
int init_road() {
    road_pos = (position_t*)malloc(people * sizeof(position_t));    
    
    int going_east = 0;
    int going_west = people - 1;
    person_t *p;

    for(int i = 0; i < people; ++i) {
        p = get_person(i);
        road_pos[i].pos = i;

        //count peoples color and directions
        if(p->direction == WEST) {
            if(p->color == RED)
                ++red_west;
            else
                ++blue_west;
        }
        else { //EAST
            if(p->color == RED)
                ++red_east;
            else
                ++blue_east;
        }

        if(going_east > going_west) //if index of east is larger than west an error has happened
            return 0;
        if(p->direction == EAST) {
            // p.road_pos = going_east;
            road_pos[going_east].road = p;
            road_pos[going_east].sidewalk = NULL;
            ++going_east;
        }
        else if(p->direction == WEST) {
            // p.road_pos = going_west;
            road_pos[going_west].road = p;
            road_pos[going_west].sidewalk = NULL;
            --going_west;
        }
    }
    print_road();
    return 1;
}

//find road position address of index i and return it
position_t* find_pos(int id) {
    for(int i = 0; i < people; ++i) {
        if(road_pos[i].road->id == id){
            return road_pos + i; //return found position
        }
    }
}

//check if moving_color needs to change
void c_check_change() {
    if(!color_has_changed) {
        if(moving_color == RED) {
            if(!(red_east + red_west)) { // there are no red people left on the road
                moving_color = BLUE;
                color_has_changed = 1;
            }
        }
        else {
            if(!(blue_west + blue_east)) { // there are no blue people left on the road
                moving_color = RED;
                color_has_changed = 1;
            }
        }
    }
}

//check if moving_direction needs to change
void d_check_change() {
    if(moving_color == RED) {
        //if it has changed once and the other color is red, check for most populated side
        if(color_has_changed == 1) {
            if(red_east == red_west)
                moving_direction = rand() % 2;
            else if(red_east > red_west)
                moving_direction = EAST;
            else
                moving_direction = WEST;
            ++direction_has_changed;
            ++color_has_changed; // change it here only so it doesnt come back to this if statement
        }
        if(moving_direction == EAST && red_east == 0) {
            moving_direction = WEST;
            ++direction_has_changed;
        }
        else if(moving_direction == WEST && red_west == 0) {
            moving_direction = EAST;
            ++direction_has_changed;
        }
    }
    else { // moving_color is blue
         //if color has changed check for most populated side
        if(color_has_changed == 1) {
            if(blue_east == blue_west)
                moving_direction = rand() % 2;
            else if(blue_east > blue_west)
                moving_direction = EAST;
            else
                moving_direction = WEST;
            color_has_changed++; // change it here only so it doesnt come back to this if statement
        }
        if(moving_direction == EAST && blue_east == 0) {
            moving_direction = WEST;
            ++direction_has_changed;
        }
        else if(moving_direction == WEST && blue_west == 0) {
            moving_direction = EAST;
            ++direction_has_changed;
        }
    }
}

// function that threads use to move the people
void* move_people(void* arg) {
    position_t* position = (position_t*) arg;
    int this_direction, this_color;

    while(1) {
        sem_wait(&move_person);
        if(position->sidewalk != NULL && position->road == NULL) { //if there is someone on the sidewalk and road is empty, move him to road
            position->road = position->sidewalk;
            position->sidewalk = NULL;
            position->road->on_sidewalk = 0;
        }
        if(position->road) { // if there is a person on the road
            this_color = position->road->color;
            this_direction = position->road->direction;
            if(position->road->direction == moving_direction && position->road->color == moving_color) { // if persons direction/color is same as the moving one 
                if(position->road->direction == WEST) {  
                    if(position->pos == 0) {// left-most position on road, so going west leaves the road
                        if(position->road->color == RED)
                            --red_west;
                        else    
                            --blue_west;
                        
                        // free(position->road);
                        position->road = NULL;

                        
                        //after someone leaves check if color/direction needs change
                        c_check_change(); //check for color first
                        d_check_change();
                        print_road();
                    }
                    else { // not an edge position
                        if(!(position-1)->road) { // if the next road position is empty, move person there
                            (position-1)->road = position->road;
                            position->road = NULL;
                            print_road();
                        }
                        else if(!((position-1)->road->color == this_color && (position-1)->road->direction == this_direction)) {
                            if(!(position-1)->sidewalk) { //if no one on next position sidewalk
                                (position-1)->sidewalk = (position-1)->road; // move next person to sidewalk
                                (position-1)->sidewalk->on_sidewalk = 1;
                                (position-1)->road = position->road; // move this person to next position road
                                position->road = NULL;
                                print_road();
                            }
                        }
                    }
                }
                else { // direction is EAST
                    if(position->pos == people - 1) { //right-most position on road, so going east leaves the road
                        if(position->road->color == RED)
                            --red_east;
                        else    
                            --blue_east;

                        // free(position->road);
                        position->road = NULL;
                        //after someone leaves check if color/direction needs change
                        c_check_change(); //check for color first
                        d_check_change();
                        print_road(); // this was added after the submission
                    }
                    else { // not an edge position
                        if((position+1)->road == NULL) { // if the next road position is empty, move person there
                            (position+1)->road = position->road;
                            position->road = NULL;
                            print_road();
                        }
                        else if(!((position+1)->road->color == this_color && (position+1)->road->direction == this_direction)) {
                            if(!(position+1)->sidewalk) { //if no one on next position sidewalk
                                (position+1)->sidewalk = (position+1)->road; // move next person to sidewalk
                                (position+1)->sidewalk->on_sidewalk = 1;
                                (position+1)->road = position->road; // move this person to next position road
                                position->road = NULL;
                                print_road();
                            }
                        }
                    }
                }
            }
        }
        
        if(all_empty()) //if all the road positions are empty then exit
            exit(0);

        sem_post(&move_person);
    }
}

//check if the whole road is empty
int all_empty() {
    for(int i = 0; i < people; ++i) {
        if(road_pos[i].road || road_pos[i].sidewalk)
            return 0;
    }
    return 1;
}