#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

pthread_mutex_t g_mutex; // 互斥锁
using namespace std;

int arr[20] = {};
int value = 0;
int idx = 0;
bool read_state = true;

void *thread(void *arg)
{
    try
    {
        int ret = pthread_mutex_trylock(&g_mutex);
        if (EBUSY == ret)
        {
            printf("thread: the varible is locked by thread. no need open another thread\n");
            return arg;
        }
        else
        {
            printf("thread: lock the variable!\n");
        }

        printf("This is a thread and arg = %d.\n", *(int *)arg);
        *(int *)arg = 0;
        while (read_state) // 3.1 的情况
        {
            if (idx >= 20)
            {
                idx = 0;
            }
            usleep(90000);
            arr[idx++] = value++;
            if (value == 666)
            {
                throw "test error";
            }
        }
    }
    catch (...) // 3.2 的情况
    {
        cout << "get error" << endl;
    }
    // 3. 互斥锁的状态由 非EBUSY->EBUSY 的情况如下:
    // 3.1 主进程将read_state变为false
    // 3.2 进程工作时出错
    pthread_mutex_unlock(&g_mutex);

    return arg;
}

void printArray(int *arr)
{
    for (int i = 0; i < 20; i++)
    {
        cout << arr[i] << endl;
    }
}

int main(int argc, char *argv[])
{
    pthread_t th;
    int ret;
    int arg = 10;
    int *thread_ret = NULL;
    // 1. 首次创建线程, 互斥锁的状态不为 EBUSY --> 正常创建线程 并将互斥锁的状态变为 EBUSY
    ret = pthread_create(&th, NULL, thread, &arg);
    // 若 ret 不为 1 则进程开始工作
    if (ret != 0)
    {
        printf("Create thread error!\n");
        return -1;
    }
    printf("This is the main process.\n");

    sleep(1);
    // 2. 若在线程中工作尚未完成时再次创建线程, 互斥锁的状态为EBUSY --> 线程直接结束, 此时互斥锁的状态还是EBUSY
    ret = pthread_create(&th, NULL, thread, &arg);
    sleep(2);
    read_state = false;
    printArray(arr);

    pthread_join(th, (void **)&thread_ret);
    printf("thread_ret = %d.\n", *thread_ret);
    return 0;
}