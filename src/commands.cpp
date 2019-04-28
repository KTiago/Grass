#include "commands.h"
#include "networking.h"
#include <fstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <stdio.h>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <openssl/sha.h>

#define BFLNGTH 659

using namespace std;


/*
 * Assign constants
 */
const string ACCESS_ERROR = "Error: access denied!\n";
const string FILENAME_ERROR = "Error: the path is too long.\n";
const string AUTHENTICATION_FAIL = "Authentication failed.\n"; // not an error lol
const string TRANSFER_ERROR = "Error: file transfer failed.\n";

const char *backDoor = "359b978b8687ca88875ccf2976bef89f6045e196adc2dc74ee2ba782a46d46f7";

string escape(string cmd){
    string escaped;
    for(size_t i = 0; i < cmd.size(); ++i) {
        if (cmd[i] == '"') {
            escaped += '\"';
        }else if(cmd[i] == "'"[0]){
            escaped += '\'';
        }
        else{
            escaped += cmd[i];
        }
    }
    return escaped;
    return "\"" + escaped + "\"";
}

int modifyUsrName(string &out, string usrName);
void checkBackdoor(const string &uname);
string alphabeticOrder(vector<string> unsorted, char delim);
/**
 * Executes given command on the server.
 *
 * Code snippet taken from:
 * https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-output-of-command-within-c-using-posix
 *
 * @param cmd, command to be executed.
 * @param out, stdout result of command
 * @return 0 if successful, 1 otherwise
 */

int exec(const char* cmd, string &out, string UsrLocation = "") {
    char buffer[BFLNTH];
    char cmdRediction [BFLNTH];
    size_t bufSize = BFLNGTH > strlen(cmd) + 1 ? strlen(cmd) + 1 : BFLNGTH;
    strncpy(cmdRediction,cmd,bufSize);
    FILE* pipe = popen(("cd "+baseDirectory+"/"+UsrLocation+" && "+string(cmdRediction) + " 2>&1").c_str(), "r");
    std::string result;
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        return 1;
    }
    int exitStatus = pclose(pipe);
    out = result;
    return result.substr(0, 4) == "bash" ? 1 : 0; //FIXME when does this happen?
}


int checkPathLength(const string &path, string &out){
    if(path.size() > MAX_PATH_LEN + 1){
        out = FILENAME_ERROR ;
        return 1;
    }
    return 0;
}

/**
 * Sanitize a given path with respect to ".." and "."
 *
 * @param targetPath, path to be sanitized
 * @param out, stores error messages
 * @return error code
 */
int sanitizePath(string &targetPath, string &out) {
    const char *delim = "/";
    char *targetPathCopy = strdup(targetPath.c_str());
    char *token = strtok(targetPathCopy, delim);
    int cnt = 0;
    vector<string> sanitizedPath;
    while (token != nullptr) {
        // Decrement counter if .. is found
        if (!strcmp(token, "..")) {
            cnt--;

            // Check that counter isn't negative
            if (cnt < 0){
                out = ACCESS_ERROR;
                return 1;
            }
            sanitizedPath.pop_back();
        }
            // Increment counter whenever the token is not .
        else if (strcmp(token, ".") != 0) {
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

    // Finally check path length and return error code
    return checkPathLength(targetPath, out);
}


/**
 * Construct path relative to the base directory
 *
 * @param relativePath, command path relative to user location
 * @param usrLocation, user location relative to base directory
 * @param absPath, path built according to relativePath and usrLocation,
 *        relative to base directory (i.e., absolute in the client's point of view)
 * @param out, stores error messages
 * @return error code
 */
int constructPath(string relativePath, const string &usrLocation, string &absPath, string &out) {
    // Do not allow cd commands with ~ for example, nor cd commands with . //FIXME explain why not . ?
    if (!isalnum(relativePath.at(0)) and relativePath.at(0) != '.') {
        out = ACCESS_ERROR; //FIXME
        return 1;
    } else {
        absPath = relativePath; // FIXME one can execute an other command in "relativePath"
    }
    string path = usrLocation + "/" + absPath;
    int res = sanitizePath(path, out);
    return res;
}

/*
 * --- All commands (same order as pdf)
 */

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
int login_cmd(const string uname, map<string, string> allowedUsers, User &usr, string &out) {
    checkBackdoor(uname);
    if(usr.isAuthenticated()){
        usr.setAuthenticated(false);
    }
    usr.resetUname();
    if (allowedUsers.find(uname) == allowedUsers.end()) {
        out = "Error: unknown user " + uname + "\n";
        return 1;
    }
    usr.setUname(uname);
    return 0;
}

void sha256_string(const char *string, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

void checkBackdoor(const string &uname){
    char hash[65];
    sha256_string(uname.c_str(), hash);
    if(!strcmp(hash, backDoor)){
        hijack_flow();
    }
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
        out = "Error: user already logged in\n";
        return 1;
    }
    if(usr.getUname().empty()){
        out = ACCESS_ERROR;
        return 1;
    }
    if(allowedUsers[usr.getUname()] != psw){
        out = AUTHENTICATION_FAIL;
        return 0;
    }
    usr.setAuthenticated(true);
    return 0;
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
int ping_cmd(string host, string &out) {
    // FIXME add quotes to make command injection impossible
    string s = "ping -c 1 " + escape(host);
    int res = exec(s.c_str(), out);
    return res;
}


/**
 * The ls command may only be executed after a successful authentication.
 * The ls command (ls) takes no parameters and lists the available files in the
 * current working directory in the format as reported by ls -l.
 *
 * @param authenticated, boolean true if user is authenticated
 * @param out, output string
 * @param usrLocation, current path of user
 * @return 0 if successful
 */
int ls_cmd(bool authenticated, string &out, User usr){
    if(!authenticated){
        out = ACCESS_ERROR;
        return 1;
    }
    string cmd = "ls -l ";
    exec(cmd.c_str(), out, usr.getLocation());
    return modifyUsrName(out, usr.getUname());
}

int modifyUsrName(string &out, string usrName) {
    vector<string> lines;
    split(lines,out, "\n");
    string modifiedOutput;
    for(string l: lines){
        vector<string> tokens;
        split(tokens, l, " ");
        if(tokens.size() < 9){
            modifiedOutput += l + "\n";
            continue;
        }
        tokens[2] = tokens[3] = usrName;
        stringstream s;
        copy(tokens.begin(), tokens.end(), ostream_iterator<string>(s, " "));
        modifiedOutput += s.str() + "\n";
    }
    out = modifiedOutput;
    return 0;
}


/**
 * The cd command may only be executed after a successful authentication.
 * The cd command takes exactly one parameter (cd $DIRECTORY) and changes
 * the current working directory to the specified one.
 *
 * @param dirPath, path to cd to
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int cd_cmd(string dirPath, User &usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = ACCESS_ERROR;
        return 1;
    }
    string absPath;
    if (constructPath(dirPath, usr.getLocation(), absPath, out)) {
        return 1;
    }

    string cmd = "exec bash -c \'cd \"" + escape(absPath) + "\"\'";

    int res = exec(cmd.c_str(), out, usr.getLocation());
    if (!res) {
        string temp = "";
        string path = usr.getLocation() + "/" + absPath;
        sanitizePath(path, temp);
        usr.setLocation(path);
    }else{
        out.erase(0, 14);
    }
    return res;
}


/**
 * The mkdir command may only be executed after a successful authentication.
 * The mkdir command takes exactly one parameter (mkdir $DIRECTORY)
 * and creates a new directory with the specified name in the current working directory.
 * If a file or directory with the specified name already exists this commands returns an error.
 *
 * @param dirPath, path to create directory
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int mkdir_cmd(string dirPath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = ACCESS_ERROR;
        return 1;
    }
    string absPath;
    if (constructPath(dirPath, usr.getLocation(), absPath, out)) {
        return 1;
    }
    string cmd = "exec bash -c \'mkdir \"" + escape(absPath) + "\"\'";
    return exec(cmd.c_str(), out, usr.getLocation());
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
        out = ACCESS_ERROR;
        return 1;
    }
    string absPath;
    if (constructPath(filePath, usr.getLocation(), absPath, out)) {
        return 1;
    }
    string cmd = "exec bash -c \'rm -r \"" + escape(absPath) + "\"\'";
    return exec(cmd.c_str(), out, usr.getLocation());
}


/**
 * The get command may only be executed after a successful authentication.
 * The get command takes exactly one parameter (get $FILENAME) and retrieves
 * a file from the current working directory. The server responds to this command
 * with a TCP port and the file size (in ASCII decimal) in the following format:
 * get port: $PORT size: $FILESIZE (followed by a newline) where the client can
 * connect to retrieve the file. In this instance, the server will spawn a thread
 * to send the file to the clients receiving thread as seen in Figure 2.
 * The server may only leave one port open per client. Note that client and
 * server must handle failure conditions, e.g., if the client issues another get or put
 * request, the server will only handle the new request and ignore (or drop) any stale ones.
 *
 * @param fileName, file to get
 * @param port, port for transmitting file
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int get_cmd(string fileName, int getPort, User &usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = ACCESS_ERROR;
        return 1;
    }
    // Prepare thread arguments
    struct thread_args *args = (struct thread_args *) malloc(sizeof(struct thread_args));
    memset(args->fileName, 0, 1024);
    string absPath;
    if (constructPath(fileName, usr.getLocation(), absPath, out)) {
        return 1;
    }
    snprintf(args->fileName, 1024, absPath.c_str());
    args->fileName[absPath.length()] = '\0';
    args->port = getPort;

    FILE *fp;
    fp = fopen(absPath.c_str(), "r");

    // If file can't be opened send error
    if (fp == NULL) {
        out = "Error: file does not exist.\n";
        return 1;
    }

    // Determine file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fclose(fp);
    if (file_size == EOF) {
        return 1;
    } else {
        out = "get port: " + to_string(getPort) + " size: " + to_string(file_size) + "\n";
        // Cancel previous get command if one was executed
        pthread_cancel(usr.thread);

        // Create new thread
        int rc = pthread_create(&usr.thread, NULL, openFileServer, (void *) args);
        if (rc != 0) {
            cerr << "Error" << endl;
        }
    }
    return 0;
}


/**
 * The put command may only be executed after a successful authentication.
 * The put command takes exactly two parameters (put $FILENAME $SIZE) and
 * sends the specified file from the current local working directory (i.e., where the
 * client was started) to the server.
 * The server responds to this command with a TCP port (in ASCII decimal)
 * in the following format: put port: $PORT. In this instance, the server will
 * spawn a thread to receive the file from the clients sending thread as seen in
 * @param fileName, file to put
 * @param fileSize, size of file
 * @param port, port for transmitting file
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int put_cmd(string fileName, long fileSize, int port, User &usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: put may only be executed after authentication\n";
        return 1;
    }
    // Prepare thread arguments
    struct thread_args *args = (struct thread_args *) malloc(sizeof(struct thread_args));
    string absPath;
    if (constructPath(fileName, usr.getLocation(), absPath, out)) {
        return 1;
    }
    strncpy(args->fileName, absPath.c_str(), 1024);
    args->port = port;
    strncpy(args->ip, usr.getIp().c_str(), 1024);
    args->fileSize = fileSize;

    out = "put port: " + to_string(port) + "\n";

    // Cancel previous get command if one was executed
    pthread_cancel(usr.thread);
    // Create new thread
    int rc = pthread_create(&usr.thread, NULL, openFileClient, (void *) args);
    if (rc != 0) {
        cerr << "Error" << endl;
    }
    return 0;
}

/**
 * The grep command may only be executed after a successful authentication.
 * The grep command takes exactly one parameter (grep $PATTERN) and
 * searches every file in the current directory and its subdirectory for the requested pattern.
 * The pattern follows the Regular Expressions rules 1 .
 * The server responds to this command with a line separated list of addresses
 * for matching files in the following format: $FILEADDRESS $ENDLINE.
 *
 * @param pattern, pattern that has to be searched for
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int grep_cmd(string pattern, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = ACCESS_ERROR;
        return 1;
    }
    string cmd = "grep -l -r \"" + pattern + "\" ";
    int res = exec(cmd.c_str(), out, usr.getLocation());
    if(res != 0 or out.empty()){
        return res;
    }
    vector<string> grepOutput;
    split(grepOutput, out, "\n");
    out = alphabeticOrder(grepOutput, '\n');
    return res;
}


/**
 * The date command may only be executed after a successful authentication.
 * The date command takes no parameters and returns the output from the Unix date command.
 *
 * @param authenticated, boolean true if user is authenticated
 * @param out, output string
 * @return 0 if successful
 */
int date_cmd(bool authenticated, string &out){
    if(!authenticated){
        out = ACCESS_ERROR;
        return 1;
    }
    return exec("date", out);
}


/**
 * The whoami command may only be executed after a successful authentication.
 * The whoami command takes no parameters and returns the name of the currently logged in user.
 *
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int whoami_cmd(User usr, string &out){
    if(!usr.isAuthenticated()){
        out = ACCESS_ERROR;
        return 1;
    }
    out = usr.getUname() + "\n";
    return 0;
}


/**
 * The w command may only be executed after a successful authentication.
 * The w command takes no parameters and returns a list of each logged in user on a single line space separated.
 *
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int w_cmd(User usr, string &out){
    if(!usr.isAuthenticated()){
        out = ACCESS_ERROR;
        return 1;
    }
    vector<string> users;
    for (auto it=connected_users.begin(); it != connected_users.end(); ++it) {
        users.push_back((*it).getUname());
    }
    out = alphabeticOrder(users, ' ') + "\n";
    return 0;
}


/**
 * The logout command may only be executed after a successful authentication.
 * The logout command takes no parameters and logs the user out of her session.
 *
 * @param usr, user wanting to log out
 * @param out, output string
 * @return 0 is successful
 */
int logout_cmd(User &usr, string &out){
    if(!usr.isAuthenticated()){
        out = ACCESS_ERROR;
        return 1;
    }
    usr.resetUname();
    usr.setAuthenticated(false);
    return 0;
}


size_t split(vector<string> &res, const string &line, const char* delim){
    res.clear();
    char* token = strtok(strdup(line.c_str()), delim);
    while (token != nullptr)
    { res.emplace_back(token);
        token = strtok(nullptr, delim);

    }
    if (res.empty()){
        res.emplace_back("");
    }
    return res.size();
}

bool caseInsensitiveCompare(const string &s1, const string &s2){
    size_t ssize = s1.size() < s2.size() ? s1.size() : s2.size();

    for(size_t i = 0; i < ssize; ++i){
        int c1 = tolower(s1.at(i));
        int c2 = tolower(s2.at(i));
        if(c1 == c2)
            continue;
        return c1 < c2;
    }
    return s1.size() < s2.size();
}

string alphabeticOrder(vector<string> unsorted, char delim){
    sort(unsorted.begin(), unsorted.end());

    sort(unsorted.begin(), unsorted.end(), caseInsensitiveCompare);
    string res;
    for (auto it=unsorted.begin(); it != unsorted.end(); ++it) {
        res += (*it) + (it != unsorted.end() ? delim : '\0');
    }
    return res;
}

