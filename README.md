# Overview
These are the assignments that we had to do for the Operating Systems class ([HY345](https://www.csd.uoc.gr/~hy345/))

# Assignment 1
In the first assignment we had to make our own shell that can run commands and also supports the use of global variables, pipes and even some signals such as SIGINT, SIGTSTP, SIGCONT, SIGQUIT.

### How to Run
Use `make` in the assignment1 directory and then simply run the executable named shell : `./shell` 

# Assignment 2
The aim of the second assignment was to get familiar with threads. The concept is that there is a narrow road where the width is large enough only for one person, so if two people moving in opposite directions meet one has to go on the side and wait
for the other to pass.
### Rules
* The pedestrians crossing the road can only be wearing red or blue clothes
* All the pedestrians want to cross the road. For a pedestrian to cross the road he has to pass by all the positions of the road. A pedestrian can move forward (to the next position) or to the side (to move out the way) only if there is an empty space there. A pedestrian can't move if the person in front of him hasn't moved.
* In the occasion that two pedestrians meet of different directions, the one that passes first is the one that has the color of the majority and the direction of the majority as well.
* The pedestrians of the other direction follow but of the same color follow next. That leaves only one color of pedestrians on the road.
* Next move the pedestrians that are larger in number for a specific direction.
* The pedestrians that want to go in the other direction follow.
* The queue must be maintained on both sides. A pedestrian of the same color and direction as the one in front of him cant pass him and exit the road earlier.

### Pedestrian Initialization
* The pedestrians must have a distinct ID, a direction and a color.
* Each pedestrian moves towards one direction each time, east or west. The direction and color of each pedestrian should be random.
### Road Initialization
* The length of the road is the amount of pedestrians that fit lined up one in front on the other.
* Pedestrians must be added randomly on the road but inserted on the opposite side of the one they are traveling towards (a pedestrian going east will be inserted on the west side of the road).

### How to Run
Use `make` in the assignment2 directory and then run the executable nr as follows: `./nr -p <num of people on road>`

The number of people must be between 0 and 100

# Assignment 3
In the third assignment we had to implement system calls for "shortest task first" scheduling policy. According to this policy each process informs the scheduler of its deadline (the time that it has to have finished by) but also its estimated runtime.
We had to implement two new system calls:
* set_task_params(int deadline, int estimated_runtime)
* get_task_params(struct d_params* params)

We also had to add the two fields (deadline and estimated_runtime) in the task struct of the kernel
### Notes
+ The kernel used for this assignment is 2.6.38.1
