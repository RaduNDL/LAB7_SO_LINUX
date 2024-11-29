#include <iostream>
#include <thread>
#include <semaphore.h>
#include <string>
#include <unistd.h>

// Semafoare globale pentru sincronizare
sem_t mutex;
sem_t whiteQueue;
sem_t blackQueue;
bool lastWasWhite = false;

// Funcția care simulează procesul alb
void runWhiteProcess() {
    std::cout << "Procesul alb rulează...\n";
    sleep(1);  // Simulează timp de execuție
}

// Funcția care simulează procesul negru
void runBlackProcess() {
    std::cout << "Procesul negru rulează...\n";
    sleep(1);  // Simulează timp de execuție
}

// Funcția de accesare a resursei de către fire albe și negre
void accessResource(const std::string &threadType) {
    if (threadType == "white") {
        sem_wait(&whiteQueue);
        if (!lastWasWhite) {
            sem_wait(&mutex);
        }
        std::cout << "Firul alb accesează resursa.\n";
        runWhiteProcess();  // Simulează rularea procesului alb
        std::cout << "Firul alb a terminat utilizarea resursei.\n";
        lastWasWhite = true;
        sem_post(&mutex);
        sem_post(&whiteQueue);
    } else if (threadType == "black") {
        sem_wait(&blackQueue);
        if (lastWasWhite) {
            sem_wait(&mutex);
        }
        std::cout << "Firul negru accesează resursa.\n";
        runBlackProcess();  // Simulează rularea procesului negru
        std::cout << "Firul negru a terminat utilizarea resursei.\n";
        lastWasWhite = false;
        sem_post(&mutex);
        sem_post(&blackQueue);
    }
}

int main() {
    // Inițializare semafoare
    sem_init(&mutex, 0, 1);
    sem_init(&whiteQueue, 0, 1);
    sem_init(&blackQueue, 0, 1);

    // Crearea firelor de execuție
    std::thread whiteThread1([] { accessResource("white"); });
    std::thread whiteThread2([] { accessResource("white"); });
    std::thread blackThread1([] { accessResource("black"); });
    std::thread blackThread2([] { accessResource("black"); });

    // Așteptarea firelor
    whiteThread1.join();
    whiteThread2.join();
    blackThread1.join();
    blackThread2.join();

    // Distrugerea semafoarelor
    sem_destroy(&mutex);
    sem_destroy(&whiteQueue);
    sem_destroy(&blackQueue);

    return 0;
}

