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


bool sanitizePath(string &targetPath,  string &out){
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
                return false;
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
    targetPath = s.str();
    // copy adds a trailing delimiter, which is removed here
    targetPath.pop_back();
    return cnt >= 0;

}

int pseudoAbsolutePath(string relativePath, const string &usrLocation, string &absPath, string &out){
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
    return 0;
}

int mkdir_cmd(string dirPath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: mkdir may only be executed after authentication";
        return 1;
    }
    string absPath;
    if(pseudoAbsolutePath(dirPath, usr.getLocation(), absPath, out) or !sanitizePath(absPath, out)){
        return 1;
    }
    string cmd = "mkdir " + absPath;
    return exec(cmd.c_str(), out);
}


int cd_cmd(string dirPath, User &usr, string &out){
    if (!usr.isAuthenticated()) {
        out = "Error: cd may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if(pseudoAbsolutePath(dirPath, usr.getLocation(), absPath, out) or !sanitizePath(absPath, out)){
        return 1;
    }
    string cmd = "cd " + absPath;
    int res = exec(cmd.c_str(), out);
    if(!res){
        usr.setLocation(absPath);
    }
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
 *
 * @param host argument to ping command
 * @param out output string
 * @return 0 if successful
 */
int ping_cmd(string host, string &out){
    string s = "ping " + host + " -c 1"; // FIXME security vulnerability ! One can change de command !
    return exec(s.c_str(), out);
}

/**
 * The login command starts authentication. The format is login $USERNAME,
 * followed by a newline. The username must be one of the allowed usernames in the configuration file.
 *
 * @param uname credential user wants to log in as
 * @param allowedUsers map of allowed users
 * @param usr object of user attempting login
 * @param out output string
 * @return 0 if successful
 */
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


/**
 * The pass command must directly follow the login command.
 * The format is pass $PASSWORD, followed by a newline. The password must match the
 * password for the earlier specified user. If the password matches, the user is successfully authenticated.
 *
 * @param psw password that user sent
 * @param allowedUsers map of allowed user credential
 * @param usr object of user attempting login
 * @param out output string
 * @return 0 if successful
 */
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

/**
 * The logout command may only be executed after a successful authentication.
 * The logout command takes no parameters and logs the user out of her session.
 *
 * @param usr object of user wanting to log out
 * @param out output string
 * @return 0 is successful
 */
int logout_cmd(User &usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: login may only be executed after authentication\n";
        return 1;
    }
    usr.resetUname();
    usr.setAuthenticated(false);
    return 0;
}

/**
 * The rm command may only be executed after a successful authentication.
 * The rm command takes exactly one parameter (rm $NAME) and deletes the file
 * or directory with the specified name in the current working directory.
 *
 * @param filePath path/name of file/directory to be deleted
 * @param usr object of user wanting to rm file
 * @param out output string
 * @return 0 if successful
 */
int rm_cmd(string filePath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: rm may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if(pseudoAbsolutePath(filePath, usr.getLocation(), absPath, out) or !sanitizePath(absPath, out)){
        cout << "oops";
        return 1;
    }
    cout << "building cmd...";
    string cmd = "rm -r" + absPath;
    cout << cmd;
    cout << "exec";
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
    for (set<User>::iterator it=connected_users.begin(); it!=connected_users.end(); ++it) {
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