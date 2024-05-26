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
const int numThread = 6;
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

    __m128 vt, va, vaik, vakj, vaij, vx;
    int i, j, k;

#pragma omp parallel num_threads(numThread),private(i,j,k,vt, va, vaik, vakj, vaij, vx)
    for (k = 0; k < N; k++)
    {
#pragma omp single
        {
            vt = _mm_set1_ps(matrix[k][k]);
            for (j = k + 1; j < N; j += 4)
            {
                if (j + 4 > N)
                {
                    for (; j < N; j++)
                    {
                        matrix[k][j] = matrix[k][j] / matrix[k][k];
                    }
                    break;
                }
                va = _mm_loadu_ps(&matrix[k][j]);
                va = _mm_div_ps(va, vt);
                _mm_storeu_ps(&matrix[k][j], va);
            }
            matrix[k][k] = 1.0;
        }
#pragma omp for schedule ( static , 1 )
        for (i = k + 1; i < N; i++)
        {
            vaik = _mm_set1_ps(matrix[i][k]);
            for (j = k + 1; j < N; j += 4)
            {
                if (j + 4 > N)
                {
                    for (; j < N; j++)
                    {
                        matrix[i][j] = matrix[i][j] - matrix[k][j] * matrix[i][k];
                    }
                    break;
                }
                vakj = _mm_loadu_ps(&matrix[k][j]);
                vaij = _mm_loadu_ps(&matrix[i][j]);
                vx = _mm_mul_ps(vakj, vaik);
                vaij = _mm_sub_ps(vaij, vx);
                _mm_storeu_ps(&matrix[i][j], vaij);
            }
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

