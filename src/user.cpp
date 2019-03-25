#include "user.h"

bool user::isAuthenticated() {return this->authenticated;}

user::user(int socket){
    this->socket = socket;
    this->authenticated = false;
}

//FIXME
bool user::operator<(const user &other) const {this->socket > other.socket; }

