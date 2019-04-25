#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H

#define MAX_DIR_LEN 50
#define MAX_PATH_LEN 128
#define OUTFILE_NAME "log.txt"
#define BFLNTH 150

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include "User.h"
#include "grass.hpp"

using namespace std;

extern const string ACCESS_ERROR;
extern const string FILENAME_ERROR;
extern const string TRANSFER_ERROR;

int mkdir_cmd(string dir, User usr, string &out);
int cd_cmd(string dir, User &usr, string &out);
int ls_cmd(bool authenticated, string &out, User usr);
int get_cmd(string fileName, int getPort, User &usr, string &out);
int put_cmd(string fileName, long fileSize, int port, User &usr, string &out);
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


size_t split(vector<string> &res, const string &line, const char* delim);

#endif //GRASS_COMMANDS_H
