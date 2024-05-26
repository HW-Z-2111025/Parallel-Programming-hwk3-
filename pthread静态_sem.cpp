#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<windows.h>
using namespace std;

const int N = 10;                       //矩阵大小（数据规模）
double mtx[N][N];                       //矩阵mtx
const int numThread = 4;                //线程数
sem_t sem_main;                         //主线程的信号量
sem_t sem_workerstart[numThread];       //每个工作线程的start信号量
sem_t sem_workerend[numThread];         //每个工作线程的end信号量


typedef struct {
    int t_id;   //线程id
}threadParam_t;

//初始化矩阵
void mtxReset()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            mtx[i][j] = 0;
        }
        mtx[i][i] = 1.0;
        for (int j = i + 1; j < N; j++)
        {
            mtx[i][j] = rand();
        }
    }
    for (int k = 0; k < N; k++)
    {
        for (int i = k + 1; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                mtx[i][j] += mtx[k][j];
            }
        }
    }
}


void* threadFunc(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int id = p->t_id;   //线程id

    for (int k = 0; k < N; k++) {//遍历所有轮
        sem_wait(&sem_workerstart[id]);

        //进行消去操作
        for (int i = k + 1 + id; i < N; i += numThread) {
            for (int j = k + 1; j < N; j++) {
                mtx[i][j] = mtx[i][j] - mtx[i][k] * mtx[k][j];
            }
            mtx[i][k] = 0.0;
        }

        sem_post(&sem_main);
        sem_wait(&sem_workerend[id]);
    }

    return NULL;
}


int main() {

    mtxReset();

    long long head, tail, freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);


    //信号量初始化
    sem_init(&sem_main, 0, 0);
    for (int i = 0; i < numThread; i++) {
        sem_init(&sem_workerstart[i], 0, 0);
        sem_init(&sem_workerend[i], 0, 0);
    }

    //创建线程
    pthread_t thread_handles[numThread];
    threadParam_t param[numThread];
    for (int i = 0; i < numThread; i++) {
        param[i].t_id = i;
        pthread_create(&thread_handles[i], NULL, threadFunc, (void*)&param[i]);
    }

    for (int k = 0; k < N; k++) {
        //主线程：进行除法操作
        for (int j = k + 1; j < N; j++) {
            mtx[k][j] = mtx[k][j] / mtx[k][k];
        }
        mtx[k][k] = 1.0;

        //唤醒start，睡眠（等待工作线程执行完），唤醒end（让工作线程准备遍历下一轮）
        for (int i = 0; i < numThread; i++) {
            sem_post(&sem_workerstart[i]);
        }
        for (int i = 0; i < numThread; i++) {
            sem_wait(&sem_main);
        }
        for (int i = 0; i < numThread; i++) {
            sem_post(&sem_workerend[i]);
        }
    }

    //主线程阻塞，等待所有工作线程执行完
    for (int i = 0; i < numThread; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    //销毁信号量
    for (int i = 0; i < numThread; i++) {
        sem_destroy(&sem_workerend[i]);
        sem_destroy(&sem_workerstart[i]);
    }
    sem_destroy(&sem_main);

    QueryPerformanceCounter((LARGE_INTEGER*)&tail);

    cout << "N=" << N << "，耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;

    // system("pause");
    return 0;

}
