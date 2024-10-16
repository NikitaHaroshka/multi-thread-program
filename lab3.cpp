#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

// ���������� ����������
volatile int* A = nullptr;
int n;

// ��������� �� ������� ��� ���������� ��������
HANDLE* MustWork;
HANDLE* CantWork;
HANDLE StartAll;

// ��������� ��� �������� ���������� ������
struct MarkerParams {
    int index;
};

// �������, ����������� �������� marker
DWORD WINAPI marker(LPVOID param) {
    MarkerParams* params = (MarkerParams*)param;
    int index = params->index;
    int count = 0;

    // ������������� ��������� ��������� �����
    srand(index);

    // �������� ������� �� ������ ������
    WaitForSingleObject(StartAll, INFINITE);

    // �������� ���� ������ ������
    while (true) {
        // ���������, ��� �� ����� ������ �� ���������
        if (WaitForSingleObject(MustWork[index], 0) == WAIT_OBJECT_0) {
            // ���� ������� ������ �� ����������, �������� ���������� ��������
            for (int j = 0; j < n; j++) {
                if (A[j] == index + 1) {
                    A[j] = 0; // �������� ���������� ��������
                }
            }
            return 0; // ��������� �����
        }

        // ��������� ���������� �������
        int randomIndex = rand() % n;

        // �������� �������� �������
        if (A[randomIndex] == 0) {
            Sleep(5); // �������� 5 ��
            A[randomIndex] = index + 1; // ������ ������ ������
            count++;
            Sleep(5); // �������� 5 ��
        }
        else {
            // ���� ������� �����, ������������� main � ���������������� ������
            cout << "Thread " << index << " cannot mark index " << randomIndex << " after marking " << count << " elements." << endl;
            SetEvent(CantWork[index]); // ������������� ������� ��� main

            // ������� ������� �� ����������� ������
            WaitForSingleObject(MustWork[index], INFINITE);
        }
    }

    return 0;
}

int main() {
    cout << "Enter array size: ";
    cin >> n;

    // ��������� ������ ��� ������
    A = new int[n];
    for (int i = 0; i < n; i++) {
        A[i] = 0; // ������������� ������� ������
    }

    int numOfThreads;
    cout << "Enter number of threads: ";
    cin >> numOfThreads;

    // �������� �������� �������
    MustWork = new HANDLE[numOfThreads];
    CantWork = new HANDLE[numOfThreads];
    HANDLE* threads = new HANDLE[numOfThreads];
    MarkerParams* params = new MarkerParams[numOfThreads];

    StartAll = CreateEvent(NULL, TRUE, FALSE, NULL); // ������� ��� ������ ������

    // �������� �������
    for (int i = 0; i < numOfThreads; i++) {
        MustWork[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        CantWork[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        params[i].index = i;
        threads[i] = CreateThread(NULL, 0, marker, &params[i], 0, NULL);
    }

    // ���� ������ �� ������ ������ ���� �������
    SetEvent(StartAll);

    // �������� ����
    int stoppedThreads = 0;
    while (stoppedThreads < numOfThreads) {
        bool allCantWork = true;
        for (int i = 0; i < numOfThreads; i++) {
            if (WaitForSingleObject(CantWork[i], 0) == WAIT_TIMEOUT) {
                allCantWork = false; // ���� ���� �� ���� ����� ����� ����������
                break;
            }
        }

        if (allCantWork) {
            // ����� ����������� �������
            cout << "Array content: ";
            for (int i = 0; i < n; i++) {
                cout << A[i] << " ";
            }
            cout << endl;

            // ������ �� ���������� ������
            int threadToStop;
            cout << "Enter thread number to stop (0 to " << numOfThreads - 1 << "): ";
            cin >> threadToStop;

            // ������ ������ �� ���������
            SetEvent(MustWork[threadToStop]);
            WaitForSingleObject(threads[threadToStop], INFINITE); // �������� ���������� ������
            stoppedThreads++; // ����������� ���������� ����������� �������

            // ����� ����������� ������� ����� ���������
            cout << "Array content after stopping thread: ";
            for (int i = 0; i < n; i++) {
                cout << A[i] << " ";
            }
            cout << endl;

            // ������������� ���������� ������� ���������� ������
            for (int i = 0; i < numOfThreads; i++) {
                ResetEvent(CantWork[i]);
                SetEvent(MustWork[i]);
            }
        }
    }

    // �������� ������� � ������������ ��������
    for (int i = 0; i < numOfThreads; i++) {
        CloseHandle(threads[i]);
        CloseHandle(CantWork[i]);
        CloseHandle(MustWork[i]);
    }
    CloseHandle(StartAll);
    delete[] A;
    delete[] threads;
    delete[] params;
    delete[] MustWork;
    delete[] CantWork;

    return 0;
}
