#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <cstring>

// Variabile globale partajate intre procese (utilizand mmap)
bool *lastWasWhite;
sem_t *mutex;
sem_t *whiteQueue;
sem_t *blackQueue;

// Functia care simuleaza procesul alb
void runWhiteProcess() {
    std::cout << "Procesul alb ruleaza...\n";
    sleep(1); // Simuleaza timp de executie
}

// Functia care simuleaza procesul negru
void runBlackProcess() {
    std::cout << "Procesul negru ruleaza...\n";
    sleep(1); // Simuleaza timp de executie
}

// Functia de accesare a resursei de catre procese albe si negre
void accessResource(const std::string &processType) {
    if (processType == "white") {
        sem_wait(whiteQueue);
        if (!*lastWasWhite) {
            sem_wait(mutex);
        }
        std::cout << "Procesul alb acceseaza resursa.\n";
        runWhiteProcess();
        std::cout << "Procesul alb a intrat in vacanta.\n";
        *lastWasWhite = true;
        sem_post(mutex);
        sem_post(whiteQueue);
    } else if (processType == "black") {
        sem_wait(blackQueue);
        if (*lastWasWhite) {
            sem_wait(mutex);
        }
        std::cout << "Procesul negru acceseaza resursa.\n";
        runBlackProcess();
        std::cout << "Procesul negru a intrat in vacanta.\n";
        *lastWasWhite = false;
        sem_post(mutex);
        sem_post(blackQueue);
    }
}

int main() {
    // Alocare memorie partajata pentru variabile globale
    lastWasWhite = static_cast<bool *>(mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    mutex = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    whiteQueue = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    blackQueue = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));

    // Initializare semafoare
    sem_init(mutex, 1, 1);
    sem_init(whiteQueue, 1, 1);
    sem_init(blackQueue, 1, 1);
    *lastWasWhite = false;

    pid_t pids[4];

    // Crearea proceselor albe si negre
    for (int i = 0; i < 4; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i % 2 == 0) {
                accessResource("white");
            } else {
                accessResource("black");
            }
            _exit(0); // Termina procesul copil
        }
    }

    // Asteapta toate procesele copil sa termine
    for (int i = 0; i < 4; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Distrugere semafoare si eliberare memorie partajata
    sem_destroy(mutex);
    sem_destroy(whiteQueue);
    sem_destroy(blackQueue);
    munmap(lastWasWhite, sizeof(bool));
    munmap(mutex, sizeof(sem_t));
    munmap(whiteQueue, sizeof(sem_t));
    munmap(blackQueue, sizeof(sem_t));

    return 0;
}
