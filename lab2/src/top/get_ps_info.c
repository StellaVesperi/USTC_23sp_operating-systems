#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <string.h>

#define max_len 500

struct task_exist{
    int PID;
    char command[16];
    int running;
    double time;
    double CPU;
};

int main(void)
{
    int sum1, sum2;                          // ��������
    int PID1[max_len], PID2[max_len];        // ���̵�PID
    char COMMAND1[max_len][16], COMMAND2[max_len][16];   // ���̵�����
    long running1[max_len], running2[max_len];  // �����Ƿ���������
    unsigned long long time1[max_len], time2[max_len]; // ����ִ��ʱ��
    struct task_exist task[max_len];        // �洢������Ϣ�Ľṹ��
    int i, j;
    int k = 0;

    syscall(333, &sum1, PID1, COMMAND1, running1, time1); // ��ȡ��һ�εĽ�����Ϣ

    while (1) {
        sleep(1);
        system("clear");
        syscall(333, &sum2, PID2, COMMAND2, running2, time2); // ��ȡ��ǰ�Ľ�����Ϣ

        k = 0;
        // �ҵ���һ�κ͵�ǰ�����ڵĽ��̣�������Ϣ�洢���ṹ����
        for (i = 0; i < sum2; i++) {
            for (j = 0; j < sum1; j++) {
                if (PID2[i] == PID1[j]) {
                    task[k].PID = PID2[i];
                    task[k].running = (int)running2[i] ? 0 : 1;
                    task[k].time = time2[i] / 1000000000.0;
                    task[k].CPU = (time2[i] - time1[j]) / 10000000.0;
                    strcpy(task[k].command, COMMAND2[i]);
                    k++;
                    break;
                }
            }
        }

        // ����CPUռ��������
        int max_num;
        double max;
        int flag[max_len]; // ���ĳ�������Ƿ��Ѿ�����ӡ��
        for (i = 0; i < max_len; i++)
            flag[i] = 1;
        printf("%-5s%-16s%-10s%-10s%-10s\n", "PID", "COMM", "ISRUNNING", "%CPU","TIME");
        // �ҵ�ռ��CPU��ߵ�ǰ20������
        for (i = 0; i < 20 && i < k; i++) {
            for (j = 0; j < k; j++) {
                if (flag[j]) {
                    max = task[j].CPU;
                    max_num = j;
                    break;
                }
            }
            for (; j < k; j++) {
                if (task[j].CPU > max && flag[j]) {
                    max = task[j].CPU;
                    max_num = j;
                }
            }
            printf("%-5d%-16s%-10d%-10.2lf%-10.2lf\n", task[max_num].PID, task[max_num].command, task[max_num].running, task[max_num].CPU, task[max_num].time);
            flag[max_num] = 0;
        }

        // ������һ�εĽ�����ϢΪ��ǰ������Ϣ
        sum1 = sum2;
        for (i = 0; i < sum2; i++) {
            PID1[i] = PID2[i];
            strcpy(COMMAND1[i], COMMAND2[i]);
            running1[i] = running2[i];
            time1[i] = time2[i];
        }
    }
    return 0;
}