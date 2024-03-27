// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <string.h>
// #include <stdio.h>
// #include <errno.h>
// #include <unistd.h>
// #include <time.h>

// #include <iostream>
// #include <sstream>
// #include <fstream>
// #include <cstring>

// int
// main(int argc, char **argv)
// {
//   // validate number of CL args
//   if (argc != 4){
//     std::cerr<<"ERROR: invalid number of arguments\n";
//     return EXIT_FAILURE;
//   }

//   // check if port number is valid
//   int port_num = std::stoi(argv[2]);
//   if (port_num < 1024 || port_num > 65535){
//     std::cerr<<"ERROR: invalid port number\n";
//     return EXIT_FAILURE;
//   }

//   // check if hostname is valid
//   char *hostname = argv[1];
//   if(strcmp(hostname, "localhost") == 0){
//     std::strcpy(hostname, "127.0.0.1");
//   }
//   struct in_addr addr;
//   if (inet_pton(AF_INET, hostname, &addr) != 1){
//     std::cout<< hostname << "\n";
//     std::cerr<<"ERROR: invalid hostname or IP address\n";
//     return EXIT_FAILURE;
//   }

//   std::ifstream myfile;
//   myfile.open(argv[3]);

//     // create a socket using TCP IP
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     // // If the socket is waiting more than 10 seconds to connect
//     // size_t timeout_in_seconds = 10;
//     // struct timeval tv;
//     // tv.tv_sec = timeout_in_seconds;
//     // tv.tv_usec = 0;
//     // setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port_num);  // open a socket on port 4000 of the server
//     serverAddr.sin_addr.s_addr = inet_addr(argv[1]); // use localhost as the IP address of the server to set up the socket
//     memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

//     // connect to the server
//     fd_set writefds;
//     FD_ZERO(&writefds);
//     FD_SET(sockfd, &writefds);

//     struct timeval tv;
//     tv.tv_sec = 10;
//     tv.tv_usec = 0;
//     int rv = select(sockfd+1, NULL, &writefds, NULL, &tv);

//     if (rv == 0) {
// 	    fprintf(stderr, "ERROR: Timeout Occured.");
//       return -1;
//     } else if (rv == -1) {
//       perror("select");
//     }

//     // connect to the server
//     if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
//       fprintf(stderr, "ERROR: Couldn't connect to server");
//       perror("connect");
//       return 2;
//     }

//     struct sockaddr_in clientAddr;
//     socklen_t clientAddrLen = sizeof(clientAddr);
//     if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
//       perror("getsockname");
//       return 3;
//     }
    

//     char ipstr[INET_ADDRSTRLEN] = {'\0'};
//     inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
//     std::cout << "Set up a connection from: " << ipstr << ":" <<
//       ntohs(clientAddr.sin_port) << std::endl;


//     // send/receive data 
//     std::string input;
//     char buf[1024] = {0};
//     std::stringstream ss;

//     if (!(myfile.is_open())){
//         std::cerr<< "ERROR: couldn't open file\n";
//         return EXIT_FAILURE;
//       }

//     while (!myfile.eof()) {
//       myfile.read(buf, sizeof(buf) -1);
//       buf[myfile.gcount()] = '\0';
//       // std::cout << "send: ";
      

//       if (send(sockfd, buf, myfile.gcount(), 0) == -1) {
//         perror("send");
//         return 4;
//       }
//     }
//     close(sockfd);

//     return 0;

// }



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <fstream>


void handle(int signum){
  exit(0);
}

int main(int argc, char **argv)
{
  // check to see if command line arguments are valid
  // need a port number and directory for received files


  if (argc != 3){
    std::cerr<<"ERROR: invalid number of arguments\n";
    return EXIT_FAILURE;
  }
  int port_num = std::stoi(argv[1]);
  if (port_num < 1024 || port_num > 65535){
    std::cerr<<"ERROR: invalid port number\n";
    return EXIT_FAILURE;
  }

  char* path = argv[2];
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  signal(SIGTERM, handle);
  signal(SIGQUIT, handle);


  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_num); // the server will listen on port 4000
  addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // open socket on localhost IP address for server
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));


  // bind socket fd to hostname and port number
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  int num_connections = 0;
  // timeval structure to set timeout value
  size_t timeout_in_seconds = 10;
  struct timeval tv;
  tv.tv_sec = timeout_in_seconds;
  tv.tv_usec = 0;
  while (true){
    // accept a new connection from a client
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSockfd == -1) {
      perror("accept");
      return 4;
    }
    // successful connection, increase the counter
    num_connections++;

    if (setsockopt(clientSockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1){
      perror("ERROR: recv timeout");
      close(clientSockfd);
      continue;
    }
    

    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Accept a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << std::endl;



    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%d.file", path, num_connections);
    // printf("%s\n", filepath);
    std::ofstream myfile;
    myfile.open(filepath);
    
    if (!(myfile.is_open())){
      std::cerr<< "ERROR: couldn't open file\n";
      return EXIT_FAILURE;
    }

    size_t n;
    char buffer[1024] = {0};
    while((n = recv(clientSockfd, buffer, 1024, 0)) > 0){
      myfile.write(buffer, n);
      
      // fwrite(buffer, 1, n, myfile);
      
      // memset(buffer, 0, 1024);
      
      // if (end - begin > 10) {
      //   fwrite(error, 1, err_size, myfile);
      //   break;
      // }
    }
    if (n < 0){
      myfile << "ERROR";
      myfile.close();
      close(clientSockfd);

    }
    myfile.close();

    close(clientSockfd);
  }
  

return 0;

}