#include "user.h"

bool user::isAuthenticated() {return this->authenticated;}

user::user(int socket, string base){
    this->socket = socket;
    this->authenticated = false;
    this->uname = "";
    this->location = base;
}
int user::getSocket() const{
    return this->socket;
}

void  user::setUname(string uname){
    this->uname = uname;
}

void  user::resetUname(){
    this->setUname("");
}

void  user::setLocation(string location){
    this->location = location;
}

void  user::setAuthenticated(bool auth){
    this->authenticated = auth;
}

string user::getUname(){
    return this->uname;
}

string user::getLocation(){
    return this->location;
}

bool user::operator<(const user &other) const {this->socket > other.socket; }

