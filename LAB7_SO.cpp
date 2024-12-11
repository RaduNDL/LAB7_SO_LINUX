#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <cstring>

// Variabile globale partajate între procese (utilizând mmap)
bool *lastWasWhite;
sem_t *mutex;
sem_t *whiteQueue;
sem_t *blackQueue;

// Funcția care simulează procesul alb
void runWhiteProcess() {
    std::cout << "Procesul alb rulează...\n";
    sleep(1); // Simulează timp de execuție
}

// Funcția care simulează procesul negru
void runBlackProcess() {
    std::cout << "Procesul negru rulează...\n";
    sleep(1); // Simulează timp de execuție
}

// Funcția de accesare a resursei de către procese albe și negre
void accessResource(const std::string &processType) {
    if (processType == "white") {
        sem_wait(whiteQueue);
        if (!*lastWasWhite) {
            sem_wait(mutex);
        }
        std::cout << "Procesul alb accesează resursa.\n";
        runWhiteProcess();
        std::cout << "Procesul alb a terminat utilizarea resursei.\n";
        *lastWasWhite = true;
        sem_post(mutex);
        sem_post(whiteQueue);
    } else if (processType == "black") {
        sem_wait(blackQueue);
        if (*lastWasWhite) {
            sem_wait(mutex);
        }
        std::cout << "Procesul negru accesează resursa.\n";
        runBlackProcess();
        std::cout << "Procesul negru a terminat utilizarea resursei.\n";
        *lastWasWhite = false;
        sem_post(mutex);
        sem_post(blackQueue);
    }
}

int main() {
    // Alocare memorie partajată pentru variabile globale
    lastWasWhite = static_cast<bool *>(mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    mutex = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    whiteQueue = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    blackQueue = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));

    // Inițializare semafoare
    sem_init(mutex, 1, 1);
    sem_init(whiteQueue, 1, 1);
    sem_init(blackQueue, 1, 1);
    *lastWasWhite = false;

    pid_t pids[4];

    // Crearea proceselor albe și negre
    for (int i = 0; i < 4; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i % 2 == 0) {
                accessResource("white");
            } else {
                accessResource("black");
            }
            _exit(0); // Termină procesul copil
        }
    }

    // Așteaptă toate procesele copil să termine
    for (int i = 0; i < 4; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Distrugere semafoare și eliberare memorie partajată
    sem_destroy(mutex);
    sem_destroy(whiteQueue);
    sem_destroy(blackQueue);
    munmap(lastWasWhite, sizeof(bool));
    munmap(mutex, sizeof(sem_t));
    munmap(whiteQueue, sizeof(sem_t));
    munmap(blackQueue, sizeof(sem_t));

    return 0;
}
