#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H
#define MAX_DIR_LEN 50
#define OUTFILE_NAME "log.txt"

#include <cstdlib>
#include <iostream>

using namespace std;

int mkdir_cmd(const char* dir);
int cd_cmd(const char* dir);
int ls_cmd(const char* dir);
int ls_cmd();

#endif //GRASS_COMMANDS_H
