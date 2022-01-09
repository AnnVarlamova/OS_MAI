#include <iostream>
#include <map>
#include "zmq.hpp"
#include <vector>
#include <cstring>
#include <memory>
#include <thread>
#include <tuple>
#include <zconf.h>

// g++ server.cpp -lzmq -pthread -o server -w

//хранение логинов и сокетов PULL (client) - PUSH (server)
std::map<std::string, std::shared_ptr<zmq::socket_t>> ports;

//сейчас онлайн
std::map<std::string, bool> logged_in;

//когда-либо заходили на сервер
std::map<std::string, bool> registered;

//вектор кортежей из получателя, отправителя, сообщения
std::vector<std::tuple<std::string, std::string, std::string, bool>> tuples;

zmq::context_t context1(1);

void send_message(std::string message_string, zmq::socket_t &socket)

{
    zmq::message_t message_back(message_string.size());
    memcpy(message_back.data(), message_string.c_str(), message_string.size());
    if (!socket.send(message_back))
    {
        std::cout << "Error" << std::endl;
    }
}

void kill_me()
{
    std::string word;
    while (std::cin >> word)
    {
        if (word == "exit") {
            _exit(0);
        } else {
            std::cout << "Enter exit to stop server" << std::endl;
        }
    }
}

std::string receive_message(zmq::socket_t& socket) {
    zmq::message_t message_main;
    socket.recv(&message_main);
    std::string answer(static_cast<char*>(message_main.data()), message_main.size());
    return answer;
}

void process_client(int id)
{
    zmq::context_t context2(1);
    zmq::socket_t puller(context2, ZMQ_PULL);
    puller.bind("tcp://*:3" + std::to_string(id + 1));
    bool alive = true;
    while (alive)
    {
        std::string command = "";
        std::string client_mes = receive_message(puller);
        for (char i : client_mes) {
            if (i != ' ') {
                command += i;
            } else {
                break;
            }
        }
        int i;
        if (command == "send") {
            std::string recipient = "";
            for(i = 5; i < client_mes.size(); ++i){
                if(client_mes[i] != ' '){
                    recipient += client_mes[i];
                } else{
                    break;
                }
            }
            ++i;
            std::string sender = "";
            for(i; i < client_mes.size(); ++i){
                if(client_mes[i] != ' '){
                    sender += client_mes[i];
                } else {
                    break;
                }
            }
            ++i;
            std::string user_mes;
            for(i; i < client_mes.size(); ++i){
                user_mes += client_mes[i];
            }
            if(logged_in[recipient]) {
                tuples.push_back(std::make_tuple(recipient, sender, user_mes, 1));
                send_message(client_mes, *ports[recipient]);
                std::cout << sender << " sent message to " << recipient << std::endl;
            } else if (registered[recipient]) {
                tuples.push_back(std::make_tuple(recipient, sender, user_mes, 0));
                send_message("out of server", *ports[sender]);
                std::cout << sender << " sent message to offline " << recipient << std::endl;
            } else {
                send_message("no client", *ports[sender]);
                std::cout << sender << "tried to sent message to unknown " << recipient << std::endl;
            }
        } else if (command == "exit") {
            std::string sender = "";
            for(i = 5; i < client_mes.size(); ++i){
                if(client_mes[i] != ' '){
                    sender += client_mes[i];
                } else{
                    break;
                }
            }
            send_message("exit ", *ports[sender]);
            logged_in[sender] = false;
            alive = false;
        } else if (command == "dialog") {
            std::string us1 = "", us2 = "";
            int i = 7, count_mess = 0;
            for(i; i < client_mes.size(); ++i){
                if(client_mes[i] != ' '){
                    us1 += client_mes[i];
                } else {
                    break;
                }
            }
            ++i;
            for(i; i < client_mes.size(); ++i){
                if(client_mes[i] != ' '){
                    us2 += client_mes[i];
                } else {
                    break;
                }
            }
            if (registered[us2]) {
                std::cout << us1 << " asked history with " << us2 << std::endl;
                for (auto&& tuple: tuples) {
                    std::string rec, sen, mes;
                    bool read;
                    std::tie(rec, sen, mes, read) = tuple;
                    if (((rec == us1) && (sen == us2)) || ((rec == us2) && (sen == us1))) {
                        send_message("dialog " + rec + " " + sen + " " + mes, *ports[us1]);
                        ++count_mess;
                    }
                }
                if (count_mess == 0)
                    send_message("zero", *ports[us1]);
            } else {
                std::cout << us1 << " asked history with unknown " << us2 << std::endl;
                send_message("no client", *ports[us1]);
            }
        } else if (command == "find") {
            std::string sender = "", str = "";
            int i = 5, count_str = 0;
            for(i; i < client_mes.size(); ++i){
                if(client_mes[i] != ' '){
                    sender += client_mes[i];
                } else {
                    break;
                }
            }
            ++i;
            for(i; i < client_mes.size(); ++i){
                str += client_mes[i];
            }
            std::cout << sender << " finds " << str << std::endl;
            for (auto&& tuple: tuples) {
                std::string rec, sen, mes;
                bool read;
                std::tie(rec, sen, mes, read) = tuple;
                std::size_t found = mes.find(str);
                if (((rec == sender) || (sen == sender)) && (found!=std::string::npos)) {
                    send_message("find " + rec + " " + sen + " " + mes, *ports[sender]);
                    ++count_str;
                }
            }
            if (count_str == 0)
                send_message("empt y", *ports[sender]);
        }
    }
}

int main(){
    zmq::context_t context(1);
    zmq::socket_t socket_for_login(context, ZMQ_REP);

    socket_for_login.bind("tcp://*:4042");

    int uzers_number = 0;

    std::thread to_die = std::thread(kill_me);
    to_die.detach();

    while (1) {
        std::string recieved_message = receive_message(socket_for_login);
        std::string id_s = "";
        int i;
        for(i = 0; i < recieved_message.size(); ++i){
            if(recieved_message[i] != ' '){
                id_s += recieved_message[i];
            } else{
                break;
            }
        }
        int id = std::stoi(id_s);
        std::string nickname;
        ++i;
        for(i; i < recieved_message.size(); ++i){
            if(recieved_message[i] != ' '){
                nickname += recieved_message[i];
            } else{
                break;
            }
        }
        if(logged_in[nickname] == true) {
            std::cout << "This user already logged in..." << std::endl;
            send_message("0", socket_for_login);
        }
        else{
            std::cout << "User " << nickname << " logged in with id " << id << std::endl;
            send_message("1", socket_for_login);

            std::shared_ptr<zmq::socket_t> socket_client = std::make_shared<zmq::socket_t>(context1, ZMQ_PUSH);
            socket_client->bind("tcp://*:3" + id_s);
            ports[nickname] = socket_client;
            //
            if (registered[nickname]) {
                for (auto&& tuple: tuples) {
                    std::string rec, sen, mes;
                    bool read;
                    std::tie(rec, sen, mes, read) = tuple;
                    if ((rec == nickname) && (read == false)) {
                        send_message("dialog " + rec + " " + sen + " " + mes, *socket_client);
                        read = true;
                    }
                }
            }
            //
            std::thread worker = std::thread(std::ref(process_client), id);
            worker.detach();
            logged_in[nickname] = true;
            if(!registered[nickname]) {
                registered[nickname] = true;
            }
        }
    }
}
