#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define OK (0);
#define ARGUMENT_ERROR (1);
#define MIN_CLOCK_ERROR (2);
#define MAX_CLOCK_ERROR (3);
#define CHILD_ERROR (4);
#define MIN_GREATER_THAN_MAX_ERROR (5);
#define FORK_ERROR (6);

int main(int argc, char *argv[])
{
    int i,
        nb_tests,
        min_clock_pid,
        max_clock_pid,
        child_pid,
        pid_first,
        pid_second,
        too_fast_counter = 0,
        too_slow_counter = 0,
        in_time_counter = 0,
        total_time = 0;
    double min_time, max_time;
    time_t t;

    // Se proporcionan al menos los 5 argumentos esperados
    if (argc < 5)
    {
        fprintf(stderr, "🚨 ERROR: Usage: %s [nb_tests] [min_time] [max_time] [program] [arguments?]\n", argv[0]);
        return ARGUMENT_ERROR;
    }

    nb_tests = atoi(argv[1]); // Número de veces a repetir

    for (i = 0; i < nb_tests; i++)
    {
        printf("\nNúmero de test: %d\n",i+1);

        t = time(NULL);

        // Creamos proceso hijo para ejecutar clock del tiempo mínimo
        if ((min_clock_pid = fork()) == 0)
        {
            execlp("./clock", "./clock", argv[2], NULL);
            perror("Min clock execution");
            return MIN_CLOCK_ERROR;
        } else if (min_clock_pid == -1){
            perror("Min clock execution");
            return FORK_ERROR;
        }

        // Creamos proceso hijo para ejecutar clock del tiempo máximo
        if ((max_clock_pid = fork()) == 0)
        {
            execlp("./clock", "./clock", argv[3], NULL);
            perror("Max clock execution");
            return MAX_CLOCK_ERROR;
        } else if (max_clock_pid == -1){
            perror("Min clock execution");
            return FORK_ERROR;
        }

        // Creamos proceso hijo para ejecutar la acción de los argumentos (lo llamaremos proceso del programa)
        if ((child_pid = fork()) == 0)
        {
            execvp(argv[4], &(argv[4]));
            perror("Child execution");
            return CHILD_ERROR;
        } else if (child_pid == -1){
            perror("Min clock execution");
            return FORK_ERROR;
        }

        pid_first = wait(NULL); // Devuelve el id del primer proceso en terminar

        if (pid_first == child_pid)
        {
            printf("El primer proceso en terminar es el del programa\n");
            // El primer proceso en terminar es el hijo
            // El proceso del programa ha tardado menos tiempo del mínimo -> Ha ido demasiado rápido
            too_fast_counter++;

            // "Matamos" el resto de procesos
            kill(min_clock_pid, SIGKILL);
            kill(max_clock_pid, SIGKILL);
            wait(NULL);
        }
        else if (pid_first == min_clock_pid)
        {
            printf("El primer proceso en terminar es el del min clock\n");
            // El primer proceso en terminar es el clock del tiempo mínimo

            // Esperamos al segundo proceso en terminar
            pid_second = wait(NULL);
            if(pid_second == child_pid){
                printf("El segundo proceso en terminar es el del programa\n");
                // El segundo proceso en terminar es el proceso del programa -> Ha ido a tiempo
                in_time_counter++;

                // "Matamos" el resto de procesos
                kill(max_clock_pid, SIGKILL);
                wait(NULL);

            } else if(pid_second == max_clock_pid) {
                printf("El segundo proceso en terminar es el de max clock\n");
                // El segundo proceso en terminar el clock del tiempo máximo
                // El proceso del programa ha tardado más tiempo del máximo -> Ha ido demasiado lento
                too_slow_counter++;
                wait(NULL);
            }

            
        } else if (pid_first == max_clock_pid){
            printf("El primer proceso en terminar es el de max clock\n");
            // El clock para el proceso máximo ha terminado de ejecutarse antes que el mínimo -> El mínimo es mayor al máximo
            perror("The given 'min' argument was greater than 'max'\n");
            return MIN_GREATER_THAN_MAX_ERROR;
        }

        t = time(NULL) - t;
        total_time += t;

    }

    printf("\n🏁 RESULTADOS DE EJECUCIÓN 🏁\n");
    printf("✅ Nº de veces a tiempo:\t\t%d\n", in_time_counter);
    printf("🐌 Nº de veces demasiado rápido:\t%d\n", too_fast_counter);
    printf("🐇 Nº de veces demasiado lento:\t\t%d\n", too_slow_counter);

    printf("\nTotal de tiempo de ejecución: %d segundos\n", total_time);

    return OK;
}
