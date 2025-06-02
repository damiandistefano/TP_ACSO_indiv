#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define ERROR_EXIT(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int c = atoi(argv[2]);
    int s = atoi(argv[3]);

    if (n < 3 || s < 0 || s >= n) {
        fprintf(stderr, "Error: n >= 3 y 0 <= s < n\n");
        exit(EXIT_FAILURE);
    }

    int pipes[n][2];
    int padre_pipe[2];

    for (int i = 0; i < n; ++i)
        if (pipe(pipes[i]) == -1)
            ERROR_EXIT("pipe");

    if (pipe(padre_pipe) == -1)
        ERROR_EXIT("pipe padre");

    printf("Se crearán %d procesos, se enviará el caracter %d desde proceso %d\n", n, c, s);

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0)
            ERROR_EXIT("fork");

        if (pid == 0) {
            // Cierro pipes que no uso
            for (int j = 0; j < n; ++j) {
                if (j != i) close(pipes[j][0]);
                if (j != (i + 1) % n) close(pipes[j][1]);
            }
            close(padre_pipe[0]);

            int num;

            if (read(pipes[i][0], &num, sizeof(int)) != sizeof(int))
                ERROR_EXIT("read hijo");

            printf("Hijo %d recibe %d\n", i, num);

            num++;

            if ((i + 1) % n == s) {
                if (write(padre_pipe[1], &num, sizeof(int)) != sizeof(int))
                    ERROR_EXIT("write a padre");
            } else {
                if (write(pipes[(i + 1) % n][1], &num, sizeof(int)) != sizeof(int))
                    ERROR_EXIT("write a siguiente");
            }

            close(pipes[i][0]);
            close(pipes[(i + 1) % n][1]);
            close(padre_pipe[1]);
            exit(EXIT_SUCCESS);
        }
    }

    // Padre
    for (int i = 0; i < n; ++i) {
        close(pipes[i][0]);
        if (i != s) close(pipes[i][1]);
    }
    close(padre_pipe[1]);

    if (write(pipes[s][1], &c, sizeof(int)) != sizeof(int))
        ERROR_EXIT("write inicial padre");

    int result;
    if (read(padre_pipe[0], &result, sizeof(int)) != sizeof(int))
        ERROR_EXIT("read padre");

    printf("Resultado final: %d\n", result);
    close(padre_pipe[0]);

    for (int i = 0; i < n; ++i)
        wait(NULL);

    return 0;
}

