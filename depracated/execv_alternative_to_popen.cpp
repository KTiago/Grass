/**
 * Code use to execute commands on server and catch stderr
 *
 * Code snippet taken from:
 *
 * https://jineshkj.wordpress.com/2006/12/22/how-to-capture-stdin-stdout-and-stderr-of-child-program/
 */
#include <unistd.h>
#include <stdio.h>

/* since pipes are unidirectional, we need two pipes.
   one for data to flow from parent's stdout to child's
   stdin and the other for child's stdout to flow to
   parent's stdin */

#define NUM_PIPES          3

#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1
#define PARENT_ERR_PIPE    2


int pipes[NUM_PIPES][3];

/* always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1
#define ERR_FD   2

#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
#define PARENT_ERR_FD   ( pipes[PARENT_ERR_PIPE][ERR_FD] )

#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )
#define CHILD_ERR_FD    ( pipes[PARENT_ERR_PIPE][ERR_FD]  )


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
    FILE* pipe = popen(cmd, "r");
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
    return 0;
}

/**
 * https://jineshkj.wordpress.com/2006/12/22/how-to-capture-stdin-stdout-and-stderr-of-child-program/
 *
 *
 * @param cmd
 * @param out
 * @return
 */
int exec_all(const char* cmd, string &out) {

    //FIXME catch more errors: https://stackoverflow.com/questions/17508626/piping-for-input-output
    // stderr pipes: http://www.cplusplus.com/forum/unices/82836/
    int outfd[2];
    int infd[2];
    int errfd[2];

    // pipes for parent to write and read
    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);
    pipe(pipes[PARENT_ERR_PIPE]);

    if(!fork()) {
        /* This is done by the child process. */

        char *argv[]={ "/bin/ping", "google.h", "-c", "1", 0}; https://stackoverflow.com/questions/190184/execv-and-const-ness

        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);
        dup2(CHILD_ERR_FD, STDERR_FILENO);

        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(CHILD_ERR_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);
        close(PARENT_ERR_FD);

        execv(argv[0], argv);

        /* If execv returns, it must have failed. */
        // FIXME
        printf("Unexpected command: execv failed.\n");
        exit(0);

    } else {
        /* This is run by the parent.  Wait for the child
        to terminate. */

        char readbuffer[100];
        int count;

        /* close fds not required by parent */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(CHILD_ERR_FD);

        // Write to child’s stdin
        // write(PARENT_WRITE_FD, "2^32\n", 5);

        // Read from child’s stdout and
        while (1)
        {
            count = read(PARENT_READ_FD, readbuffer, sizeof(readbuffer)-1);

            if (count <= 0) {
                if (!out.empty()) {
                    printf("Correct execution");
                    return 0;
                } else {
                    break;
                }
            }

            readbuffer[count] = '\0';
            out += readbuffer;
        }

        while (1)
        {
            count = read(PARENT_ERR_FD, readbuffer, sizeof(readbuffer)-1);

            if (count <= 0) {
                return 1;
            }

            readbuffer[count] = '\0';
            out += readbuffer;
            printf(readbuffer);
        }


        // FIXME enough to wait for termination like that, or do something like: http://www.cs.ecu.edu/karl/4630/spr01/example1.html
    }
}


