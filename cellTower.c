#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>



// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handles are SHUTDOWN, CONNECT and UPDATE.  A SHUTDOWN request causes the tower to
// go offline.   A CONNECT request contains 4 additional bytes which are the high and
// low bytes of the vehicle's X coordinate, followed by the high and low bytes of its
// Y coordinate.  If within range of this tower, the connection is accepted and a YES
// is returned, along with a char id for the vehicle and the tower id.   If UPDATE is
// received, the additional 4 byes for the (X,Y) coordinate are also received as well
// as the id of the vehicle.   Then YES is returned if the vehicle is still within
// the tower range, otherwise NO is returned.
void *handleIncomingRequests(void *ct) {
  CellTower       *tower = ct;

  int                 serverSocket, clientSocket;
  struct sockaddr_in  serverAddress, clientAddr;
  int                 status, addrSize, bytesRcv;
  unsigned char       buffer[30];
  char*               response = "OK";
  int                 updateX, updateY;

  // Create the server socket
  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverSocket < 0) {
    printf("*** SERVER ERROR: Could not open socket.\n");
    exit(-1);
  }

  // Setup the server address
  memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons((unsigned short) SERVER_PORT + (tower-> id));

  // Bind the server socket
  status = bind(serverSocket,  (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (status < 0) {
    printf("*** SERVER ERROR: Could not bind socket.\n");
    exit(-1);
  }

  // Set up the line-up to handle up to 5 clients in line
  status = listen(serverSocket, 5);
  if (status < 0) {
    printf("*** SERVER ERROR: Could not listen on socket.\n");
    exit(-1);
  }

  // Wait for clients now
  while (1) {
    addrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
    if (clientSocket < 0) {
      printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
      exit(-1);
    }
    printf("SERVER: Received client connection.\n");

// set up infinite loop for server
    while (1){
      // Get the message from the client
      bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);

      buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
      printf("SERVER: Received client request of %u\n", buffer[0]);

      // if shutdown, break out of loop and close server socket
      if(buffer[0] == SHUTDOWN){
        break;
      }

      // if client wants to connected
      if(buffer[0] == CONNECT){
        // check if theres room in array of connected vehicles
        if (tower->numConnectedVehicles >= MAX_CONNECTIONS){
          buffer[0] = NO;
          buffer[1] = '0';
          send(clientSocket, buffer, sizeof(buffer), 0);
          break;
        }

        // convert x and y values from 2 values into 1 each
        updateX = buffer[1] + (buffer[2] * 256);
        updateY = buffer[3] + (buffer[4] * 256);

        printf("SERVER: Client coordinate is (%d, %d).\n", updateX, updateY);

        // calculate using distance formula from center of tower to vehicle
        int dist = sqrt( pow((tower->x)-(updateX), 2) +  pow((tower->y)-(updateY), 2)   );

        // if distance is less than or equal to radius
        if ((dist <= tower->radius)){
          int index = -1; // flag for if spot in array
          // iterate through array
          for (int j=0; j < MAX_CONNECTIONS; j++){
            // if there is spot that vehicle is not connected, enter it there
            if (tower->connectedVehicles[j].connected == 0){
              index = j;
              tower->numConnectedVehicles++;
              break;
            }
          }
          // index value was changed, therefore, a spot exists
          if (index != -1){
            // create new vehicle object w x y and connected as '1'
            ConnectedVehicle newV;
            newV.x = updateX;
            newV.y = updateY;
            newV.connected = 1;
            tower->connectedVehicles[index] = newV;

            // send back YES, index in array
            buffer[0] = YES;
            buffer[1] = index;
            buffer[2] = '0';
            send(clientSocket, buffer, sizeof(buffer), 0);
            break;
          }
          else{
            // otherwise, no space, send back NO
            buffer[0] = NO;
            buffer[1] = '0';
            send(clientSocket, buffer, sizeof(buffer), 0);
            break;
          }

        }
        // not in range, NO
        else{
          buffer[0] = NO;
          buffer[1] = 0;
          send(clientSocket, buffer, sizeof(buffer), 0);
          break;
        }
      }
      // if vehicle wants to update location
      if(buffer[0] == UPDATE){
        // vehicle's index in array
        int vehicleindex = buffer[5];

        // if vehicle is out of bounds, turn its connected to 0;
        if(buffer[6] == -1){
          tower->connectedVehicles[vehicleindex].connected = 0;
          buffer[0] = NO;
          buffer[1] = '0';
          send(clientSocket, buffer, sizeof(buffer), 0);

          break;
        }

        // turn buffer x and y values into actual x and y values
        updateX = buffer[1] + (buffer[2] * 256);
        updateY = buffer[3] + (buffer[4] * 256);



        // determine distance from center
        int distance = sqrt( pow((tower->x)-(updateX), 2) +  pow((tower->y)-(updateY), 2)   );


        // if distance is in range, update location in array
        if (distance <= tower->radius){
          tower->connectedVehicles[vehicleindex].x = updateX;
          tower->connectedVehicles[vehicleindex].y = updateY;

          buffer[0] = YES;
          buffer[1] = '0';
          send(clientSocket, buffer, sizeof(buffer), 0);
          break;

        }
        // otherwise, not in range anymore, "remove" from array
        else{
          tower->connectedVehicles[vehicleindex].connected = 0;
          tower->numConnectedVehicles--;
          // send back NO
          buffer[0] = NO;
          buffer[1] = '0';
          send(clientSocket, buffer, sizeof(buffer), 0);
          break;
        }
      }
    }
    // close client once break out of first loop
    printf("SERVER: Closing client connection.\n");
    close(clientSocket);

    // if buffer said to SHUTDOWN, break both loops
    if(buffer[0] == SHUTDOWN){
      break;
    }
  }
    // Don't forget to close the sockets!
    close(serverSocket);
    printf("SERVER: Shutting down.\n");

}
