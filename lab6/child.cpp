#include "zmq.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <zconf.h>
#include <vector>
#include <signal.h>
#include <fstream>
#include <algorithm>
#include <map>

// g++ child.cpp -lzmq -o child -w


void send_message(std::string message_string, zmq::socket_t& socket){
    zmq::message_t message_back(message_string.size());
    memcpy(message_back.data(), message_string.c_str(), message_string.size());
    if(!socket.send(message_back)){
        std::cout << "Error: can't send message from node with pid " << getpid() << std::endl;
    }
}


int main(int argc, char * argv[]){
    std::string adr = argv[1];
    zmq::context_t context(1);
    zmq::socket_t main_socket(context, ZMQ_REQ);
    main_socket.connect(argv[1]);
    send_message("OK: " + std::to_string(getpid()), main_socket);
    int id = std::stoi(argv[2]); // id of this node
    std::map<std::string, int> m;
    int left_id = 0;
    int right_id = 0;
    zmq::context_t context_l(1);
    zmq::context_t context_r(1);
    zmq::socket_t left_socket(context_l, ZMQ_REP);
    std::string adr_left = "tcp://127.0.0.1:300";
    zmq::socket_t right_socket(context_r, ZMQ_REP);
    std::string adr_right = "tcp://127.0.0.1:300";
    while(1){
        zmq::message_t message_main;
        main_socket.recv(&message_main);
        std::string recieved_message(static_cast<char*>(message_main.data()), message_main.size());
        std::string command;
        for(int i = 0; i < recieved_message.size(); ++i){
            if(recieved_message[i] != ' '){
                command += recieved_message[i];
            } else {
                break;
            }
        }
        if(command == "exec"){
            int id_proc; // id of node for adding
            std::string id_proc_;
            std::string big, small, for_return;
            int flag = 0;
            std::vector<int> answers;
            for(int i = 5; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    break;
                }
            }
            id_proc = std::stoi(id_proc_);
            if(id_proc == id) { // id == proc_id
                for(int i = 6 + id_proc_.size(); i < recieved_message.size(); ++i){
                    if (recieved_message[i] == ' ')
                        ++flag;
                    if ((recieved_message[i] != ' ') && (flag == 0)) {
                        big += recieved_message[i];
                    } else if ((recieved_message[i] != ' ') && (flag == 1)){
                        small += recieved_message[i];
                    }
                }
                if (big.size() >= small.size()) {
                    int start = 0;
                    while(big.find(small, start) != -1){
                        start = big.find(small, start);
                        answers.push_back(start);
                        ++start;
                    }
                }
                if(answers.size() == 0){
                    for_return = "-1";
                }else{
                    for_return = std::to_string(answers[0]);
                    for(int i = 1; i < answers.size();++i) {
                        for_return = for_return + ";" + std::to_string(answers[i]);
                    }
                }
                for_return = "OK:" + id_proc_ + ":" + for_return;
                send_message(for_return, main_socket);
            } else {
                if(id > id_proc){
                    if(left_id == 0){ // if node not exists
                        std::string message_string = "Error:id: Not found";
                        send_message("Error:id: Not found", main_socket);
                    } else {
                        zmq::message_t message(recieved_message.size());
                        memcpy(message.data(), recieved_message.c_str(), recieved_message.size());
                        if(!left_socket.send(message)){
                            std::cout << "Error: can't send message to left node from node with pid: " << getpid() << std::endl;
                        }
                        // catch and send to parent
                        if(!left_socket.recv(&message)){
                            std::cout << "Error: can't receive message from left node in node with pid: " << getpid() << std::endl;
                        }
                        if(!main_socket.send(message)){
                            std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                        }
                    }
                } else {
                    if(right_id == 0){ // if node not exists
                        std::string message_string = "Error:id: Not found";
                        zmq::message_t message(message_string.size());
                        memcpy(message.data(), message_string.c_str(), message_string.size());
                        if(!main_socket.send(message)){
                            std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                        }
                    } else {
                        zmq::message_t message(recieved_message.size());
                        memcpy(message.data(), recieved_message.c_str(), recieved_message.size());
                        if(!right_socket.send(message)){
                            std::cout << "Error: can't send message to right node from node with pid: " << getpid() << std::endl;
                        }
                        // catch and send to parent
                        if(!right_socket.recv(&message)){
                            std::cout << "Error: can't receive message from left node in node with pid: " << getpid() << std::endl;
                        }
                        if(!main_socket.send(message)){
                            std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                        }
                    }
                }
            }
        } else if(command == "create"){
            int id_proc; // id of node for creating
            std::string id_proc_;
            for(int i = 7; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    break;
                }
            }
            id_proc = std::stoi(id_proc_);
            if(id_proc == id){
                send_message("Error: Already exists", main_socket);
            } else if(id_proc > id){
                if(right_id == 0){ // there is not right node
                    right_id = id_proc;
                    int right_id_tmp = right_id - 1;
                    right_socket.bind(adr_right + std::to_string(++right_id_tmp));
                    adr_right += std::to_string(right_id_tmp);
                    char* adr_right_ = new char[adr_right.size() + 1];
                    memcpy(adr_right_, adr_right.c_str(), adr_right.size() + 1);
                    char* right_id_ = new char[std::to_string(right_id).size() + 1];
                    memcpy(right_id_, std::to_string(right_id).c_str(), std::to_string(right_id).size() + 1);
                    char* args[] = {"./child", adr_right_, right_id_, NULL};
                    int f = fork();
                    if(f == 0){
                        execv("./child", args);
                    } else if (f == -1){
                        std::cout << "Error in forking in node with pid: " << getpid() << std::endl;
                    } else {
                        // catch message from new node
                        zmq::message_t message_from_node;
                        if(!right_socket.recv(&message_from_node)){
                            std::cout << "Error: can't receive message from right node in node with pid:" << getpid() << std::endl;
                        }
                        std::string recieved_message_from_node(static_cast<char*>(message_from_node.data()), message_from_node.size());
                        // send message to main node
                        if(!main_socket.send(message_from_node)){
                            std::cout << "Error: can't send message to main node from node with pid:" << getpid() << std::endl;
                        }
                    }
                    delete [] adr_right_;
                    delete [] right_id_;
                } else { // send task to right node
                    send_message(recieved_message, right_socket);
                    // catch and send to parent
                    zmq::message_t message;
                    if(!right_socket.recv(&message)){
                        std::cout << "Error: can't receive message from left node in node with pid: " << getpid() << std::endl;
                    }
                    if(!main_socket.send(message)){
                        std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                    }
                }
            } else {
                if(left_id == 0){ // there is not left node
                    left_id = id_proc;
                    int left_id_tmp = left_id - 1;
                    left_socket.bind(adr_left + std::to_string(++left_id_tmp));
                    adr_left += std::to_string(left_id_tmp);
                    char* adr_left_ = new char[adr_left.size() + 1];
                    memcpy(adr_left_, adr_left.c_str(), adr_left.size() + 1);
                    char* left_id_ = new char[std::to_string(left_id).size() + 1];
                    memcpy(left_id_, std::to_string(left_id).c_str(), std::to_string(left_id).size() + 1);
                    char* args[] = {"./child", adr_left_, left_id_, NULL};
                    int f = fork();
                    if(f == 0){
                        execv("./child", args);
                    } else if(f == -1){
                        std::cout << "Error in forking in node with pid: " << getpid() << std::endl;
                    } else {
                        // catch message from new node
                        zmq::message_t message_from_node;
                        if(!left_socket.recv(&message_from_node)){
                            std::cout << "Error: can't receive message from left node in node with pid:" << getpid() << std::endl;
                        }
                        std::string recieved_message_from_node(static_cast<char*>(message_from_node.data()), message_from_node.size());
                        // send message to main node
                        if(!main_socket.send(message_from_node)){
                            std::cout << "Error: can't send message to main node from node with pid:" << getpid() << std::endl;
                        }
                    }
                    delete [] adr_left_;
                    delete [] left_id_;
                } else { // send task to left node
                    send_message(recieved_message, left_socket);
                    // catch and send to parent
                    zmq::message_t message;
                    if(!left_socket.recv(&message)){
                        std::cout << "Error: can't receive message from left node in node with pid: " << getpid() << std::endl;
                    }
                    if(!main_socket.send(message)){
                        std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                    }
                }
            }
        } else if(command == "ping") {
            int id_proc; // id of node for creating
            std::string id_proc_;
            for(int i = 5; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    break;
                }
            }
            id_proc = std::stoi(id_proc_);
            if(id_proc == id){
                send_message("OK: 1", main_socket);
            } else if(id_proc < id) {
                if(left_id == 0){
                    send_message("OK: 0", main_socket);
                } else {
                    left_socket.send(message_main);
                    zmq::message_t answ;
                    left_socket.recv(&answ);
                    main_socket.send(answ);
                }
            } else if(id_proc > id) {
                if(right_id == 0){
                    send_message("OK: 0", main_socket);
                } else {
                    right_socket.send(message_main);
                    zmq::message_t answ;
                    right_socket.recv(&answ);
                    main_socket.send(answ);
                }
            }
        } else if(command == "kill") {
            int id_proc; // id of node for killing
            std::string id_proc_;
            for(int i = 5; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    break;
                }
            }
            id_proc = std::stoi(id_proc_);
            if(id_proc > id){
                if(right_id == 0){
                    send_message("Error: there isn`t node with this id", main_socket);
                } else {
                    if(right_id == id_proc){
                        send_message("Ok: " + std::to_string(right_id), main_socket);
                        send_message("DIE", right_socket);
                        right_socket.unbind(adr_right);
                        adr_right = "tcp://127.0.0.1:300";
                        right_id = 0;
                    } else {
                        right_socket.send(message_main);
                        zmq::message_t message;
                        right_socket.recv(&message);
                        main_socket.send(message);
                    }
                }
            } else if(id_proc < id){
                if(left_id == 0){
                    send_message("Error: there isn`t node with this id", main_socket);
                } else {
                    if(left_id == id_proc){
                        send_message("Ok: " + std::to_string(left_id), main_socket);
                        send_message("DIE", left_socket);
                        left_socket.unbind(adr_left);
                        adr_left = "tcp://127.0.0.1:300";
                        left_id = 0;
                    } else {
                        left_socket.send(message_main);
                        zmq::message_t message;
                        left_socket.recv(&message);
                        main_socket.send(message);
                    }
                }
            }
        } else if (command == "DIE") {
            if (left_id){
                send_message("DIE", left_socket);
                left_socket.unbind(adr_left);
                adr_left = "tcp://127.0.0.1:300";
                left_id = 0;
            }
            if (right_id){
                send_message("DIE", right_socket);
                right_socket.unbind(adr_right);
                adr_right = "tcp://127.0.0.1:300";
                right_id = 0;
            }
            main_socket.unbind(adr);
            return 0;
        }
    }
}