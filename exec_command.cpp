#include "header.h"
#include "data_def.h"
#include "all_func.h"

void Exec_Interactive_Command(char *buf, int len, int cur_idx, int shim_cli_fd)
{
        int pipe1_fd[2];
        int pipe2_fd[2];
        int pipe3_fd[3];
        pipe(pipe1_fd);
        pipe(pipe2_fd);
        pipe(pipe3_fd);
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
                argv[next_idx++] = "exec";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
                argv[next_idx++] = "-d";
        } else {
                argv[next_idx++] = "exec";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
                argv[next_idx++] = "-d";
                argv[next_idx++] = buf + tmp_idx;
        }
        while (cur_idx < len) {
                tmp_idx = cur_idx;
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
        argv[next_idx++] = (char *)(NULL);
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
                        strcpy(err_info, "exec_command_error ");
                        int len = strlen("exec_command_error ");
                        int read_len = read(pipe3_fd[0], err_info + len, 1000 - len);
                        err_info[read_len + len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe1_fd[1]);
                        close(pipe2_fd[0]);
                        close(pipe3_fd[0]);
                } else {
                        int to_wait_pid = -1;
                        int pid_file_fd = open("./pid", O_RDONLY);
                        lseek(pid_file_fd, 0, SEEK_SET);
                        char num[41] = {0};
                        strcpy(num, "new_process_pid ");
                        int str_len = strlen("new_process_pid ");
                        int read_len = read(pid_file_fd, num + str_len, 40 - str_len);
                        num[str_len + read_len] = '\0';
                        to_wait_pid = atoi(num + str_len);
                        write(shim_cli_fd, num, str_len + read_len);
                        pid_to_stdin[to_wait_pid] = pipe1_fd[1];
                        close(pid_file_fd);
                        Pid_And_Stdin *pid_and_stdin = new Pid_And_Stdin;
                        (*pid_and_stdin).new_pid = to_wait_pid;
                        (*pid_and_stdin).std_input_fd = pipe1_fd[1];
                        pthread_create(NULL, NULL, new_waiter, (void *)pid_and_stdin);
                        Sockfd_And_Filefd *sockfd_and_filefd_1 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_1).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_1).file_fd = pipe2_fd[0];
                        (*sockfd_and_filefd_1).str = "new_process_output ";
                        pthread_create(NULL, NULL, transfer_data, (void *)sockfd_and_filefd_1);
                        Sockfd_And_Filefd *sockfd_and_filefd_2 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_2).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_2).file_fd = pipe3_fd[0];
                        (*sockfd_and_filefd_2).str = "new_process_error ";
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
void Exec_Non_Interactive_Command(char *buf, int len, int cur_idx, int shim_cli_fd)
{
        int pipe2_fd[2];
        int pipe3_fd[3];
        pipe(pipe2_fd);
        pipe(pipe3_fd);
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
                argv[next_idx++] = "exec";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
                argv[next_idx++] = "-d";
        } else {
                argv[next_idx++] = "exec";
                argv[next_idx++] = "--pid-file";
                argv[next_idx++] = "./pid";
                argv[next_idx++] = "-d";
                argv[next_idx++] = buf + tmp_idx;
        }
        while (cur_idx < len) {
                tmp_idx = cur_idx;
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
        argv[next_idx++] = (char *)(NULL);
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
                        strcpy(err_info, "exec_command_error ");
                        int len = strlen("exec_command_error ");
                        int read_len = read(pipe3_fd[0], err_info + len, 1000 - len);
                        err_info[read_len + len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe2_fd[0]);
                        close(pipe3_fd[0]);
                } else {
                        int to_wait_pid = -1;
                        int pid_file_fd = open("./pid", O_RDONLY);
                        lseek(pid_file_fd, 0, SEEK_SET);
                        char num[41] = {0};
                        strcpy(num, "new_process_pid ");
                        int str_len = strlen("new_process_pid ");
                        int read_len = read(pid_file_fd, num + str_len, 40 - str_len);
                        num[str_len + read_len] = '\0';
                        to_wait_pid = atoi(num + str_len);
                        write(shim_cli_fd, num, str_len + read_len);
                        close(pid_file_fd);
                        Pid_And_Stdin *pid_and_stdin = new Pid_And_Stdin;
                        (*pid_and_stdin).new_pid = to_wait_pid;
                        (*pid_and_stdin).std_input_fd = -1;
                        pthread_create(NULL, NULL, new_waiter, (void *)pid_and_stdin);
                        Sockfd_And_Filefd *sockfd_and_filefd_1 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_1).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_1).file_fd = pipe2_fd[0];
                        (*sockfd_and_filefd_1).str = "new_process_output ";
                        pthread_create(NULL, NULL, transfer_data, (void *)sockfd_and_filefd_1);
                        Sockfd_And_Filefd *sockfd_and_filefd_2 = new Sockfd_And_Filefd;
                        (*sockfd_and_filefd_2).shim_cli_fd = shim_cli_fd;
                        (*sockfd_and_filefd_2).file_fd = pipe3_fd[0];
                        (*sockfd_and_filefd_2).str = "new_process_error ";
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
void Exec_Command(char *buf, int len, int shim_cli_fd)
{
        int cur_idx = strlen("exec_command ");
        char *bool_str = buf + cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        ++cur_idx;
        Interactive tmp_if_interactive = False;
        if (strcmp(bool_str, "true") == 0) 
                tmp_if_interactive = True;
        switch (tmp_if_interactive) {
        case True:
                Exec_Interactive_Command(buf, len, cur_idx, shim_cli_fd);
                break;
        case False:
                Exec_Non_Interactive_Command(buf, len, cur_idx, shim_cli_fd);
                break;
        }
}