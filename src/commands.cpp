#include "commands.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <stdio.h>


using namespace std;

/*
 * Code snippet taken from:
 * https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-output-of-command-within-c-using-posix
 */
string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int execute_cmd(const char* cmd_name, const char* arg){
    char cmd[MAX_DIR_LEN + 6];
    strcpy(cmd, cmd_name);
    strncat(cmd, arg, MAX_DIR_LEN);
    strcat(cmd, " > ");
    strcat(cmd, OUTFILE_NAME);
    return system(cmd);
}

int checkAuthentication(const string &cmdName, bool authenticated){
    if(!authenticated){
        ofstream outfile;
        outfile.open (OUTFILE_NAME);
        outfile << cmdName << " may only be executed after a successful authentication\n";
        return 1;
    }
    return 0;
}

int mkdir_cmd(const char* dir, bool authenticated){
    if(checkAuthentication("mkdir", authenticated)){
        return 1;
    }
    return execute_cmd("mkdir ", dir);
}


int cd_cmd(const char* dir, bool authenticated){
    if(checkAuthentication("cd", authenticated)){
        return 1;
    }
    return execute_cmd("cd ", dir);
}

long get_cmd(const char* file_name, bool authenticated){
    //if(checkAuthentication("get", authenticated)){//FIXME
    //    return -1;
    //}
    FILE *fp;
    fp = fopen(file_name, "r");
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    if (file_size == EOF)
        return -1;
    fclose(fp);
    return file_size;
}

/* We are not asked to implement ls with an argument
 *
int ls_cmd(const char* dir, bool authenticated){
    if(checkAuthentication("ls", authenticated)){
        return 1;
    }
    return execute_cmd("ls ", dir);
}*/

int ls_cmd(bool authenticated){
    if(checkAuthentication("ls", authenticated)){
        return 1;
    }
    return execute_cmd("ls ", " -l ");
}

/**
 * The ping may always be executed even if the user is not authenticated.
 * The ping command takes one parameter, the host of the machine that is about
 * to be pinged (ping $HOST). The server will respond with the output of the Unix
 * command ping $HOST -c 1.
 * @param host
 * @return string output of ping command
 */
string ping_cmd(string host){
    string s = "ping " + host + " -c 1"; // FIXME security vulnerability ! One can change de command !
    return exec(s.c_str());
}

