#include "header.h"
#include "data_def.h"
#include "all_func.h"

void set_port_reuse(int fd)
{
        int reuse = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)(&reuse), sizeof(reuse));
}
void *new_waiter(void *arg)
{
        pthread_detach(pthread_self());
        Pid_And_Stdin pid_and_stdin = (*(Pid_And_Stdin *)(arg));
        delete arg;
        waitpid(pid_and_stdin.new_pid, NULL, 0);
        if (pid_and_stdin.std_input_fd != -1)
                close(pid_and_stdin.std_input_fd);
        return NULL;
}
void *waiter(void *arg)
{
        pthread_detach(pthread_self());
        Sockfd_Pid_And_Stdin sockfd_pid_and_stdin = (*(Sockfd_Pid_And_Stdin *)(arg));
        delete arg;
        int container_exit_code = -1;
        waitpid(sockfd_pid_and_stdin.container_pid, &container_exit_code, 0);
        char exit_code[50] = {0};
        strcpy(exit_code, "container_exit_code ");
        int len = strlen("container_exit_code ");
        sprintf(exit_code+len, "%d", container_exit_code);
        write(sockfd_pid_and_stdin.shim_cli_fd, exit_code, strlen(exit_code));
        if (sockfd_pid_and_stdin.std_input_fd != -1)
                close(sockfd_pid_and_stdin.std_input_fd);
        return NULL;
}
void *transfer_data(void *arg)
{
        pthread_detach(pthread_self());
        Sockfd_And_Filefd sockfd_and_filefd = (*(Sockfd_And_Filefd *)(arg));
        delete arg;
        char buf[1051] = {0};
        strcpy(buf, sockfd_and_filefd.str);
        int str_len = strlen(sockfd_and_filefd.str);
        while (true) {
                int len = read(sockfd_and_filefd.file_fd, buf + str_len, 1050 - str_len);
                if (len == 0) {
                        close(sockfd_and_filefd.file_fd);
                        return NULL;
                }
                write(sockfd_and_filefd.shim_cli_fd, buf, str_len + len);
        }
        return NULL;
}
CMD parse_cmd(char *buf, int len) 
{
        int cur_idx = 0;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        if (strcmp(buf, "create_container") == 0) 
                return create_container; 
        if (strcmp(buf, "start_container") == 0) 
                return start_container;
        if (strcmp(buf, "run_container") == 0) 
                return run_container;
        if (strcmp(buf, "exec_command") == 0) 
                return exec_command;
        if (strcmp(buf, "attach_process") == 0) 
                return attach_process;
        if (strcmp(buf, "container_input") == 0) 
                return container_input;
        if (strcmp(buf, "kill_container") == 0) 
                return kill_container;
        if (strcmp(buf, "pause_container") == 0) 
                return pause_container;
        if (strcmp(buf, "resume_container") == 0) 
                return resume_container;
        if (strcmp(buf, "list_container") == 0) 
                return list_container;
        if (strcmp(buf, "delete_container") == 0) 
                return delete_container;
        if (strcmp(buf, "checkpoint_container") == 0) 
                return checkpoint_container;
        if (strcmp(buf, "restore_container") == 0) 
                return restore_container;
        if (strcmp(buf, "ps_process") == 0) 
                return ps_process;
        if (strcmp(buf, "events_look") == 0) 
                return events_look;
        if (strcmp(buf, "spec_bundle") == 0) 
                return spec_bundle;
        if (strcmp(buf, "update_container") == 0) 
                return update_container;
        if (strcmp(buf, "state_container") == 0) 
                return state_container;
        if (strcmp(buf, "stop_serve") == 0) 
                return stop_serve;
}
void *serve_shim(void *arg)
{
        pthread_detach(pthread_self()); 
        int shim_cli_fd = (*(int *)(arg));
        delete arg;
        int interactive_input_fd = open("/dev/null", O_RDWR);
        int container_pid = -1;
        int std_fd[3] = {-1, -1, -1};
        bool break_flag = false;
        char buf[1001];
        while (break_flag == false) {
                int len = read(shim_cli_fd, buf, 1000);
                if (len == -1) {
                        perror("read");
                        pthread_exit(NULL);
                } else {
                        switch (parse_cmd(buf, len)) {
                        case create_container:
                                Create_Container(buf, len, shim_cli_fd, 
                                                 std_fd, &container_pid);
                                break;
                        case start_container:
                                Start_Container(buf, len, shim_cli_fd, 
                                                container_pid, std_fd);
                                break;
                        case run_container:
                                Run_Container(buf, len, shim_cli_fd, std_fd, 
                                              &container_pid);
                                break;
                        case exec_command:
                                Exec_Command(buf, len, shim_cli_fd);
                                break;
                        case attach_process:
                                Attach_Process(buf, len, &interactive_input_fd);
                                break;
                        case container_input:
                                Container_Input(buf, len, shim_cli_fd, 
                                                interactive_input_fd);
                                break;
                        case kill_container:
                                Kill_Container(buf, len, shim_cli_fd);
                                break;
                        case pause_container:
                                Pause_Container(buf, len, shim_cli_fd);
                                break;
                        case resume_container:
                                Resume_Container(buf, len, shim_cli_fd);
                                break;
                        case list_container:
                                List_Container(buf, len, shim_cli_fd);
                                break;
                        case delete_container:
                                Delete_Container(buf, len, shim_cli_fd);
                                break;
                        case checkpoint_container:
                                Checkpoint_Container(buf, len, shim_cli_fd);
                                break;
                        case restore_container:
                                Restore_Container(buf, len, shim_cli_fd);
                                break;
                        case ps_process:
                                Ps_Process(buf, len, shim_cli_fd);
                                break;
                        case events_look:
                                Events_Look(buf, len, shim_cli_fd);
                                break;
                        case spec_bundle:
                                Spec_Bundle(buf, len, shim_cli_fd);
                                break;
                        case update_container:
                                Update_Container(buf, len, shim_cli_fd);
                                break;
                        case state_container:
                                State_Container(buf, len, shim_cli_fd);
                                break;
                        case stop_serve:
                                Stop_Serve(shim_cli_fd, &break_flag);
                                break;
                        }
                }
        }
        return NULL;
}
void listener()
{
        int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        inet_pton(AF_INET, (const char *)ip_addr, (void *)(&server_addr.sin_addr.s_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        set_port_reuse(listen_fd);
        bind(listen_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        listen(listen_fd, 5);
        while (true) {
                int *shim_cli_fd = new int(-1);
                (*shim_cli_fd) = accept(listen_fd, NULL, NULL);
                set_port_reuse(*shim_cli_fd);
                pthread_create(NULL, NULL, serve_shim, (void *)shim_cli_fd);
        }
}
void ignore_sigpipe()
{
        struct sigaction new_sig_handle;
        new_sig_handle.sa_handler = SIG_IGN;
        sigemptyset(&new_sig_handle.sa_mask);
        new_sig_handle.sa_flags = 0;
        if (sigaction(SIGPIPE, &new_sig_handle, NULL) < 0) {
                perror("sigaction");
                exit(1);
        }
}