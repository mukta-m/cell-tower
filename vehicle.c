#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"

#define PI 3.14159265358979323846

// GPS Data for this client as well as the connected tower ID
short  x;
short  y;
short  direction;
char  connectionID;
char  connectedTowerID;

int main(int argc, char * argv[]) {
  int                 clientSocket;
  struct sockaddr_in  clientAddress;
  int                 status, bytesRcv;
  unsigned char       buffer[80];   // stores sent and received data
  unsigned char       x1, x2, y1, y2;
  Vehicle             vehicle;

  // Create socket
  clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (clientSocket < 0) {
    printf("*** CLIENT ERROR: Could open socket.\n");
    exit(-1);
  }

  // Set up the random seed
  srand(time(NULL));

  // To start, this vehicle is not connected to any cell towers
  connectionID = -1;
  connectedTowerID = -1;

  // Get the starting coordinate and direction from the command line arguments
  x = atoi(argv[1]);
  y = atoi(argv[2]);
  direction = atoi(argv[3]);


  // Go into an infinite loop to keep sending updates to cell towers
  // while the vehicle is still in city range or connected to tower
  while(((x >= 0 || x <= CITY_WIDTH) && (y >= 0 || y <= CITY_HEIGHT)) || (connectedTowerID != -1)) {
    usleep(50000);  // A delay to slow things down a little

    int num = rand() % 3; // yields result between 0-2

    // switch statement for direction to change 1/3 of time
    switch(num){
      case 0: direction = direction - VEHICLE_TURN_ANGLE;
              break;
      case 1: direction = direction + VEHICLE_TURN_ANGLE;
              break;
      case 2: direction = direction;
              break;
    };

    // change x and y according to switch statement
    x = x + (VEHICLE_SPEED * cos(direction * (PI/180)));
    y = y  + (VEHICLE_SPEED * sin(direction * (PI/180)));

    // if vehicle is not connected to any tower
    if (connectedTowerID == -1){
      // iterate through all cell towers
      for(int i = 0; i < NUM_TOWERS; i++){

        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket < 0) {
          printf("*** CLIENT ERROR: Could open socket.\n");
          exit(-1);
        }
          // Setup address
          memset(&clientAddress, 0, sizeof(clientAddress));
          clientAddress.sin_family = AF_INET;
          clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
          clientAddress.sin_port = htons((unsigned short) SERVER_PORT + i);

          // Connect to server
          status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
          if (status < 0) {
            printf("*** CLIENT ERROR: Could not connect.\n");
            exit(-1);
          }

          // send connect buffer
          buffer[0] = CONNECT;
          printf("CLIENT: sending a connect message as %u\n", buffer[0]);
          // split x and y value into 2 unsigned chars, to send in buffer
          x1 = x % 256;
          x2 = x / 256;

          buffer[1] = x1;
          buffer[2] = x2;

          y1 = y % 256;
          y2 = y / 256;

          buffer[3] = y1;
          buffer[4] = y2;

          buffer[5] = '0';

          printf("CLIENT: Current coordinate num of (%d, %d)\n", x, y);

          send(clientSocket, buffer, sizeof(buffer), 0);

          // Get response from server
          bytesRcv = recv(clientSocket, buffer, 80, 0);
          buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string

          printf("CLIENT: Got back response \"%u\" from server.\n", buffer[0]);

          // if response is a yes, change connectedTowerID and connectionID
          if (buffer[0] == YES){
            connectedTowerID = i;
            connectionID = buffer[1];
            break;
          }
      }
    }
    else{
      // if connected, make sure it is still within bounds of screen, if not, let server know and break
      if (((x < 0 || x > CITY_WIDTH) && (y < 0 || y > CITY_HEIGHT))){
        connectedTowerID = -1;
        connectionID = -1;
        buffer[0] = 0;
        buffer[1] = 0;
        buffer[2] = 0;
        buffer[3] = 0;
        buffer[3] = 0;
        buffer[5] = connectionID;
        buffer[6] = -1;

        send(clientSocket, buffer, sizeof(buffer), 0);
        bytesRcv = recv(clientSocket, buffer, 80, 0);
        break;
      }
      memset(&clientAddress, 0, sizeof(clientAddress));
      clientAddress.sin_family = AF_INET;
      clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
      clientAddress.sin_port = htons((unsigned short) SERVER_PORT + connectedTowerID);

      // Connect to server
      status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
      if (status < 0) {
        printf("*** CLIENT ERROR: Could not connect.\n");
        exit(-1);
      }
      // split x and y value into 2 unsigned chars, to send in buffer
      x1 = x % 256;
      x2 = x / 256;

      buffer[1] = x1;
      buffer[2] = x2;

      y1 = y % 256;
      y2 = y / 256;

      buffer[3] = y1;
      buffer[4] = y2;

      // 5th value is index
      buffer[5] = connectionID;

      buffer[6] = '0';

      send(clientSocket, buffer, sizeof(buffer), 0);

      // Get response from server
      bytesRcv = recv(clientSocket, buffer, 80, 0);
      buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
      printf("CLIENT: Got back response \"%s\" from server.\n", buffer);
      // if buffer is no longer in range, no longer connected
      if (buffer[0] == NO){
        connectionID = -1;
        connectedTowerID = -1;
        break;
      }
    }
  }
  close(clientSocket);  // Don't forget to close the socket !
  printf("CLIENT: Shutting down.\n");
  }
