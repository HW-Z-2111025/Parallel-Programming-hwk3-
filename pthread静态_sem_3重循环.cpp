#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<windows.h>
using namespace std;

const int N = 10;                       //矩阵大小（数据规模）
double mtx[N][N];                       //矩阵mtx
const int numThread = 4;                //线程数
sem_t sem_leader;
sem_t sem_division[numThread - 1];
sem_t sem_elimination[numThread - 1];


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
    int t_id = p->t_id;

    for (int k = 0; k < N; k++) {
        if (t_id == 0) {
            for (int j = k + 1; j < N; j++) {
                mtx[k][j] = mtx[k][j] / mtx[k][k];
            }
            mtx[k][k] = 1.0;
        }
        else {
            sem_wait(&sem_division[t_id - 1]);
        }

        if (t_id == 0) {
            for (int i = 0; i < numThread - 1; i++) {
                sem_post(&sem_division[i]);
            }
        }

        for (int i = k + 1 + t_id; i < N; i += numThread) {
            for (int j = k + 1; j < N; j++) {
                mtx[i][j] = mtx[i][j] - mtx[i][k] * mtx[k][j];
            }
            mtx[i][k] = 0.0;
        }

        if (t_id == 0) {
            for (int i = 0; i < numThread - 1; i++) {
                sem_wait(&sem_leader);
            }
            for (int i = 0; i < numThread - 1; i++) {
                sem_post(&sem_elimination[i]);
            }
        }
        else {
            sem_post(&sem_leader);
            sem_wait(&sem_elimination[t_id - 1]);
        }
    }
    return NULL;
}


int main() {

    mtxReset();

    long long head, tail, freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);

    sem_init(&sem_leader, 0, 0);
    for (int i = 0; i < numThread - 1; i++) {
        sem_init(&sem_division[i], 0, 0);
        sem_init(&sem_elimination[i], 0, 0);
    }

    pthread_t thread_handles[numThread];
    threadParam_t param[numThread];
    for (int t_id = 0; t_id < numThread; t_id++) {
        param[t_id].t_id = t_id;
        pthread_create(&thread_handles[t_id], NULL, threadFunc, (void*)&param[t_id]);
    }

    for (int t_id = 0; t_id < numThread; t_id++) {
        pthread_join(thread_handles[t_id], NULL);
    }

    sem_destroy(&sem_leader);
    for (int i = 0; i < numThread - 1; i++) {
        sem_destroy(&sem_division[i]);
        sem_destroy(&sem_elimination[i]);
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&tail);

    cout << "N=" << N << "，耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;

    // system("pause");
    return 0;

}
