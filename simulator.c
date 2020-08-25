#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "simulator.h"

#include "display.c"
#include "cellTower.c"

int main() {
  City     ottawa;

  // Cell tower data
  short    xLocations[7] = {100, 400, 650, 640, 400, 120, 200};
  short    yLocations[7] = {100, 200, 150, 450, 500, 350, 500};
  short     cellRadii[7] = {110, 200, 150, 180, 160, 120, 100};
  int      cellColors[7] = {0xFF8822 /*Orange*/, 0xFF2222 /*Red*/, 0xFFFF44 /*Yellow*/,
			    0x22FF22 /*Green*/, 0xAA66FF /*Purple*/, 0x0099FF /*Blue*/,
			    0x999999 /*LightGray*/};

  // Set up the Cell Towers with the above data ... and no connected vehicles to begin
  ottawa.numTowers = 7;
  for (int i=0; i<ottawa.numTowers; i++) {
    ottawa.towers[i].online = 1;
    ottawa.towers[i].id = (unsigned char)i;
    ottawa.towers[i].x = xLocations[i];
    ottawa.towers[i].y = yLocations[i];
    ottawa.towers[i].radius = cellRadii[i];
    ottawa.towers[i].color = cellColors[i];
    ottawa.towers[i].numConnectedVehicles = 0;
    for (int j=0; j<MAX_CONNECTIONS; j++)
      ottawa.towers[i].connectedVehicles[j].connected = 0;
  }

  // Remove the line of code below.  Add code to spawn the necessary threads and
  // wait for their completion before exiting gracefully with some kind of message
  pthread_t t, t1, t2, t3, t4, t5, t6, t7;

  // create thread for display
  pthread_create(&t, NULL, showSimulation, &ottawa);

  // array of pthread_t
  pthread_t threads[7] = {t1, t2, t3, t4, t5, t6, t7};

  // create thread with each in array
  for (int i = 0; i < ottawa.numTowers; i++){
    pthread_create(&threads[i], NULL, handleIncomingRequests, &ottawa.towers[i]);
  }

  // join all threads 
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
  pthread_join(threads[3], NULL);
  pthread_join(threads[4], NULL);
  pthread_join(threads[5], NULL);
  pthread_join(threads[6], NULL);



}
