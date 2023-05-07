#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <paths.h>

#define MAX_CMDLINE_LENGTH  1024    /* max cmdline length in a line*/
#define MAX_BUF_SIZE        4096    /* max buffer size */
#define MAX_CMD_ARG_NUM     32      /* max number of single command args */
#define WRITE_END 1     // pipe write end
#define READ_END 0      // pipe read end

#define PATH_SIZE 50
extern char **__environ;
/* 
 * ��Ҫ�����ɵĴ����Ѿ���ע��TODO���
 * ���Ա༭�������ҵ�
 * ʹ��֧��TODO�����༭������vscodeװTODO highlight�������ͬѧ���������ҵ�Ҫ������ݵĵط���
 */

/*  
    int split_string(char* string, char *sep, char** string_clips);

    ���ڷָ���sep����string���ָ��ȥ��ͷβ�Ŀո�

    arguments:      char* string, ����, ���ָ���ַ��� 
                    char* sep, ����, �ָ��
                    char** string_clips, ���, �ָ�õ��ַ�������

    return:   �ָ�Ķ��� 
*/

int split_string(char* string, char *sep, char** string_clips) {
    
    char string_dup[MAX_BUF_SIZE];
    string_clips[0] = strtok(string, sep);
    int clip_num=0;
    
    do {
        char *head, *tail;
        head = string_clips[clip_num];
        tail = head + strlen(string_clips[clip_num]) - 1;
        while(*head == ' ' && head != tail)
            head ++;
        while(*tail == ' ' && tail != head)
            tail --;
        *(tail + 1) = '\0';
        string_clips[clip_num] = head;
        clip_num ++;
    }while(string_clips[clip_num]=strtok(NULL, sep));
    return clip_num;
}

/*
    ִ����������
    arguments:
        argc: ���룬����Ĳ�������
        argv: ���룬���δ���ÿ��������ע���һ����������Ҫִ�е����
        ��ִ��"ls a b c"�����argc=4, argv={"ls", "a", "b", "c"}
        fd: ��������������������ļ������� (Deprecated)
    return:
        int, ��ִ�гɹ�����0�����򷵻�ֵ����
*/
int exec_builtin(int argc, char**argv, int *fd) {
    if(argc == 0) {
        return 0;
    }
    /* TODO : ��Ӻ�ʵ������ָ�� */

    if (strcmp(argv[0], "cd") == 0) {
        if(chdir(argv[1]) != 0){
            printf("cd: no such file or directory: %s", argv[1]);
            return -1;
        }
    } else if (strcmp(argv[0], "exit") == 0){
       exit(0);
    } else {
        // ��������ָ��ʱ
        return -1;
    }
}

/*
    ��argv��ɾ���ض���������Ĳ��������򿪶�Ӧ���ļ������ļ�����������fd�����С�
    ���к�fd[0]���˵��ļ���������fd[1]��д�˵��ļ�������
    arguments:
        argc: ���룬����Ĳ�������
        argv: ���룬���δ���ÿ��������ע���һ����������Ҫִ�е����
        ��ִ��"ls a b c"�����argc=4, argv={"ls", "a", "b", "c"}
        fd: �����������������ʹ�õ��ļ�������
    return:
        int, ���ش�����ض��������Ĳ�������
*/

int process_redirect(int argc, char** argv, int *fd) {
    /* Ĭ����������������У�������STDIN_FILENO�����STDOUT_FILENO */
    fd[READ_END] = STDIN_FILENO;
    fd[WRITE_END] = STDOUT_FILENO;
    int i = 0, j = 0;
    while(i < argc) {
        int tfd;
        if(strcmp(argv[i], ">") == 0) {
            //TODO : ������ļ���ͷд��
            tfd = open(argv[i + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
            if(tfd < 0) {
                printf("open '%s' error: %s\n", argv[i+1], strerror(errno));
            } else {
                //TODO : ����ض���
                fd[WRITE_END] = tfd;
            }
            i += 2;
        } else if(strcmp(argv[i], ">>") == 0) {
            //TODO ������ļ�׷��д��
            tfd = open(argv[i + 1], O_RDWR | O_CREAT | O_APPEND, 0666);
            if(tfd < 0) {
                printf("open '%s' error: %s\n", argv[i+1], strerror(errno));
            } else {
                //TODO����ض���
                fd[WRITE_END] = tfd;
            }
            i += 2;
        } else if(strcmp(argv[i], "<") == 0) {
            //TODO �������ļ�
            tfd = open(argv[i + 1], O_RDONLY);
            if(tfd < 0) {
                printf("open '%s' error: %s\n", argv[i+1], strerror(errno));
            } else {
                //TODO  ����ض���
                fd[READ_END] = tfd;
            }
            i += 2;
        } else {
            argv[j++] = argv[i++];
        }
    }
    argv[j] = NULL;
    return j;   // �µ�argc
}



/*
    �ڱ�������ִ�У���ִ����Ϻ�������̡�
    arguments:
        argc: ����Ĳ�������
        argv: ���δ���ÿ��������ע���һ����������Ҫִ�е����
        ��ִ��"ls a b c"�����argc=4, argv={"ls", "a", "b", "c"}
    return:
        int, ��ִ�гɹ��򲻻᷵�أ�����ֱ�ӽ����������򷵻ط���
*/
int execute(int argc, char** argv) {
    int fd[2];
    // Ĭ����������������У�������STDIN_FILENO�����STDOUT_FILENO 
    fd[READ_END] = STDIN_FILENO;
    fd[WRITE_END] = STDOUT_FILENO;
    // �����ض����������������������ݣ���ע�͵�process_redirect�ĵ���
    argc = process_redirect(argc, argv, fd);
    if(exec_builtin(argc, argv, fd) == 0) {
        exit(0);
    }
    // ����׼�������STDIN_FILENO��STDOUT_FILENO�޸�Ϊfd��Ӧ���ļ�
    dup2(fd[READ_END], STDIN_FILENO);
    dup2(fd[WRITE_END], STDOUT_FILENO);
    /* TODO������������� */
    execvp(argv[0], argv);
    return 0;
}

int main() {
    /* ����������� */
    char cmdline[MAX_CMDLINE_LENGTH];

    char *commands[128];
    char *multi_cmd[128];
    int cmd_count;
    while (1) {
        /* TODO : ���Ӵ�ӡ��ǰĿ¼����ʽ����"shell:/home/oslab ->"������Ҫ�������printf */
        char path_name[51];
        getcwd(path_name, PATH_SIZE);
        printf("shell:%s -> ", path_name);
        fflush(stdout);

        fgets(cmdline, 256, stdin);
        strtok(cmdline, "\n");

        /* TODO: ����";"�Ķ�����ִ�У�������ѡ��λ����� */
        int multi_cmd_num = split_string(cmdline, ";", multi_cmd);
        for(int i = 0; i < multi_cmd_num; i++){
            strcpy(cmdline, multi_cmd[i]);

            /* �ɹܵ�������'|'�ָ�������и������֣�ÿ��������һ������ */
            /* ��������� */
            cmd_count = split_string(cmdline, "|", commands);

            if(cmd_count == 0) {
                continue;
            } else if(cmd_count == 1) {     // û�йܵ��ĵ�һ����
                char *argv[MAX_CMD_ARG_NUM];
                int argc;
                int fd[2];
                /* TODO :����������ֳ��������Ͳ���
                *
                *
                * 
                */
                argc = split_string(cmdline, " ", argv);

                /* ��û�йܵ�ʱ���ڽ�����ֱ��������������ɣ��ⲿ����ͨ�������ӽ������ */
                if(exec_builtin(argc, argv, fd) == 0) {
                    continue;
                }
                /* TODO :�����ӽ��̣���������ȴ��������н���
                *
                *
                *
                *
                */
                pid_t pid = fork();
                if(pid == 0) {
                    if(execute(argc, argv) < 0) {
                        printf("%s : Command not found.\n",argv[0]);
                        exit(0);
                    }
                }
                while(wait(NULL) > 0);

            } else if(cmd_count == 2) {     // ���������Ĺܵ�
                int pipefd[2];
                int ret = pipe(pipefd);
                if(ret < 0) {
                    printf("pipe error!\n");
                    continue;
                }
                // �ӽ���1
                int pid = fork();
                if(pid == 0) {  
                    /*TODO :�ӽ���1 ����׼����ض��򵽹ܵ���ע������������±걻�ڿ���Ҫ��ȫ*/
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                    /* 
                        ��ʹ�ùܵ�ʱ��Ϊ�˿��Բ������У������ڽ�����Ҳ���ӽ���������
                        �����������һ����װ�õ�execute����
                    */
                    char *argv[MAX_CMD_ARG_NUM];

                    int argc = split_string(commands[0], " ", argv);
                    execute(argc, argv);
                    exit(255);
                    
                }
                // ��Ϊ��shell������У��ܵ��ǲ���ִ�еģ��������ǲ���ÿ���ӽ��̽������������һ��
                // ����ֱ�Ӵ�����һ���ӽ���
                // �ӽ���2
                pid = fork();
                if(pid == 0) {  
                    /* TODO :�ӽ���2 ����׼�����ض��򵽹ܵ���ע������������±걻�ڿ���Ҫ��ȫ */
                    close(pipefd[1]);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);

                    char *argv[MAX_CMD_ARG_NUM];
                    /* TODO :����������ֳ��������Ͳ�������ʹ��execute����
                    * ��ʹ�ùܵ�ʱ��Ϊ�˿��Բ������У������ڽ�����Ҳ���ӽ���������
                    * �����������һ����װ�õ�execute����
                    *
                    * 
                    */
                    int argc = split_string(commands[1], " ", argv);
                    execute(argc, argv);
                    exit(255);
                }
                close(pipefd[WRITE_END]);
                close(pipefd[READ_END]);
                
                while (wait(NULL) > 0);
            } else {    // ѡ�����������ϵ�����
                int read_fd;    // ��һ���ܵ��Ķ��˿ڣ����ڣ�
                for(int i = 0; i < cmd_count; i++) {
                    int pipefd[2];
                    /* TODO :�����ܵ���n������ֻ��Ҫn-1���ܵ���������һ��ѭ�����ǲ��ô����ܵ���
                    *
                    *
                    * 
                    */
                    if(i != cmd_count - 1){
                        int ret = pipe(pipefd);
                        if(ret < 0) {
                            printf("pipe error!\n");
                            continue;
                        }
                    }

                    int pid = fork();
                    if(pid == 0) {
                        /* TODO :�������һ�������⣬������׼����ض��򵽵�ǰ�ܵ����
                        *
                        *
                        * 
                        */
                        if(i != cmd_count - 1) {
                            close(pipefd[0]);
                            dup2(pipefd[1], STDOUT_FILENO);
                            close(pipefd[1]);
                        }

                        /* TODO :���˵�һ�������⣬������׼�����ض�����һ���ܵ�����
                        *
                        *
                        * 
                        */
                        if(i != 0) {
                            close(pipefd[1]);
                            dup2(read_fd, STDIN_FILENO);
                            close(read_fd);
                            if(i == cmd_count - 1) close(pipefd[0]);
                        }

                        /* TODO :����������ֳ��������Ͳ�������ʹ��execute����
                        * ��ʹ�ùܵ�ʱ��Ϊ�˿��Բ������У������ڽ�����Ҳ���ӽ���������
                        * �����������һ����װ�õ�execute����
                        * 
                        * 
                        */
                        char *argv[MAX_CMD_ARG_NUM];
                        int argc = split_string(commands[i], " ", argv);
                        execute(argc, argv);
                        exit(255);
                    }
                    /* �����̳��˵�һ���������Ҫ�رյ�ǰ�����������һ���ܵ����˿� 
                    * �����̳������һ���������Ҫ���浱ǰ����Ĺܵ����˿� 
                    * �ǵùرո�����û�õĹܵ�д�˿�
                    * 
                    */
                    if(i != 0) close(read_fd);

                    if(i != cmd_count - 1) read_fd = pipefd[0];
                    
                    close(pipefd[1]);
                    // ��Ϊ��shell������У��ܵ��ǲ���ִ�еģ��������ǲ���ÿ���ӽ��̽������������һ��
                    // ����ֱ�Ӵ�����һ���ӽ���
                }
                // TODO :�ȴ������ӽ��̽���
                while(wait(NULL) > 0);
            }
        }

    }
}