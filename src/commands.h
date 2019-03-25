#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H
#define MAX_DIR_LEN 50
#define OUTFILE_NAME "log.txt"

#include <cstdlib>
#include <iostream>

using namespace std;

int mkdir(const char* dir, bool authenticated);
int cd_(const char* dir, bool authenticated);
int ls_(const char* dir, bool authenticated);
int ls_(bool authenticated);

#endif //GRASS_COMMANDS_H
