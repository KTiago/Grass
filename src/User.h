#ifndef GRASS_CLIENT_H
#define GRASS_CLIENT_H

#include <cstdint>
#include <string>

using namespace std;

class User{
public:
    explicit User(int socket, string base);
    bool isAuthenticated();
    int getSocket() const;
    string getUname() const;
    string getLocation();
    void  setUname(const string& uname);
    void  resetUname();
    void  setLocation(const string& location);
    void  setAuthenticated(bool auth);
    bool operator<(const User &other) const;

private:
    int socket;
    bool authenticated;
    string uname;
    string location;
};

#endif //GRASS_CLIENT_H
