#include<iostream>
#include<pthread.h>
#include<windows.h>
using namespace std;

const int N = 10;   //矩阵大小（数据规模）
double mtx[N][N];   //矩阵mtx


typedef struct {
    int k;      //消去的轮次
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
    int k = p->k;       //消去的轮次
    int id = p->t_id;   //线程id
    int i = k + id + 1;

    //进行消去操作
    for (int j = k + 1; j < N; j++) {
        mtx[i][j] = mtx[i][j] - (mtx[i][k] * mtx[k][j]);
    }
    mtx[i][k] = 0;

    return NULL;
}


int main() {

    mtxReset();

    long long head, tail, freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);


    for (int k = 0; k < N; k++) {
        //主线程：进行除法操作
        for (int j = k + 1; j < N; j++) {
            mtx[k][j] = mtx[k][j] / mtx[k][k];
        }
        mtx[k][k] = 1.0;

        //工作线程：进行消去操作
        int numThread = N - 1 - k;                                  //所需要创建的线程数
        pthread_t* thread_handles = new pthread_t[numThread];   //每个线程的句柄
        threadParam_t* param = new threadParam_t[numThread];    //每个线程的参数
        for (int i = 0; i < numThread; i++) {                         //设置参数中的k和t_id
            param[i].k = k;
            param[i].t_id = i;
        }
        for (int i = 0; i < numThread; i++) {                           //创建线程
            pthread_create(&thread_handles[i], NULL, threadFunc, (void*)&param[i]);
        }
        for (int i = 0; i < numThread; i++) {                           //主线程挂起等待所有工作线程执行完
            pthread_join(thread_handles[i], NULL);
        }

    }


    QueryPerformanceCounter((LARGE_INTEGER*)&tail);

    cout << "N=" << N << "，耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;

    // system("pause");
    return 0;

}
