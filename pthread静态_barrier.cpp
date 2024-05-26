#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<windows.h>
using namespace std;

const int N = 10;                       //矩阵大小（数据规模）
double mtx[N][N];                       //矩阵mtx
const int numThread = 4;                //线程数
pthread_barrier_t barrier_division;
pthread_barrier_t barrier_elimination;


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

        pthread_barrier_wait(&barrier_division);

        for (int i = k + 1 + t_id; i < N; i += numThread) {
            for (int j = k + 1; j < N; j++) {
                mtx[i][j] = mtx[i][j] - mtx[i][k] * mtx[k][j];
            }
            mtx[i][k] = 0.0;
        }

        pthread_barrier_wait(&barrier_elimination);
    }
    return NULL;
}


int main() {

    mtxReset();

    long long head, tail, freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);


    pthread_barrier_init(&barrier_division, NULL, numThread);
    pthread_barrier_init(&barrier_elimination, NULL, numThread);

    pthread_t thread_handles[numThread];
    threadParam_t param[numThread];
    for (int t_id = 0; t_id < numThread; t_id++) {
        param[t_id].t_id = t_id;
        pthread_create(&thread_handles[t_id], NULL, threadFunc, (void*)&param[t_id]);
    }

    for (int t_id = 0; t_id < numThread; t_id++) {
        pthread_join(thread_handles[t_id], NULL);
    }

    pthread_barrier_destroy(&barrier_division);
    pthread_barrier_destroy(&barrier_elimination);

    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    cout << "N=" << N << "，耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;;

    // system("pause");
    return 0;

}
