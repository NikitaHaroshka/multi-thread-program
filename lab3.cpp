#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

// Глобальные переменные
volatile int* A = nullptr;
int n;

// Указатели на события для управления потоками
HANDLE* MustWork;
HANDLE* CantWork;
HANDLE StartAll;

// Структура для передачи параметров потоку
struct MarkerParams {
    int index;
};

// Функция, выполняемая потоками marker
DWORD WINAPI marker(LPVOID param) {
    MarkerParams* params = (MarkerParams*)param;
    int index = params->index;
    int count = 0;

    // Инициализация генерации случайных чисел
    srand(index);

    // Ожидание сигнала на начало работы
    WaitForSingleObject(StartAll, INFINITE);

    // Основной цикл работы потока
    while (true) {
        // Проверяем, был ли подан сигнал на остановку
        if (WaitForSingleObject(MustWork[index], 0) == WAIT_OBJECT_0) {
            // Если получен сигнал на завершение, обнуляем помеченные элементы
            for (int j = 0; j < n; j++) {
                if (A[j] == index + 1) {
                    A[j] = 0; // Обнуляем помеченные элементы
                }
            }
            return 0; // Завершаем поток
        }

        // Генерация случайного индекса
        int randomIndex = rand() % n;

        // Проверка элемента массива
        if (A[randomIndex] == 0) {
            Sleep(5); // Задержка 5 мс
            A[randomIndex] = index + 1; // Запись номера потока
            count++;
            Sleep(5); // Задержка 5 мс
        }
        else {
            // Если элемент занят, сигнализируем main и приостанавливаем работу
            cout << "Thread " << index << " cannot mark index " << randomIndex << " after marking " << count << " elements." << endl;
            SetEvent(CantWork[index]); // Устанавливаем событие для main

            // Ожидаем сигнала на продолжение работы
            WaitForSingleObject(MustWork[index], INFINITE);
        }
    }

    return 0;
}

int main() {
    cout << "Enter array size: ";
    cin >> n;

    // Выделение памяти под массив
    A = new int[n];
    for (int i = 0; i < n; i++) {
        A[i] = 0; // Инициализация массива нулями
    }

    int numOfThreads;
    cout << "Enter number of threads: ";
    cin >> numOfThreads;

    // Создание массивов событий
    MustWork = new HANDLE[numOfThreads];
    CantWork = new HANDLE[numOfThreads];
    HANDLE* threads = new HANDLE[numOfThreads];
    MarkerParams* params = new MarkerParams[numOfThreads];

    StartAll = CreateEvent(NULL, TRUE, FALSE, NULL); // Событие для начала работы

    // Создание потоков
    for (int i = 0; i < numOfThreads; i++) {
        MustWork[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        CantWork[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        params[i].index = i;
        threads[i] = CreateThread(NULL, 0, marker, &params[i], 0, NULL);
    }

    // Даем сигнал на начало работы всем потокам
    SetEvent(StartAll);

    // Основной цикл
    int stoppedThreads = 0;
    while (stoppedThreads < numOfThreads) {
        bool allCantWork = true;
        for (int i = 0; i < numOfThreads; i++) {
            if (WaitForSingleObject(CantWork[i], 0) == WAIT_TIMEOUT) {
                allCantWork = false; // Если хотя бы один поток может продолжить
                break;
            }
        }

        if (allCantWork) {
            // Вывод содержимого массива
            cout << "Array content: ";
            for (int i = 0; i < n; i++) {
                cout << A[i] << " ";
            }
            cout << endl;

            // Запрос на завершение потока
            int threadToStop;
            cout << "Enter thread number to stop (0 to " << numOfThreads - 1 << "): ";
            cin >> threadToStop;

            // Подать сигнал на остановку
            SetEvent(MustWork[threadToStop]);
            WaitForSingleObject(threads[threadToStop], INFINITE); // Ожидание завершения потока
            stoppedThreads++; // Увеличиваем количество завершенных потоков

            // Вывод содержимого массива после остановки
            cout << "Array content after stopping thread: ";
            for (int i = 0; i < n; i++) {
                cout << A[i] << " ";
            }
            cout << endl;

            // Сигнализируем оставшимся потокам продолжить работу
            for (int i = 0; i < numOfThreads; i++) {
                ResetEvent(CantWork[i]);
                SetEvent(MustWork[i]);
            }
        }
    }

    // Закрытие потоков и освобождение ресурсов
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
