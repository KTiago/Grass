#ifndef GRASS_CLIENT_H
#define GRASS_CLIENT_H

#include <cstdint>
#include <string>

using namespace std;

class user{
public:
    user(string ip, uint16_t port);
    bool isAuthenticated();
    bool operator<(const user &other) const;

private:
    string ip;
    uint16_t port;
    bool authenticated;
};

#endif //GRASS_CLIENT_H
