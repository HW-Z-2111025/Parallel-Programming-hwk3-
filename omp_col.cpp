#include <iostream>
#include <nmmintrin.h>
#include<math.h>
#include<windows.h>
#include<random>
#include<ctime>
#include<time.h>
#include<omp.h>
#include<fstream>
using namespace std;


int N = 10;
int numThread = 6;
float** matrix;



//ʼ
void m_reset()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            matrix[i][j] = 0;
        }
        matrix[i][i] = 1.0;
        for (int j = i + 1; j < N; j++)
        {
            matrix[i][j] = rand();
        }
    }
    for (int k = 0; k < N; k++)
    {
        for (int i = k + 1; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                matrix[i][j] += matrix[k][j];
            }
        }
    }
}

int main()
{
    matrix = new float* [N];
    for (int iniNo = 0; iniNo < N; iniNo++) {
        matrix[iniNo] = new float[N];
    }

    m_reset();

    long long head, tail, freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);

    float tmp;
    int i, j, k;

#pragma omp parallel num_threads(numThread),private(i,j,k,tmp)
    for (k = 0; k < N; k++)
    {
#pragma omp single
        {
            tmp = matrix[k][k];
            for (j = k + 1; j < N; j++)
                matrix[k][j] = matrix[k][j] / tmp;
            matrix[k][k] = 1.0;
        }

#pragma omp for schedule(static,1)
        for (i = k + 1; i < N; i++)
        {
            for (j = k + 1; j < N; j++)
                matrix[j][i] = matrix[j][i] - matrix[j][k] * matrix[k][i];
        }
#pragma omp single
        for (int i = k + 1; i < N; i++) {
            matrix[i][k] = 0;
        }
    }


    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    cout << "N=" << N << "ʱ" << (tail - head) * 1000.0 / freq << "ms" << endl;

    for (int delNo = 0; delNo < N; delNo++) {
        delete[] matrix[delNo];
    }
    delete[] matrix;

    // system("pause");
    return 0;
}

