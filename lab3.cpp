#include <iostream>
#include <Windows.h>
using namespace std;

int main() {
	int sizeArr, numOfThread;
	cin >> sizeArr;
	int* A = new int[sizeArr];
	for (int i = 0; i < sizeArr; i++) {
		A[i] = 0;
	}
	cin >> numOfThread;

	return 0;
}