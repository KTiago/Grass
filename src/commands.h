#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include "User.h"
#include "grass.hpp"

using namespace std;

extern const string ACCESS_ERROR;
extern const string FILENAME_ERROR;
extern const string TRANFER_ERROR;
extern const int MAXIMAL_PATH_LEN;


int mkdir_cmd(string dir, User usr, string &out);
int cd_cmd(string dir, User &usr, string &out);
int ls_cmd(bool authenticated, string &out, string usrLocation);
int get_cmd(string fileName, int getPort, User &usr, string &out);
int login_cmd(string uname, map<string, string> allowedUsers, User &usr, string &out);
int logout_cmd(User &usr, string &out);
int pass_cmd(string psw, map<string, string> allowedUsers, User &usr, string &out);
int exec(const char* cmd, string &out);
int ping_cmd(string host, string &out);
int rm_cmd(string fileName, User usr, string &out);
int whoami_cmd(User usr, string &out);
int w_cmd(User usr, string &out);
int date_cmd(bool authenticated, string &out);
int grep_cmd(string pattern, User usr, string &out);

#endif //GRASS_COMMANDS_H
