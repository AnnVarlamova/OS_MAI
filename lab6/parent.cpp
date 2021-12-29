#include "zmq.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <zconf.h>
#include <vector>
#include <signal.h>
#include <sstream>
#include <set>
#include <algorithm>

// g++ parent.cpp -lzmq -o main -w


int main(){
    zmq::context_t context(1); // служебная структура контекст
    zmq::socket_t main_socket(context, ZMQ_REP); // поднятие сокета в контексте ZMQ_REP для отправки запросов и получения ответов
    std::string adr = "tcp://127.0.0.1:300";
    std::string command;
    int child_id = 0;
    while(1){
        std::cout << "command:";
        std::cin >> command;
        if(command == "create") {
            if(child_id == 0){
                int id;
                std::cin >> id;
                int id_tmp = id - 1;
                main_socket.bind(adr + std::to_string(++id_tmp));
                std::string new_adr = adr + std::to_string(id_tmp);
                char* adr_ = new char[new_adr.size() + 1];
                memcpy(adr_, new_adr.c_str(), new_adr.size() + 1);
                char* id_ = new char[std::to_string(id).size() + 1];
                memcpy(id_, std::to_string(id).c_str(), std::to_string(id).size() + 1);
                char* args[] = {"./child", adr_, id_, NULL};
                int id2 = fork();
                if (id2 == -1) {
                    std::cout << "Unable to create first worker node" << std::endl;
                    id = 0;
                    exit(1);
                } else if(id2 == 0){
                    execv("./child", args);
                } else {
                    child_id = id;
                }
                zmq::message_t message;
                main_socket.recv(&message);
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                std::cout << recieved_message << std::endl;
                delete [] adr_;
                delete [] id_;
            } else {
                int id;
                std::cin >> id;
                std::string message_string = command + " " + std::to_string(id);
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                main_socket.send(message);

                // catch message from new node
                main_socket.recv(&message);
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                std::cout << recieved_message << std::endl;
            }
        } else if (command == "exec") {
            int id;
            std::string big, small;
            std::cin >> id;
            std::cin >> big >> small;
            std::string message_string = command + " " + std::to_string(id) + " " + big + " " + small;
            zmq::message_t message(message_string.size());
            memcpy(message.data(), message_string.c_str(), message_string.size());
            main_socket.send(message);
            // return value from map
            main_socket.recv(&message);
            std::string recieved_message(static_cast<char*>(message.data()), message.size());
            std::cout << recieved_message << std::endl;
        } else if (command == "ping") {
            int id;
            std::cin >> id;
            std::string message_string = command + " " + std::to_string(id);
            zmq::message_t message(message_string.size());
            memcpy(message.data(), message_string.c_str(), message_string.size());
            main_socket.send(message);
            // receive answer from child
            main_socket.recv(&message);
            std::string recieved_message(static_cast<char*>(message.data()), message.size());
            std::cout << recieved_message << std::endl;
        } else if(command == "kill"){
            int id;
            std::cin >> id;
            if(child_id == 0){
                std::cout << "Error: there isn't nodes" << std::endl;
            } else if(child_id == id){
                std::string kill_message = command + " " + std::to_string(id);
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                main_socket.send(message);
                main_socket.recv(&message);
                std::string received_message(static_cast<char*>(message.data()), message.size());
                std::cout << received_message << std::endl;
                std::cout << "Tree deleted successfully" << std::endl;
                return 0;
            } else {
                std::string kill_message = command + " " + std::to_string(id);
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                main_socket.send(message);
                main_socket.recv(&message);
                std::string received_message(static_cast<char*>(message.data()), message.size());
                std::cout << received_message << std::endl;
            }
        } else if(command == "exit"){
            if(child_id){
                std::string kill_message = "DIE";
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                main_socket.send(message);
                std::cout << "Tree was deleted" << std::endl;
            }
            main_socket.close();
            context.close();
            break;
        } else {
            std::cout << "Error: incorrect command\n";
        }
    }
}
