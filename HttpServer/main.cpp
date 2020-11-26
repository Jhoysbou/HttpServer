//
//  main.cpp
//  HttpServer
//
//  Created by Artem Bakuta on 25.11.2020.
//

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <regex>
#include <algorithm>
#include <iostream>
#include <string>
#include "double_pendulum.hpp"

std::string solve(const double* params) {
    int steps = params[0];
    
    std::pair<double, double> mass = {params[1], params[2]};
    std::pair<double, double> length = {params[3], params[4]};
    
    dp::state st{{3.14, 1.5}, {0, 0}};
    dp::system ss{mass, length};

    int k = 0;
    std::string result = "";
    
    while (k < steps){
       k++;
       result += std::to_string(st.theta.first) + ";" + std::to_string(st.theta.second) + "\n";
       st = dp::advance(st, ss, 0.2);
    }
    return result;
}

std::string response(std::string data) {
    std::string headers = "HTTP/1.1 200 OK\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\nContent-Length: ";
    long length = data.length();
    std::string result = headers + std::to_string(length) + "\n\n";
    result += data;
    return result;
}

double* parseBody(char requestBuffer[]) {
    std::string request = std::string(requestBuffer);
    
    long index = request.find("\r\n\r\n");
    std::string requestBody = request.substr(index + 8);

    
    index = requestBody.find("steps");
    long commaIndex = requestBody.find("\r\n");
    std::string steps = requestBody.substr(index + 8, commaIndex - index - 9);
    requestBody = requestBody.substr(commaIndex + 1);
    
    index = requestBody.find("l_1");
    commaIndex = requestBody.find("\r\n");
    std::string l_1 = requestBody.substr(index + 6, commaIndex - index - 7);
    requestBody = requestBody.substr(commaIndex + 1);
 
    index = requestBody.find("l_2");
    commaIndex = requestBody.find("\r\n");
    std::string l_2 = requestBody.substr(index + 6, commaIndex - index - 7);
    requestBody = requestBody.substr(commaIndex + 1);
    
    index = requestBody.find("m_1");
    commaIndex = requestBody.find("\r\n");
    std::string m_1 = requestBody.substr(index + 6, commaIndex - index - 7);
    requestBody = requestBody.substr(commaIndex + 1);
    
    index = requestBody.find("m_2");
    commaIndex = requestBody.find("\r\n");
    std::string m_2 = requestBody.substr(index + 6, commaIndex - index - 6);
    

    static double res[5];
    res[0] = std::stod(steps);
    res[1] = std::stod(l_1);
    res[2] = std::stod(l_2);
    res[3] = std::stod(m_1);
    res[4] = std::stod(m_2);

    return res;
}


#define PORT 8080
int main(int argc, char const *argv[]) {
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);



    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    memset(address.sin_zero, '\0', sizeof address.sin_zero);


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1) {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
       

        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        double* params = parseBody(buffer);
        std::string simData = solve(params);
        std::string response = ::response(simData);
//        printf("%s\n",buffer );
        write(new_socket , response.c_str(), response.length());
        printf("------------------ Data sent -------------------\n");
        close(new_socket);
    }
    return 0;
}

