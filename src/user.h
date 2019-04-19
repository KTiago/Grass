#ifndef GRASS_CLIENT_H
#define GRASS_CLIENT_H

#include <cstdint>
#include <string>

using namespace std;

class user{
public:
    explicit user(int socket);
    bool isAuthenticated();
    int getSocket() const;
    bool operator<(const user &other) const;
    pthread_t thread;

private:
    int socket;
    bool authenticated;
};

#endif //GRASS_CLIENT_H
