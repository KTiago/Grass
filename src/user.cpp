#include "user.h"

bool user::isAuthenticated() {return this->authenticated;}

user::user(int socket){
    this->socket = socket;
    this->authenticated = false;
    this->uname = "";
}
int user::getSocket() const{
    return this->socket;
}

void  user::setUname(string uname){
    this->uname = uname;
}
void  user::setAuthenticated(bool auth){
    this->authenticated = auth;
}

string user::getUname(){
    return this->uname;
}

bool user::operator<(const user &other) const {this->socket > other.socket; }

