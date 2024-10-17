#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;

volatile int* A = nullptr;
int arraySize, threadToCloseInThread;
HANDLE Ready, ReportOfClose, Again;
HANDLE* CantWork, Threads;
HANDLE StopWork;
volatile int countOfCant = 0; //Переменная счетчик потоков, которые подали сигнал о желании завершить
CRITICAL_SECTION cs;  // Критическая секция

DWORD WINAPI marker(LPVOID param) {
    WaitForSingleObject(Ready, INFINITE);
    int index = (int)param;
    int count = 0;
    srand(index + 1);
    int funny = 5;

    while (true) {
        int randNumber = rand() % arraySize;
        EnterCriticalSection(&cs);

        if (A[randNumber] == 0) {
            Sleep(5);
            A[randNumber] = index + 1;
            count++;
            Sleep(5);
            LeaveCriticalSection(&cs);

        }
        else {
            printf("Thread number %d marked %d elements and stopped on %d.\n", index+1,count,randNumber+1);
            count = 0;
            LeaveCriticalSection(&cs);
            SetEvent(CantWork[index]);
            WaitForSingleObject(StopWork, INFINITE);

            if (threadToCloseInThread == index + 1) {
                for (int i = 0; i < arraySize; i++) {
                    if (A[i] == index + 1) {
                        A[i] = 0;
                    }
                }
                ResetEvent(StopWork);
                goto finish;
            }
            else {
                ResetEvent(CantWork[index]);
                WaitForSingleObject(Again, INFINITE);
            }
        }
        funny += 20;
    }
finish:
    return 0;
}

int main() {
    InitializeCriticalSection(&cs);  // Инициализация критической секции
    cout << "Enter array size: ";
    cin >> arraySize;
    A = new int[arraySize];
    for (int i = 0; i < arraySize; i++) {
        A[i] = 0;
    }

    int numOfThreads;
    cout << "Enter number of threads: ";
    cin >> numOfThreads;

    HANDLE* Threads = new HANDLE[numOfThreads];
    CantWork = new HANDLE[numOfThreads];
    Again = CreateEvent(NULL, TRUE, FALSE, NULL);
    ReportOfClose = CreateEvent(NULL, TRUE, FALSE, NULL);
    Ready = CreateEvent(NULL, TRUE, FALSE, NULL);
    StopWork = CreateEvent(NULL, TRUE, FALSE, NULL);  // Событие для остановки потоков

    int* threadToEnd = new int[numOfThreads];  // Номера завершенных потоков
    for (int i = 0; i < numOfThreads; i++) {
        threadToEnd[i] = 0;
    }

    for (int i = 0; i < numOfThreads; i++) {
        Threads[i] = CreateThread(NULL, 0, marker, (void*)(i), 0, NULL);
        CantWork[i] = CreateEvent(NULL, TRUE, FALSE, NULL);  // Событие, которое подает поток
    }

    SetEvent(Ready);

    int workingThreads = numOfThreads;
    int countOfFinished = 0;

    while (countOfCant < numOfThreads) {
        WaitForMultipleObjects(workingThreads, CantWork, TRUE, INFINITE);

        EnterCriticalSection(&cs);
        cout << endl << "Array: ";
        for (int i = 0; i < arraySize; i++) {
            cout << A[i] << " ";
        }
        cout << endl << "Ended threads: ";
        for (int i = 0; i < countOfFinished; i++) {
            cout << threadToEnd[i] << " ";
        }
        if (countOfFinished == 0) cout << " NO.";
        cout << endl << "Enter number of thread to end: ";

        cin >> threadToEnd[countOfFinished];
        LeaveCriticalSection(&cs);

        threadToCloseInThread = threadToEnd[countOfFinished];
        SetEvent(StopWork);

        WaitForSingleObject(Threads[threadToCloseInThread - 1], INFINITE);
        EnterCriticalSection(&cs);
        cout << "Array: ";
        for (int i = 0; i < arraySize; i++) {
            cout << A[i] << " ";
        }
        cout << endl;
        LeaveCriticalSection(&cs);

        countOfCant++;
        workingThreads--;
        countOfFinished++;

        PulseEvent(Again);
    }

    for (int i = 0; i < numOfThreads; i++) {
        CloseHandle(CantWork[i]);
        CloseHandle(Threads[i]);
    }
    CloseHandle(Again);
    CloseHandle(ReportOfClose);
    CloseHandle(Ready);
    CloseHandle(StopWork);
    DeleteCriticalSection(&cs);  // Удаление критической секции
    delete[] A;
    delete[] Threads;

    return 0;
}
