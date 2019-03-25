#include "user.h"

bool user::isAuthenticated() {return this->authenticated;}

user::user(string ip, uint16_t port){
    this->ip = ip;
    this->port = port;
    this->authenticated = false;
}

//FIXME
bool user::operator<(const user &other) const {this->port > this->port; }

