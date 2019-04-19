#include "commands.h"
#include <fstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <stdio.h>
#include <algorithm>
#include <sstream>
#include <iterator>

using namespace std;

/*
 * Code snippet taken from:
 * https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-output-of-command-within-c-using-posix
 */
/**
 * Executes given command on the server.
 *
 * @param cmd, command to be executed.
 * @param out, stdout result of command
 * @return 0 if successful, 1 otherwise
 */
int exec(const char* cmd, string &out) {
    char buffer[128];
    std::string result;
    char cmdRediction [150];
    strcpy(cmdRediction,cmd);
    strcat(cmdRediction, " 2>&1");


    FILE* pipe = popen(cmdRediction, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        return 1;
    }
    pclose(pipe);
    out = result;
    // FIXME
    return result.substr(0,3) == "sh:" ? 1: 0;
}

/**
 * Sanitize a given path with respect to ".." and "."
 *
 * @param targetPath
 *      path to be sanitized
 * @param out
 *      stores error messages
 * @return
 *      error code
 */
int sanitizePath(string &targetPath,  string &out){
    const char *delim = "/";
    char* targetPathCopy = strdup(targetPath.c_str());
    char* token = strtok(targetPathCopy, delim);
    int cnt = 0;
    vector<string> sanitizedPath;
    while (token != nullptr)
    {
        if(!strcmp(token, "..")){
            cnt--;
            if (cnt < 0){
                out = "Error: access denied\n";
                return 1;
            }

            sanitizedPath.pop_back();
        }
        else if(strcmp(token, ".")){
            cnt++;
            sanitizedPath.emplace_back(token);
        }

        token = strtok(nullptr, "/");
    }
    free(targetPathCopy);
    stringstream s;
    copy(sanitizedPath.begin(), sanitizedPath.end(), ostream_iterator<string>(s, delim));
    targetPath = baseDirectory + "/" + s.str();
    // copy adds a trailing delimiter, which is removed here
    targetPath.pop_back();

    return 0;

}

/**
 * Construct path relative to the base directory
 * @param relativePath
 *          command path relative to user location
 * @param usrLocation
 *          user location relative to base directory
 * @param absPath
 *          path built according to relativePath and usrLocation, relative to base directory (i.e., absolute in the client's point of view)
 * @param out
 *          stores error messages
 * @return
 *          error code
 */
int constructPath(string relativePath, const string &usrLocation, string &absPath, string &out){
    if(relativePath.at(0) == '/'){
        // the path is absolute from the client's point of view but is actually relative to the server's base directory
        absPath = relativePath.substr(1);
    }
    else if(!isalnum(relativePath.at(0)) and relativePath.at(0) != '.'){ //FIXME
        out = "Error: directory path not allowed\n";
        return 1;
    }
    else{
        absPath = usrLocation + "/" + relativePath; // FIXME one can execute an other command in "relativePath"
    }
    int res = sanitizePath(absPath, out);
    return res;
}

int mkdir_cmd(string dirPath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: mkdir may only be executed after authentication";
        return 1;
    }
    string absPath;
    if(constructPath(dirPath, usr.getLocation(), absPath, out)){
        return 1;
    }
    string cmd = "mkdir " + absPath;
    return exec(cmd.c_str(), out);
}


int cd_cmd(string dirPath, User &usr, string &out){
    cout << "cd start: "<< usr.getLocation() << endl;
    if (!usr.isAuthenticated()) {
        out = "Error: cd may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if(constructPath(dirPath, usr.getLocation(), absPath, out)){
        return 1;
    }
    string cmd = "cd " + absPath;
    int res = exec(cmd.c_str(), out);
    if(!res){
        usr.setLocation(absPath);
        cout << "abs : " << absPath << endl;
    }
    cout << "cd end: " <<    usr.getLocation() << endl;
    return res;

}

int ls_cmd(bool authenticated, string &out, string usrLocation){
    if(!authenticated){
        out = "Error: ls may only be executed after authentication\n";
        return 1;
    }
    string cmd = "ls -l " + usrLocation;
    return exec(cmd.c_str(), out);
}

/**
 * The ping may always be executed even if the user is not authenticated.
 * The ping command takes one parameter, the host of the machine that is about
 * to be pinged (ping $HOST). The server will respond with the output of the Unix
 * command ping $HOST -c 1.
 * @param host
 * @return string output of ping command
 */
int ping_cmd(string host, string &out){
    string s = "ping " + host + " -c 1"; // FIXME security vulnerability ! One can change de command !
    return exec(s.c_str(), out);
}


int login_cmd(const string uname, map<string, string> allowedUsers, User &usr, string &out){
    if(usr.isAuthenticated()){
        out = "Error: User already logged in\n";
        return 1;
    }
    usr.resetUname();
    if (allowedUsers.find(uname) == allowedUsers.end()){
        out = "Error: unknown User " + uname + "\n";
        return 1;
    }
    usr.setUname(uname);
    return 0;
}

int logout_cmd(User &usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: login may only be executed after authentication\n";
        return 1;
    }
    usr.resetUname();
    usr.setAuthenticated(false);
    return 0;
}


int pass_cmd(const string psw, map<string, string> allowedUsers, User &usr, string &out){
    if(usr.isAuthenticated()){
        out = "Error: User already logged in\n";
        return 1;
    }
    if(usr.getUname().empty()){
        cout << usr.getUname();
        out = "Error: login command required directly before pass\n";
        return 1;
    }
    if(allowedUsers[usr.getUname()] != psw){
        out = "Error: wrong password\n";
        return 1;
    }
    usr.setAuthenticated(true);
    out = usr.getUname() + " successfully logged in !\n";
    return 0;

}

int rm_cmd(string filePath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: rm may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if(constructPath(filePath, usr.getLocation(), absPath, out)){
        return 1;
    }
    string cmd = "rm " + absPath;
    return exec(cmd.c_str(), out);
}

int whoami_cmd(User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: whoami may only be executed after authentication\n";
        return 1;
    }
    out = usr.getUname() + "\n";
    return 0;
}

int w_cmd(User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: w may only be executed after authentication\n";
        return 1;
    }
    // Print all connected users
    for (auto it=connected_users.begin(); it != connected_users.end(); ++it) {
        out += (*it).getUname() + "\n";
    }
    return 0;
}

int date_cmd(bool authenticated, string &out){
    if(!authenticated){
        out = "Error: date may only be executed after authentication\n";
        return 1;
    }
    return exec("date", out);
}

int grep_cmd(string pattern, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: grep may only be executed after authentication\n";
        return 1;
    }
    string cmd = "grep -l -r \"" + pattern + "\" " + usr.getLocation();
    return exec(cmd.c_str(), out);
}