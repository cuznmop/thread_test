/*
    程序功能:
    在main函数中设置参数PTHREAD_RUNNING_TIME来改变线程启动的时间;
    线程启动后根据延时PTHREAD_OPEN_AGAIN_TIME的时间, 再次开启线程(此方案用来演示当程序误多次创建线程时出现的情况),
    因为使用了互斥锁, 所以当出现上个线程未结束, 下个线程要启动的情况下, 运行的线程会知道, 并做出相应反应
    eg: printf("thread: the varible is locked by thread. no need open another thread\n");
    在经过READ_STATE_CHANGE_TIME时间后, 修改read_state状态, 关闭线程
    若线程在工作的时候发现read_state状态为false or 延时时间到会结束线程,
    另外: 全局变量有value当他等于某个数值时, 线程会throw error, 这里用来测试线程出错对进程的影响
*/

/*
   测试样例: 
   1.PTHREAD_RUNNING_TIME=2, PTHREAD_OPEN_AGAIN_TIME=4, READ_STATE_CHANGE_TIME=3, 线程会正常启动2次, 因超时结束线程2次
   2.PTHREAD_RUNNING_TIME=2, PTHREAD_OPEN_AGAIN_TIME=4, READ_STATE_CHANGE_TIME=1, 线程会正常启动2次, 因超时结束线程1次, 因read_state状态改变结束线程1次
   3.PTHREAD_RUNNING_TIME=3, PTHREAD_OPEN_AGAIN_TIME=1, READ_STATE_CHANGE_TIME=1, 线程会正常启动1次, 第二次启动会因为互斥锁的原因直接结束尔不进行计数
   4.在1的基础上将ERROR_NUM由666改为66, 线程会出错, 但是不会影响主进程正常工作; 此外, 由于catch(...)接到了线程错误, 使得报误的线程可以正常推出, 并不影响线程的再次启动
     可以根据输出得知线程错误后将互斥锁释放, 使得下次线程还会继续启动
*/


#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

pthread_mutex_t g_mutex; // 互斥锁
using namespace std;

#define PTHREAD_RUNNING_TIME 2
#define PTHREAD_OPEN_AGAIN_TIME 4
#define READ_STATE_CHANGE_TIME 3
#define ERROR_NUM 666


int arr[20] = {};
int value = 0;
int idx = 0;
bool read_state = true;


void printArray(int *arr)
{
    for (int i = 0; i < 19; i++)
    {
        cout << arr[i] << " ";
    }
    cout << arr[19] << endl;
}

void printArrayThread(int *arr)
{
    for (int i = 0; i < 19; i++)
    {
        cout << "*"<< arr[i] << " ";
    }
    cout << "*" << arr[19] << endl;
}

// 参数arg为默认计数时间(单位s)
// 主线程主要功能是对一个数组循环计数, 得到数值
// 线程计数结束后输出计数结果
void *thread(void *arg)
{
    try
    {
        int ret = pthread_mutex_trylock(&g_mutex);  // 得到互斥锁状态
        if (EBUSY == ret)
        {
            printf("thread: the varible is locked by thread. no need open another thread\n");  // 互斥锁变量已经上锁, 不能再上锁
            return arg;
        } 
        else
        {
            printf("thread: lock the variable!\n");  // 将互斥锁变量上锁
        }
        // 启动线程 打印输入的数字, 这里这个数字用来控制延时        
        printf("This is a thread and arg = %d.\n", *(int *)arg);
        
        int timeEnd = time(NULL) + *(int *)arg;
        while (read_state && (time(NULL) < timeEnd))  // 3.1 的情况 + 3.2 的情况
        {
            if (idx >= 20)
            {
                idx = 0;
            }
            usleep(10000);  // 微妙
            arr[idx++] = value++;
            if (value == ERROR_NUM)
            {
                throw "test error";
            }
        }
        cout << "print in thread: " << endl;
        printArrayThread(arr);
    }
    catch (...) // 3.3 的情况
    {
        cout << "get error in thread" << endl;
    }
    // 3. 互斥锁的状态由 非EBUSY->EBUSY 的情况如下:
    // 3.1 主进程将read_state变为false
    // 3.2 记录时间到
    // 3.3 进程工作时出错
    pthread_mutex_unlock(&g_mutex);

    return arg;
}

int main(int argc, char *argv[])
{
    pthread_t th;
    int ret;
    int arg = 2;
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

    sleep(PTHREAD_OPEN_AGAIN_TIME);
    // 2. 若在线程中工作尚未完成时再次创建线程, 互斥锁的状态为EBUSY --> 线程直接结束, 此时互斥锁的状态还是EBUSY
    ret = pthread_create(&th, NULL, thread, &arg);
    sleep(READ_STATE_CHANGE_TIME);
    read_state = false;
    cout << "print in process: " << endl;
    printArray(arr);

    // 然后调用pthread_join()接口与刚才的创建进行合并。
    // 这个接口的第一个参数就是新创建线程的句柄，而第二个参数就会去接受线程的返回值。
    // 线程的合并是一种主动回收线程资源的方案。
    // 当一个进程或线程调用了针对其它线程的pthread_join()接口，就是线程合并。
    // 这个接口会阻塞调用进程或线程，直到被合并的线程结束为止。当被合并线程结束，
    // pthread_join()接口就会回收这个线程的资源，并将这个线程的返回值返回给合并者。
    pthread_join(th, (void **)&thread_ret);
    printf("thread_ret = %d.\n", *thread_ret);
    return 0;
}