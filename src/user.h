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
    string getUname();
    void  setUname(string uname);
    void  setAuthenticated(bool auth);
    bool operator<(const user &other) const;

private:
    int socket;
    bool authenticated;
    string uname;
};

#endif //GRASS_CLIENT_H
