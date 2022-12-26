
/* Exemple 2 de test du module */

// Online C compiler to run C program online
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

/* Création d'un ou ensemble de sémaphore */

int CreerSem(int key, int nb){
    // key : le clé 
    // nb : nombre de semaphore dans l'ensemble
    return semget(key, nb, IPC_CREAT|0666);
}

/* Initialisation d'un sémaphore */

int InitSem(int sem, int mutex, int valeur){
    // sem : ID de l'ensemble de sémaphore
    // mutex : numéro de sémaphore
    // valeur : la valeur de sémphore
    union semun { 
        int val; 
        struct semid_ds *buf; 
        unsigned short *array;
    } sb;
    sb.val = valeur; // la valeur de semaphore
    if(mutex>=0){
        /* si le numéro de sémaphore > 0, 
           ie : on précise le sémaphore */
        return semctl(sem, mutex, SETVAL, sb);
    }
    return -1;
}

/* La primitive P */

int P(int sem, int mutex){
    struct sembuf Psem = {mutex, -1, 0};
    return semop(sem, &Psem, 1);
}

/* La primitive V */

int V(int sem, int mutex){
    struct sembuf Vsem = {mutex, 1, 0};
    return semop(sem, &Vsem, 1);
}

/* Destruction */

int SupprSem(int sem, int mutex){
    return semctl(sem, mutex, IPC_RMID, 0);
}

int CreerShm(int key, int size){
    // key : le clé 
    // size : la taille du mémoire en octet
    return shmget(key, size, IPC_CREAT|0666);
}

void* Attacher(int shm, _Bool rom){
    if(rom) return shmat(shm, (void*)0, SHM_RDONLY);
    else return shmat(shm, (void*)0, 0);
}

int Dettacher(void* shmadr){
    return shmdt(shmadr);
}
int Liberer(int shm){
    return shmctl(shm, IPC_RMID, 0);
}

#define N 5

#define mutex 0
#define nlibre 1

int shmMdispo;
int sem;

int main() {
    // Write C code here
    int key;
    key = ftok("./main.c", 1);
    shmMdispo = CreerShm(key,N*sizeof(_Bool));
    // shm : l'identifiant de l'ensemble de sémaphore
    if(shmMdispo == -1){
        printf("la creation est echouée\n");
        return -1;
    } else {
        printf("shm : %d / la mémoire est allouée\n",shmMdispo);
    }
    
    _Bool* Mdispo = (_Bool*)Attacher(shmMdispo,0);
    if(Mdispo == NULL) { 
        printf("Erreur"); 
        return -2;
    }
    for(int i=0; i < N; i++) Mdispo[i] = 0; // Mdispo[0] = faux;
    int ctl = Dettacher(Mdispo);
    
    // Read Only
    Mdispo = (_Bool*)Attacher(shmMdispo,1);
    for(int i=0; i < N; i++) printf("Mdispo[%d] = %d\n",i,Mdispo[i]);
    ctl = Dettacher(Mdispo);
    
    /* Semaphore */
    
    key = ftok("./main.c", 12);
    
    // Creation
    sem = CreerSem(key, 2);
    
    // Initialisation
    InitSem(sem, mutex, 1); // Sémaphore mutex init(1);
    InitSem(sem, nlibre, N); // Sémaphore mutex init(N);
    
    /* Programme */
    int p;
    for(int j =0; j < 10; j++){
        p = fork();
        if(p == -1){
            printf("les processus 'P%d' n'est pas crée\n",j);
            exit(41);
        }    
        
        if(p > 0) printf("les processus 'P%d' est crée, PID : %d\n", j, p);
        
        if(p == 0) {
            printf("Debut de P%d\n",j);
            execl("./Prgm_client","./Prgm_client", NULL);
            printf("Fin de P%d\n",j);
            exit(51);
        }
    }
    
    /* Attente de processus */
    while((p = wait(NULL)) != -1) {
        printf("le processus de PID : %d est fini\n",p);
    }
    
    // Liberer
    SupprSem(sem, 2);
    Liberer(shmMdispo);
    
    printf("Fin de Programme\n");
    return 0;
}
