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
#include <fstream>
#include <string>
#include <regex>
#include <map>
#include "../MathModel2020/c++/double_pendulum.hpp"

std::string solve(std::map<std::string, double> paramMap) {
    int steps = paramMap["steps"];
    
    std::pair<double, double> mass = {paramMap["m_1"], paramMap["m_2"]};
    std::pair<double, double> length = {paramMap["l_1"], paramMap["l_2"]};
    std::pair<double, double> angle = {paramMap["angle_1"], paramMap["angle_2"]};
    std::pair<double, double> speed = {paramMap["speed_1"], paramMap["speed_2"]};
        
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

std::string response(std::string data, std::string contentType) {
    std::string headers = "HTTP/1.1 200 OK\nContent-Type: " + contentType + "\nAccess-Control-Allow-Origin: *\nContent-Length: ";
    long length = data.length();
    std::string result = headers + std::to_string(length) + "\n\n";
    result += data;
    return result;
}

std::map<std::string, double> parseBody(char requestBuffer[]) {
    std::cout << requestBuffer << std::endl;
    std::string request = std::string(requestBuffer);
    std::regex regex ("\".{3,7}\":.{1,4}[,}]");
    std::string temp;
    std::map<std::string, double> params;

    
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
        params.insert({key, std::stod(value)});
    }

    return params;
}

std::string readHtml() {
    std::ifstream file("MathModel2020/model/index.html", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    static std::vector<char> buffer(size);
    if (file.read(buffer.data(), size))
    {
        return &buffer[0];
    }
    return "Internal error! Cannot read html file";
}

std::string handleRoutes(char buffer[]) {
    std::string request = std::string(buffer);
    std::smatch m;
    std::regex regexGet ("GET / .*");
    std::regex regexPost ("POST / .*");
    std::string response = "404";
    
    if (std::regex_search(request, m, regexGet)) {
        response = ::response(readHtml(), "text/html");
    } else if (std::regex_search(request, m, regexPost)) {
        std::map<std::string, double> params = parseBody(buffer);
        std::string simData = solve(params);
        response = ::response(simData, "text/plain");
    }
    
    return response;
}


#define PORT 8085
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
        valread = read(new_socket , buffer, 30000);
        std::string response = handleRoutes(buffer);
//        printf("%s\n",buffer );
        write(new_socket , response.c_str(), response.length());
        printf("------------------ Data sent -------------------\n");
        close(new_socket);
    }
    return 0;
}

