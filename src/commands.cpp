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


using namespace std;


/*
 * Assign constants
 */
const string ACCESS_ERROR = "Error: access denied!";
const string FILENAME_ERROR = "Error: the path is too long.";
const string TRANSFER_ERROR = "Error: file transfer failed.";


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
int exec(const char *cmd, string &out) {
    char buffer[150];
    char cmdRediction[150];

    strcpy(cmdRediction, cmd);

    FILE *pipe = popen((string(cmdRediction) + " 2>&1").c_str(), "r");
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

    // FIXME different option
    cout << "exec status ";
    cout << exitStatus << " result substring " << result.substr(0, 3) << endl;

    return result.substr(0, 3) == "sh:" ? 1 : 0; //FIXME when does this happen?
}


int checkPathLength(const string &path, string &out) {
    if (path.size() > MAX_PATH_LEN + 1) {
        out = FILENAME_ERROR + "\n";
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
            if (cnt < 0) {
                out = ACCESS_ERROR + "\n";
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
    if (relativePath.at(0) == '/') {
        // the path is absolute from the client's point of view but is actually relative to the server's base directory
        absPath = relativePath.substr(1);
    }
        // Do not allow cd commands with ~ for example, nor cd commands with . //FIXME explain why not . ?
    else if (!isalnum(relativePath.at(0)) and relativePath.at(0) != '.') {
        out = "Error: directory path not allowed\n";
        return 1;
    } else {
        absPath = usrLocation + "/" + relativePath; // FIXME one can execute an other command in "relativePath"
    }
    int res = sanitizePath(absPath, out);

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

    if (usr.isAuthenticated()) {
        out = "Error: User already logged in\n";
        return 1;
    }
    usr.resetUname();
    if (allowedUsers.find(uname) == allowedUsers.end()) {
        out = "Error: unknown user " + uname + "\n";
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
int pass_cmd(const string psw, map<string, string> allowedUsers, User &usr, string &out) {
    if (usr.isAuthenticated()) {
        out = "Error: User already logged in\n";
        return 1;
    }
    if (usr.getUname().empty()) {
        cout << usr.getUname();
        out = "Error: login command required directly before pass\n";
        return 1;
    }
    if (allowedUsers[usr.getUname()] != psw) {
        out = "Error: wrong password\n";
        return 1;
    }
    usr.setAuthenticated(true);
    out = usr.getUname() + " successfully logged in !\n";
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
    string s = "ping -c 1 " + host; // + "\" -c 1"; // FIXME security vulnerability ! One can change de command !
    return exec(s.c_str(), out);
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
int ls_cmd(bool authenticated, string &out, string usrLocation) {
    if (!authenticated) {
        out = "Error: ls may only be executed after authentication\n";
        return 1;
    }
    string cmd = "ls -l " + usrLocation;
    return exec(cmd.c_str(), out);
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
        out = "Error: cd may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if (constructPath(dirPath, usr.getLocation(), absPath, out)) {
        return 1;
    }
    string cmd = "cd \"" + absPath + "\"";
    int res = exec(cmd.c_str(), out);
    if (!res) {
        usr.setLocation(absPath);
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
int mkdir_cmd(string dirPath, User usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: mkdir may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if (constructPath(dirPath, usr.getLocation(), absPath, out)) {
        return 1;
    }
    string cmd = "mkdir \"" + absPath + "\"";
    return exec(cmd.c_str(), out);
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
int rm_cmd(string filePath, User usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: rm may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if (constructPath(filePath, usr.getLocation(), absPath, out)) {
        return 1;
    }
    string cmd = "rm -r \"" + absPath + "\"";
    return exec(cmd.c_str(), out);
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
 * @param getPort, port for transmitting file
 * @param usr, user wanting to execute command
 * @param out, output string
 * @return 0 if successful
 */
int get_cmd(string fileName, int getPort, User &usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: get may only be executed after authentication\n";
        return 1;
    }
    // Prepare thread arguments
    struct thread_args *args = (struct thread_args *) malloc(sizeof(struct thread_args));
    memset(args->fileName, 0, 1024);
    snprintf(args->fileName, 1024, fileName.c_str()); // HUEHUEHUEHUEHUEHUEH fileName.copy(args->fileName, 1024);
    args->fileName[fileName.length()] = '\0';
    args->port = getPort;

    FILE *fp;
    fp = fopen(fileName.c_str(), "r");

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
 *
 * @return 0 if successful
 */
int put_cmd(string fileName, long fileSize, int port, User &usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: put may only be executed after authentication\n";
        return 1;
    }

    // Prepare thread arguments
    struct thread_args *args = (struct thread_args *) malloc(sizeof(struct thread_args));
    strncpy(args->fileName, fileName.c_str(), 1024);
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
int grep_cmd(string pattern, User usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: grep may only be executed after authentication\n";
        return 1;
    }
    string cmd = "grep -l -r \"" + pattern + "\" " + usr.getLocation();
    return exec(cmd.c_str(), out);
}


/**
 * The date command may only be executed after a successful authentication.
 * The date command takes no parameters and returns the output from the Unix date command.
 *
 * @param authenticated, boolean true if user is authenticated
 * @param out, output string
 * @return 0 if successful
 */
int date_cmd(bool authenticated, string &out) {
    if (!authenticated) {
        out = "Error: date may only be executed after authentication\n";
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
int whoami_cmd(User usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: whoami may only be executed after authentication\n";
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
int w_cmd(User usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: w may only be executed after authentication\n";
        return 1;
    }
    // Print all connected users
    for (auto it = connected_users.begin(); it != connected_users.end(); ++it) {
        out += (*it).getUname() + "\n";
    }
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
int logout_cmd(User &usr, string &out) {
    if (!usr.isAuthenticated()) {
        out = "Error: logout may only be executed after authentication\n";
        return 1;
    }
    usr.resetUname();
    usr.setAuthenticated(false);
    return 0;
}


