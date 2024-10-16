#include <windows.h>
#include <iostream>
using namespace std;
HANDLE hSemaphore;
volatile int kek = 0;

DWORD WINAPI marker(LPVOID Param) {
	int index = *(int*)Param;
	WaitForSingleObject(hSemaphore, INFINITE);
	//cout << endl << index;
	//Sleep(100);
	kek++;
	cout << "Thread " << index << ": kek = " << kek << endl;

	ReleaseSemaphore(hSemaphore, 1, NULL);
	//cout << kek;
	return 0;
}

int main() {
	int k, numOfThreads;
	cin >> k;
	volatile int n=k;
	volatile int* A = new int[n];
	for (int i = 0; i < n; i++) {
		A[i] = 0;
	}
	cin >> numOfThreads;
	HANDLE* hThread = new HANDLE[numOfThreads];
	int* Indexs = new int[numOfThreads];

	//Создание семафора
	hSemaphore = CreateSemaphore(NULL, 1, 1, NULL);


	//Создание потоков
	for (int i = 0; i < numOfThreads; i++) {
		Indexs[i] = i;
		hThread[i] = CreateThread(NULL,0,marker,(void*)&Indexs[i], 0, NULL);
		if (hThread[i] == NULL) {
			return GetLastError();
			return 1;
		}

	}
	
	WaitForMultipleObjects(numOfThreads, hThread,TRUE,INFINITE);
	CloseHandle(hSemaphore);
	//Закрытие потоков
	for (int i = 0; i < numOfThreads; i++) {
		CloseHandle(hThread[i]);
	}
	delete[] A;
	
	return 0;
}