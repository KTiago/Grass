#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H
#define MAX_DIR_LEN 50
#define OUTFILE_NAME "log.txt"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include "user.h"

using namespace std;

int mkdir_cmd(const char* dir, bool authenticated);
int cd_cmd(const char* dir, bool authenticated);
int ls_cmd(bool authenticated);
int login_cmd(const string uname, map<string, string> allowedUsers, user &usr, string &res);
int pass_cmd(const string psw, map<string, string> allowedUsers, user usr, string &res);
int exec(const char* cmd, string &out);
int ping_cmd(string host, string &out);


#endif //GRASS_COMMANDS_H
