#include "user.h"

bool user::isAuthenticated() {return this->authenticated;}

user::user(int socket){
    this->socket = socket;
    this->authenticated = false;
    this->thread = NULL;
}
int user::getSocket() const{
    return this->socket;
}
bool user::operator<(const user &other) const {this->socket > other.socket; }

