#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
// #include <sys/time.h>

using namespace std;

int arr[20] = {};
int value = 0;
int idx=0;
bool read_state = true;


void *thread(void *arg)
{
    try
    {
        printf("This is a thread and arg = %d.\n", *(int *)arg);
        *(int *)arg = 0;
        while(read_state)
        {
            if(idx>=20)
            {
                idx = 0;
            }
            usleep(90000);
            arr[idx++] = value++;
            if (value == 8)
            {
                throw "test error";
            }
        }        
    }
    catch(...)
    {
        cout << "get error" << endl;
    }
    
    return arg;
}

void printArray(int*arr)
{
    for (int i=0; i<20; i++)
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
    ret = pthread_create(&th, NULL, thread, &arg);
    // 若 ret 不为 1 则进程开始工作
    if (ret != 0)
    {
        printf("Create thread error!\n");
        return -1;
    }
    printf("This is the main process.\n");

    sleep(2);

    read_state=false;

    pthread_join(th, (void **)&thread_ret);
    printf("thread_ret = %d.\n", *thread_ret);

    printArray(arr);

    return 0;
}