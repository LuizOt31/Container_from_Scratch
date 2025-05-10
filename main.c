#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>     // mkdir
#include <sys/mount.h>    // mount
#include <unistd.h>       // chroot, chdir
#include <stdio.h>        // perror
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>     // mmap
#include <semaphore.h>    // sem_t
#define STACK_SIZE (1024 * 1024)

sem_t *sem;

int hostname_id = 100000;

void die(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int child_main(void *arg) {

    printf("Filho esperando rede...\n");
    sem_wait(sem); // Espera o pai liberar o semáforo

    printf("Rede pronta! Continuando...\n");

    /* mounting the new container filesystem */

    char path[256];
    const char *new_root = "/tmp/ns_root";

    // every namespace with low priority will not see our file changes
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        perror("mount make private");
        return 1;
    }

    mkdir(new_root, 0755);
    // mounts our new root file (the processess in this namespace will see "/tmp/ns_root" as "/")
    mount(NULL, new_root, "tmpfs", MS_REC, "");
    // creating our system files (bin, tmp, proc, etc.)
    system("tar xvf alpine-minirootfs-3.13.1-x86_64.tar.gz -C /tmp/ns_root/ &");
    system("cp ./prog /tmp/ns_root");
    sleep(1);  //////// USAR SEMAFARO OU BARREIRA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    /* PID Namespace */

    printf("PID no container: %d\n", getpid());

    /* UTS Namespace */
    char host_name[6];
    // itoa(hostname_id, host_name, 6);
    sethostname(host_name, 6);

    if (access("/bin/sh", X_OK) == -1) {
        perror("sh não está acessível");
    }

    chdir(new_root);
    chroot(".");

    printf("What do you want know?\nType 1 to run a simplified shell\nType 2 to run a basic program\n");
    int aux;
    scanf("%d", &aux);
    if (aux == 1){

        execl("/bin/sh", "sh", (char *)NULL);
        die("execl");

    } else if (aux == 2) {

        char *const args[] = {"./prog", "12", NULL };
        if (execve("./prog", args, NULL) == -1) {
            perror("execve");
        }

    }
    return 1;
}

int main() {

    sem = mmap(NULL, sizeof(sem_t),
               PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS,
               -1, 0);

    if (sem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if (sem_init(sem, 1, 0) == -1) {
        perror("sem_init");
        exit(1);
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        exit(1);
    }

    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD;
    pid_t pid = clone(child_main, stack + STACK_SIZE, flags, NULL);
    if (pid == -1) {
        perror("clone");
        exit(1);
    }

    char sh_command[30];
    if (sprintf(sh_command, "sh ip_container.sh %d", pid) < 0){
        perror("sprintf");
        exit(1);
    }

    system(sh_command);

    // Após configurar, sinaliza o semáforo para liberar o filho
    sem_post(sem);

    waitpid(pid, NULL, 0);

    // Finaliza o semáforo e a memória mapeada
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));

    return 0;
}

