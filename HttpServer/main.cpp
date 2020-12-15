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
#include <regex>
#include <map>
#include "../../MathModel2020/c++/double_pendulum.hpp"

std::string solve(const double* params) {
    int steps = params[0];
    
    std::pair<double, double> mass = {params[1], params[2]};
    std::pair<double, double> length = {params[3], params[4]};
    std::pair<double, double> angle = {params[5], params[6]};
    std::pair<double, double> speed = {params[7], params[8]};
    
    dp::state st{angle, speed};
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
    std::cout << requestBuffer << std::endl;
    std::string request = std::string(requestBuffer);
    static double res[9];
    std::regex regex ("\".{3,7}\":.{1,4}[,}]");
    std::string temp;
    std::map<std::string, std::string> params;

    
    std::regex_token_iterator<std::string::iterator> rend;
    std::regex_token_iterator<std::string::iterator> a (request.begin(), request.end(), regex);
    
    long fIndex;
    long eIndex;
    bool isCommaExists;
    std::string key;
    std::string value;

    while (a!=rend) {
        temp = *a++;
        fIndex = temp.find("\"");
        eIndex = temp.substr(fIndex+1).find("\"");
        key = temp.substr(fIndex+1, eIndex - fIndex);
       
        isCommaExists = temp.find(",") != -1 || temp.find("}") != -1;
        value = temp.substr(eIndex + 3);
        if (isCommaExists)
            value.pop_back();

        params.insert({key, value});
    }
            
    res[0] = std::stod(params["steps"]);
    res[1] = std::stod(params["l_1"]);
    res[2] = std::stod(params["l_2"]);
    res[3] = std::stod(params["m_1"]);
    res[4] = std::stod(params["m_2"]);
    res[5] = std::stod(params["angle_1"]);
    res[6] = std::stod(params["angle_2"]);
    res[7] = std::stod(params["speed_1"]);
    res[8] = std::stod(params["speed_2"]);

    return res;
}


#define PORT 8080
int main(int argc, char const *argv[]) {
//    char buffer[] = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nContent-Length: 98\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 11_0_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36\r\nDNT: 1\r\nContent-Type: text/plain;charset=UTF-8\r\nAccept: */*\r\nOrigin: null\r\nSec-Fetch-Site: cross-site\r\nSec-Fetch-Mode: cors\r\nSec-Fetch-Dest: empty\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9,ru;q=0.8\r\n\r\n{\"steps\":200,\"l_1\":1,\"l_2\":1,\"m_1\":1,\"m_2\":1,\"angle_1\":3.14,\"angle_2\":1.5,\"speed_1\":0,\"speed_2\":0}";
//    parseBody(buffer);
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

