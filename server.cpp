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

int
main(int argc, char **argv)
{
  // check to see if command line arguments are valid
  // need a port number and directory for received files

  int num_connections = 0;

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

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  // accept a new connection from a client
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;



  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/1.file", path);
  std::ofstream myfile;
  myfile.open(filepath);
  
  if (!(myfile.is_open())){
    std::cerr<< "ERROR: couldn't open file\n";
    return EXIT_FAILURE;
  }

  size_t n;
  char buffer[1024] = {0};
  while((n = recv(clientSockfd, buffer, sizeof(buffer), 0)) > 0){
    myfile.write(buffer, n);
  }
    


  // receive/send data (1 message) from/to the client
  bool isEnd = false;
  char buf[20] = {0};
  std::stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    if (recv(clientSockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }

    ss << buf << std::endl;
    std::cout << buf << std::endl;

    if (send(clientSockfd, buf, 20, 0) == -1) {
      perror("send");
      return 6;
    }

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

close(clientSockfd);

return 0;

}