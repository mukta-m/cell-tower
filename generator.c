#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "simulator.h"

void main() {
  // Set up the random seed
  srand(time(NULL));

  while(1) {
    for (int i=0; i<5; i++) {
      // Start off with a random location and direction
      short x = (int)(rand()/(double)(RAND_MAX)*CITY_WIDTH);
      short y = (int)(rand()/(double)(RAND_MAX)*CITY_HEIGHT);
      short direction = (int)((rand()/(double)(RAND_MAX))*360 - 180);
      // create array to hold command
      char commandprompt[30];
      // copy variables into array to create prompt
      sprintf(commandprompt, "./vehicle %d %d %d &", x, y, direction);
      // run entire prompt
      system(commandprompt);
    }
    sleep(1);
  }
}
