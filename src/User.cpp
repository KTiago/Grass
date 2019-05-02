#include "User.h"

bool User::isAuthenticated() { return this->authenticated; }

User::User(int socket, string ip, string base) {
    this->socket = socket;
    this->authenticated = false;
    this->uname = "";
    this->ip = ip;
    this->location = base;
    this->putThread = NULL;
    this->getThread = NULL;
}

int User::getSocket() const {
    return this->socket;
}

void User::setUname(const string &uname) {
    this->uname = uname;
}

void User::resetUname() {
    this->setUname("");
}

void User::setLocation(const string &location) {
    this->location = location;
}

void User::setAuthenticated(bool auth) {
    this->authenticated = auth;
}

string User::getUname() const {
    return this->uname;
}

string User::getLocation() {
    return this->location;
}

string User::getIp() {
    return this->ip;
}

bool User::operator<(const User &other) const { this->socket > other.socket; }

