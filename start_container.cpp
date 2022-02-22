#include "header.h"
#include "data_def.h"
#include "all_func.h"

void Start_Container(char *buf, int len, int shim_cli_fd, int container_pid, int *std_fd)
{
        int pipe_fd[2];
        pipe(pipe_fd);
        char *argv[30];
        argv[0] = "runc";
        int cur_idx = strlen("start_container ");
        int tmp_idx = cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        ++cur_idx;
        int next_idx = 1;
        if (strcmp(buf + tmp_idx, "-gb") == 0) {
                while (true) {
                        tmp_idx = cur_idx;
                        while (cur_idx < len && buf[cur_idx] != ' ') ++cur_idx;
                        buf[cur_idx] = '\0';
                        ++cur_idx;
                        if (strcmp(buf + tmp_idx, "-ge") == 0) 
                                break;
                        argv[next_idx++] = buf + tmp_idx;
                }
                argv[next_idx++] = "start";
        } else {
                argv[next_idx++] = "start";
                argv[next_idx++] = buf + tmp_idx;
        }
        while (cur_idx < len) {
                if (cur_idx > len) 
                        break;
                int tmp_idx = cur_idx;
                while (cur_idx < len && buf[cur_idx] != ' ') ++cur_idx;
                if (cur_idx == len) {
                        buf[cur_idx] = '\0';
                        argv[next_idx++] = buf + tmp_idx;
                        break;
                }
                buf[cur_idx] = '\0';
                argv[next_idx++] = buf + tmp_idx;
                ++cur_idx;
        }
        argv[next_idx++] = (char *)NULL;
        pid_t pid = fork();
        if (pid == -1) {
                perror("fork");
                pthread_exit(NULL);
        } else if (pid > 0) {
                close(pipe_fd[1]);
                int runc_exit_code = -1;
                waitpid(pid, &runc_exit_code, 0);
                if (runc_exit_code != 0) {
                        char err_info[1001] = {0};
                        strcpy(err_info, "start_container_error ");
                        int len = strlen("start_container_error ");
                        int read_len = read(pipe_fd[0], err_info + len, 1000 - len);
                        err_info[len + read_len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe_fd[0]);
                } else {
                        Sockfd_Pid_And_Stdin *sockfd_pid_and_stdin = new Sockfd_Pid_And_Stdin;
                        (*sockfd_pid_and_stdin).shim_cli_fd = shim_cli_fd;
                        (*sockfd_pid_and_stdin).container_pid = container_pid;
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
                close(pipe_fd[0]);
                close(2);
                dup(pipe_fd[1]);
                close(pipe_fd[1]);
                execv((const char *)runc_path, (char *const *)argv);
                perror("execv");
                exit(1);
        }
}