#ifndef GRASS_CLIENT_H
#define GRASS_CLIENT_H

#include <cstdint>
#include <string>

using namespace std;

class user{
public:
    explicit user(int socket, string base);
    bool isAuthenticated();
    int getSocket() const;
    string getUname();
    string getLocation();
    void  setUname(string uname);
    void  resetUname();
    void  setLocation(string location);
    void  setAuthenticated(bool auth);
    bool operator<(const user &other) const;

private:
    int socket;
    bool authenticated;
    string uname;
    string location;
};

#endif //GRASS_CLIENT_H
