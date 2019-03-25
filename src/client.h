#ifndef GRASS_CLIENT_H
#define GRASS_CLIENT_H

#include <cstdint>
#include <string>

using namespace std;

class client{
public:
    client(string ip, uint16_t port);
    bool isAuthenticated();
private:
    string ip;
    uint16_t port;
    bool authenticated;
};

#endif //GRASS_CLIENT_H
