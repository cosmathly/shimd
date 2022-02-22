#include "header.h"
#include "data_def.h"
#include "all_func.h"

void Run_Interactive_Container(char *buf, int len, int cur_idx, char *bundle, 
                               char *container_id, int shim_cli_fd, 
                               int *std_fd, int *container_pid)
{
        int pipe1_fd[2];
        int pipe2_fd[2];
        int pipe3_fd[2];
        pipe(pipe1_fd);
        pipe(pipe2_fd);
        pipe(pipe3_fd);
        std_fd[0] = pipe1_fd[1];
        std_fd[1] = pipe2_fd[0];
        std_fd[2] = pipe3_fd[0];
        char *argv[30];
        argv[0] = "runc";
        int tmp_idx = cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        ++cur_idx;
        int next_idx = 1;
        if (strcmp(buf + tmp_idx, "-gb") == 0) {
                while (true) {
                        tmp_idx = cur_idx;
                        while (cur_idx < len && buf[cur_idx] != ' ') 
                                ++cur_idx;
                        buf[cur_idx] = '\0';
                        ++cur_idx;
                        if (strcmp(buf + tmp_idx, "-ge") == 0) 
                                break;
                        argv[next_idx++] = buf + tmp_idx;
                }
                argv[next_idx++] = "run";
                argv[next_idx++] = "-b";
                argv[next_idx++] = bundle;
                argv[next_idx++] = "-d";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
        } else {
                argv[next_idx++] = "run";
                argv[next_idx++] = "-b";
                argv[next_idx++] = bundle;
                argv[next_idx++] = "-d";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
                argv[next_idx++] = buf + tmp_idx;
        }
        while (cur_idx < len) {
                if (cur_idx > len) 
                        break;
                int tmp_idx = cur_idx;
                while (cur_idx < len && buf[cur_idx] != ' ') 
                        ++cur_idx;
                if (cur_idx == len) {
                        buf[cur_idx] = '\0';
                        argv[next_idx++] = buf + tmp_idx;
                        break;
                }
                buf[cur_idx] = '\0';
                argv[next_idx++] = buf + tmp_idx;
                ++cur_idx;
        }
        argv[next_idx++] = container_id;
        argv[next_idx++] = (char *)NULL;
        pid_t pid = fork();
        if (pid == -1) {
                perror("fork");
                pthread_exit(NULL);
        } else if (pid > 0) {
                close(pipe1_fd[0]);
                close(pipe2_fd[1]);
                close(pipe3_fd[1]);
                int runc_exit_code = -1;
                waitpid(pid, &runc_exit_code, 0);
                if (runc_exit_code != 0) {
                        char err_info[1001] = {0};
                        strcpy(err_info, "run_container_error ");
                        int len = strlen("run_container_error ");
                        int read_len = read(pipe3_fd[0], err_info + len, 1000 - len);
                        err_info[read_len + len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe1_fd[1]);
                        close(pipe2_fd[0]);
                        close(pipe3_fd[0]);
                } else {
                        int pid_file_fd = open("./pid", O_RDONLY);
                        lseek(pid_file_fd, 0, SEEK_SET);
                        char num[31] = {0};
                        strcpy(num, "container_pid ");
                        int str_len = strlen("container_pid ");
                        int read_len = read(pid_file_fd, num + str_len, 30 - str_len);
                        num[str_len + read_len] = '\0';
                        (*container_pid) = atoi(num + str_len);
                        pid_to_stdin[*container_pid] = std_fd[0];
                        write(shim_cli_fd, num, str_len + read_len);
                        close(pid_file_fd);
                        Sockfd_Pid_And_Stdin *sockfd_pid_and_stdin = new Sockfd_Pid_And_Stdin;
                        (*sockfd_pid_and_stdin).shim_cli_fd = shim_cli_fd;
                        (*sockfd_pid_and_stdin).container_pid = (*container_pid);
                        (*sockfd_pid_and_stdin).std_input_fd = std_fd[0];
                        pthread_create(NULL, NULL, waiter, (void *)sockfd_pid_and_stdin);
                        Sockfd_And_Filefd *sockfd_and_filefd_1 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_1).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_1).file_fd = std_fd[1];
                        (*sockfd_and_filefd_1).str = "container_output ";
                        pthread_create(NULL, NULL, transfer_data, (void *)sockfd_and_filefd_1);
                        Sockfd_And_Filefd *sockfd_and_filefd_2 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_2).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_2).file_fd = std_fd[2];
                        (*sockfd_and_filefd_2).str = "container_error ";
                        pthread_create(NULL, NULL, transfer_data, (void *)sockfd_and_filefd_2);
                }
        } else if (pid == 0) {
                close(pipe1_fd[1]);
                close(pipe2_fd[0]);
                close(pipe3_fd[0]);
                close(0);
                close(1);
                close(2);
                dup(pipe1_fd[0]);
                close(pipe1_fd[0]);
                dup(pipe2_fd[1]);
                close(pipe2_fd[1]);
                dup(pipe3_fd[1]);
                close(pipe3_fd[1]);
                execv((const char *)runc_path, (char *const *)argv);
                perror("execv");
                exit(1);
        }
}
void Run_Non_Interactive_Container(char *buf, int len, int cur_idx, char *bundle, 
                                   char *container_id, int shim_cli_fd, 
                                   int *std_fd, int *container_pid)
{
        int pipe2_fd[2];
        int pipe3_fd[2];
        pipe(pipe2_fd);
        pipe(pipe3_fd);
        std_fd[1] = pipe2_fd[0];
        std_fd[2] = pipe3_fd[0];
        char *argv[30];
        argv[0] = "runc";
        int tmp_idx = cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        ++cur_idx;
        int next_idx = 1;
        if (strcmp(buf + tmp_idx, "-gb") == 0) {
                while (true) {
                        tmp_idx = cur_idx;
                        while (cur_idx < len && buf[cur_idx] != ' ') 
                                ++cur_idx;
                        buf[cur_idx] = '\0';
                        ++cur_idx;
                        if (strcmp(buf+tmp_idx, "-ge") == 0) 
                                break;
                        argv[next_idx++] = buf + tmp_idx;
                }
                argv[next_idx++] = "run";
                argv[next_idx++] = "-b";
                argv[next_idx++] = bundle;
                argv[next_idx++] = "-d";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
        } else {
                argv[next_idx++] = "run";
                argv[next_idx++] = "-b";
                argv[next_idx++] = bundle;
                argv[next_idx++] = "-d";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
                argv[next_idx++] = buf + tmp_idx;
        }
        while (cur_idx < len) {
                if (cur_idx > len) 
                        break;
                int tmp_idx = cur_idx;
                while (cur_idx < len && buf[cur_idx] != ' ') 
                        ++cur_idx;
                if (cur_idx == len) {
                        buf[cur_idx] = '\0';
                        argv[next_idx++] = buf + tmp_idx;
                        break;
                }
                buf[cur_idx] = '\0';
                argv[next_idx++] = buf + tmp_idx;
                ++cur_idx;
        }
        argv[next_idx++] = container_id;
        argv[next_idx++] = (char *)NULL;
        pid_t pid = fork();
        if (pid == -1) {
                perror("fork");
                pthread_exit(NULL);
        } else if (pid > 0) {
                close(pipe2_fd[1]);
                close(pipe3_fd[1]);
                int runc_exit_code = -1;
                waitpid(pid, &runc_exit_code, 0);
                if (runc_exit_code != 0) {
                        char err_info[1001] = {0};
                        strcpy(err_info, "run_container_error ");
                        int len = strlen("run_container_error ");
                        int read_len = read(pipe3_fd[0], err_info + len, 1000 - len);
                        err_info[read_len + len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe2_fd[0]);
                        close(pipe3_fd[0]);
                } else {
                        int pid_file_fd = open("./pid", O_RDONLY);
                        lseek(pid_file_fd, 0, SEEK_SET);
                        char num[41] = {0};
                        strcpy(num, "container_pid ");
                        int str_len = strlen("container_pid ");
                        int read_len = read(pid_file_fd, num + str_len, 40 - str_len);
                        num[str_len + read_len] = '\0';
                        (*container_pid) = atoi(num + str_len);
                        write(shim_cli_fd, num, str_len + read_len);
                        close(pid_file_fd);
                        Sockfd_Pid_And_Stdin *sockfd_pid_and_stdin = new Sockfd_Pid_And_Stdin;
                        (*sockfd_pid_and_stdin).shim_cli_fd = shim_cli_fd;
                        (*sockfd_pid_and_stdin).container_pid = (*container_pid);
                        (*sockfd_pid_and_stdin).std_input_fd = -1;
                        pthread_create(NULL, NULL, waiter, (void *)sockfd_pid_and_stdin);
                        Sockfd_And_Filefd *sockfd_and_filefd_1 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_1).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_1).file_fd = std_fd[1];
                        (*sockfd_and_filefd_1).str = "container_output ";
                        pthread_create(NULL, NULL, transfer_data, (void *)sockfd_and_filefd_1);
                        Sockfd_And_Filefd *sockfd_and_filefd_2 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_2).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_2).file_fd = std_fd[2];
                        (*sockfd_and_filefd_2).str = "container_error ";
                        pthread_create(NULL, NULL, transfer_data, (void *)sockfd_and_filefd_2);
                }
        } else if (pid == 0) {
                close(pipe2_fd[0]);
                close(pipe3_fd[0]);
                close(0);
                close(1);
                close(2);
                open("/dev/null", O_RDWR);
                dup(pipe2_fd[1]);
                close(pipe2_fd[1]);
                dup(pipe3_fd[1]);
                close(pipe3_fd[1]);
                execv((const char *)runc_path, (char *const *)argv);
                perror("execv");
                exit(1);
        }
}
void Run_Container(char *buf, int len, int shim_cli_fd, int *std_fd, 
                   int *container_pid)
{
        char *bundle = NULL;
        char *container_id = NULL;
        Interactive if_interactive = False;
        int sub_len = strlen("run_container ");
        int cur_idx = sub_len;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        bundle = buf + sub_len;
        ++cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        container_id = buf + sub_len + strlen(bundle) + 1;
        ++cur_idx;
        int tmp_idx = cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        if (strcmp(buf + tmp_idx, "true") == 0) 
                if_interactive = True;
        ++cur_idx;
        switch (if_interactive) {
        case True:
                Run_Interactive_Container(buf, len, cur_idx, bundle, container_id, 
                                          shim_cli_fd, std_fd, container_pid);
                break;
        case False:
                Run_Non_Interactive_Container(buf, len, cur_idx, bundle, container_id, 
                                              shim_cli_fd, std_fd, container_pid);
                break;
        }
}