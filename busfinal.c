/*
TungHo Lin
Rongjie Zeng
Final Project
BusRoute
*/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>

#define RED 0
#define GREEN 1
#define BUS1 3
#define BUS2 4

//semaphore mutex that represents a lock that allows one bus moves at a time
//semaphore next that synchronizes the threads so that one bus will wait for the other to finish before moving
sem_t mutex, next;
//semaphores that controls the lights at the intersection
sem_t horizontal_light, vertical_light;
//a 5x5 grid representing a map
int grid[5][5];
//a 2D array that represents the x and y-coord of each point on a route
int routeOne[9][2] = {{0,0}, {1,0}, {2,0}, {2,1}, {2,2}, {2,3}, {2,4}, {3,4}, {4,4}};
int routeTwo[9][2] = {{4,0}, {4,1}, {4,2}, {3,2}, {2,2}, {1,2}, {0,2}, {0,3}, {0,4}};
//an array storing the x-coord and y-coord of the intersections 
int intersections[1][2] = {{2,2}};
//a done flag that will be incremented to 1 when all the buses are done 
int done = -1;
//thread methods
void *route1();
void *route2();
void *trafficLight();

int main() {

	sem_init(&mutex, 0, 1);
	sem_init(&next, 0, 0);
	sem_init(&horizontal_light, 0, RED);
	sem_init(&vertical_light, 0, GREEN);
	
	pthread_t tid1, tid2, lights;

	pthread_create(&tid1, NULL, route1, NULL);

	pthread_create(&tid2, NULL, route2, NULL);
	
	pthread_create(&lights, NULL, trafficLight, NULL);
	
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_join(lights, NULL);
	sem_destroy(&mutex);
	sem_destroy(&horizontal_light);
	sem_destroy(&vertical_light);
	return 0;
}

//route that bus1 runs on
void *route1() {
	//count the number of steps
	int steps = 0;
	//x and y set to the starting point
	int x = routeOne[steps][0];
	int y = routeOne[steps][1];
	//the while loop stops if the bus reaches the destination
	while(steps < 9) {
		//if the current position is an intersection
		if(x == intersections[0][0] && y == intersections[0][1]) {
			//wait for a green vertical light
			sem_wait(&vertical_light);
			printf("Bus1 at intersection(%d, %d)\n", x, y);
			//ensure only one bus move at a time
			sem_wait(&mutex);
			//move BUS1 into the intersection
			grid[x][y] = BUS1;
			sem_post(&mutex);
			sem_post(&vertical_light);
			steps++;
			sleep(1);
			//set x and y to the next point
			x = routeOne[steps][0];
			y = routeOne[steps][1];
			sem_wait(&mutex);
			//move BUS1 out of the intersection 
			grid[x][y] = BUS1;
			sem_post(&mutex);
			printf("Bus1 at (%d, %d)\n", x, y);
			steps++;
			//set x and y to the next point
			x = routeOne[steps][0];
			y = routeOne[steps][1];
		}
		//if the current position is not an intersection
		else {
			sem_wait(&mutex);
			//move BUS1 to current position
			grid[x][y] = BUS1;
			sem_post(&mutex);
			printf("Bus1 at (%d, %d)\n", x, y);
			steps++;
			//set x and y to the next point
			x = routeOne[steps][0];
			y = routeOne[steps][1];
		}
		sleep(1);
		//If none of the bus has arrvied at its destination, wait for bus2 to finish
		if(done == -1)
			sem_wait(&next);
	}
	//print the final destination of the bus
	printf("Bus1 has arrived at its destination\n");
	//increment done flag
	done++;
	pthread_exit(0);
}

//route that bus2 runs on
void *route2() {
	//count the number of steps
	int steps = 0;
	//x and y set to the starting point
	int x = routeTwo[steps][0];
	int y = routeTwo[steps][1];
	while(steps < 9) {
		
		//if the current position is an intersection
		if(x == intersections[0][0] && y == intersections[0][1]) {
			sem_wait(&horizontal_light);
			printf("Bus2 at intersection(%d, %d)\n", x, y);
			sem_wait(&mutex);
			grid[x][y] = BUS2;
			sem_post(&mutex);
			sem_post(&horizontal_light);
			steps++;
			sleep(1);
			x = routeTwo[steps][0];
			y = routeTwo[steps][1];
			sem_wait(&mutex);
			grid[x][y] = BUS2;
			sem_post(&mutex);	
			printf("Bus2 at (%d, %d)\n", x, y);
			steps++;
			x = routeTwo[steps][0];
			y = routeTwo[steps][1];
		}
		//if the current position is not an intersection
		else {
			sem_wait(&mutex);
			grid[x][y] = BUS2;
			sem_post(&mutex);	
			printf("Bus2 at (%d, %d)\n", x, y);
			steps++;
			x = routeTwo[steps][0];
			y = routeTwo[steps][1];
		}
		sleep(1);
		//signal the other bus that bus 2 has finished its step
		sem_post(&next);
	}
	printf("Bus2 has arrived at its destination\n");
	//increment done flag
	done++;
	pthread_exit(0);
}

//trafficLight thread that will change the lights with respect to time
void *trafficLight() {
	//keeps changing the lights until both buses reach their destinations
	while(done != 1) {
		//sleep for 2 sec
		sleep(2);
		//switch the lights
		sem_wait(&vertical_light);
		sem_post(&horizontal_light);
		//sleep for 2 sec again
		sleep(2);
		//switch the lights again
		sem_wait(&horizontal_light);
		sem_post(&vertical_light);
	}
	pthread_exit(0);
}