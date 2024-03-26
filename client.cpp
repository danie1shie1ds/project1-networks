#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

int
main(int argc, char **argv)
{
  // validate number of CL args
  if (argc != 4){
    std::cerr<<"ERROR: invalid number of arguments\n";
    return EXIT_FAILURE;
  }

  // check if port number is valid
  int port_num = std::stoi(argv[2]);
  if (port_num < 1024 || port_num > 65535){
    std::cerr<<"ERROR: invalid port number\n";
    return EXIT_FAILURE;
  }

  // check if hostname is valid
  char *hostname = argv[1];
  if(strcmp(hostname, "localhost") == 0){
    std::strcpy(hostname, "127.0.0.1");
  }
  struct in_addr addr;
  if (inet_pton(AF_INET, hostname, &addr) != 1){
    std::cout<< hostname << "\n";
    std::cerr<<"ERROR: invalid hostname or IP address\n";
    return EXIT_FAILURE;
  }

  std::ifstream myfile;
  myfile.open(argv[3]);

    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_num);  // open a socket on port 4000 of the server
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]); // use localhost as the IP address of the server to set up the socket
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
      perror("connect");
      return 2;
    }

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
      perror("getsockname");
      return 3;
    }

    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Set up a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << std::endl;


    // send/receive data 
    std::string input;
    char buf[1024] = {0};
    std::stringstream ss;

    if (!(myfile.is_open())){
        std::cerr<< "ERROR: couldn't open file\n";
        return EXIT_FAILURE;
      }

    while (!myfile.eof()) {
      myfile.read(buf, sizeof(buf) -1);
      buf[myfile.gcount()] = '\0';
      // std::cout << "send: ";
      

      if (send(sockfd, buf, myfile.gcount(), 0) == -1) {
        perror("send");
        return 4;
      }
    }
    close(sockfd);

    return 0;

}