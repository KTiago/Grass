#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H
#define MAX_DIR_LEN 50
#define OUTFILE_NAME "log.txt"

#include <cstdlib>
#include <iostream>

using namespace std;

int mkdir_cmd(const char* dir, bool authenticated);
int cd_cmd(const char* dir, bool authenticated);
int ls_cmd(bool authenticated);
int exec(const char* cmd, string &out);
int ping_cmd(string host, string &out);


#endif //GRASS_COMMANDS_H
