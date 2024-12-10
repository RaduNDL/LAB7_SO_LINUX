#include <iostream>
#include <thread>
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>

class ProcessAccess {
private:
    static HANDLE mutex;       // Controleaza accesul exclusiv
    static HANDLE turnstile;   // Controleaza alternanta intre grupuri
    static int whiteCount;     // Numarul de fire albe care acceseaza resursa
    static int blackCount;     // Numarul de fire negre care acceseaza resursa
    static bool lastWasWhite;  // Indica ultimul grup care a folosit resursa
    static std::mutex outputMutex; // Protejeaza output-ul pentru a evita intercalarea

    // Simuleaza rularea unui fir alb
    static void runWhiteProcess() {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "Firul alb acceseaza resursa." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Firul alb a terminat utilizarea resursei." << std::endl;
    }

    // Simuleaza rularea unui fir negru
    static void runBlackProcess() {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "Firul negru acceseaza resursa." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Firul negru a terminat utilizarea resursei." << std::endl;
    }

public:
    static void accessResource(const std::string& threadType) {
        if (threadType == "white") {
            WaitForSingleObject(turnstile, INFINITE); // Controleaza alternanta
            WaitForSingleObject(mutex, INFINITE);
            whiteCount++;
            if (blackCount > 0 && lastWasWhite) {
                ReleaseSemaphore(turnstile, 1, NULL); // Permite accesul firelor negre
            }
            else {
                ReleaseSemaphore(turnstile, 1, NULL); // Continua cu firele albe
            }
            ReleaseMutex(mutex);

            runWhiteProcess();

            WaitForSingleObject(mutex, INFINITE);
            whiteCount--;
            if (whiteCount == 0) lastWasWhite = true;
            ReleaseMutex(mutex);
        }
        else if (threadType == "black") {
            WaitForSingleObject(turnstile, INFINITE); // Controleaza alternanta
            WaitForSingleObject(mutex, INFINITE);
            blackCount++;
            if (whiteCount > 0 && !lastWasWhite) {
                ReleaseSemaphore(turnstile, 1, NULL); // Permite accesul firelor albe
            }
            else {
                ReleaseSemaphore(turnstile, 1, NULL); // Continua cu firele negre
            }
            ReleaseMutex(mutex);

            runBlackProcess();

            WaitForSingleObject(mutex, INFINITE);
            blackCount--;
            if (blackCount == 0) lastWasWhite = false;
            ReleaseMutex(mutex);
        }
    }

    static void initialize() {
        mutex = CreateMutex(NULL, FALSE, NULL);
        turnstile = CreateSemaphore(NULL, 1, 1, NULL); // Porneste alternanta
        whiteCount = 0;
        blackCount = 0;
        lastWasWhite = false;
    }

    static void cleanup() {
        CloseHandle(mutex);
        CloseHandle(turnstile);
    }
};

// Definirea variabilelor statice
HANDLE ProcessAccess::mutex;
HANDLE ProcessAccess::turnstile;
int ProcessAccess::whiteCount = 0;
int ProcessAccess::blackCount = 0;
bool ProcessAccess::lastWasWhite = false;
std::mutex ProcessAccess::outputMutex;

int main() {
    ProcessAccess::initialize();

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([]() { ProcessAccess::accessResource("white"); });
        threads.emplace_back([]() { ProcessAccess::accessResource("black"); });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    ProcessAccess::cleanup();

    return 0;
}
    