#include "header.h"
#include "all_func.h"
#include <unordered_map>

int port = 8080;
char *ip_addr = "10.248.172.136";
char *runc_path = "/usr/local/sbin/runc";
std::unordered_map<int, int> pid_to_stdin;
int main(int argc, char *argv[])
{
        //daemonize shimd
        if (argc == 4) {
                port = atoi(argv[1]);
                ip_addr = argv[2];
                runc_path = argv[3];
        }
        pid_t pid = fork();
        if (pid == -1) {
                perror("fork");
                exit(-1);
        } else if (pid > 0) {
                exit(0);
        } else { 
                setsid();
                pid = fork();
                if (pid < 0) {
                        perror("fork");
                        exit(-1);
                } else if (pid > 0) {
                        //write shimd pid to file
                        int fd = open("./shimd_pid", O_WRONLY|O_CREAT, 0777);
                        lseek(fd, 0, SEEK_SET);
                        char buf[31] = {0};
                        sprintf(buf, "shimd_pid: %d", pid);
                        int len = strlen(buf);
                        write(fd, buf, len);
                        exit(0);
                } else {
                        umask(0);
                        chdir("/");
                        int fd_open_lim = getdtablesize();
                        for (int i = 0; i < fd_open_lim; ++i) 
                                close(i);
                        open("/dev/null", O_RDWR); 
                        dup(0);
                        dup(0);
                        // config subreaper to make shimd get grandchild's exit code.
                        prctl(PR_SET_CHILD_SUBREAPER, 1, 0, 0, 0); 
                        ignore_sigpipe();
                        listener();
                }
        }
        return 0;
}