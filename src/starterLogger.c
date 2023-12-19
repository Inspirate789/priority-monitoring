#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>

#define LOG_SIZE 8192
#define LOGS_COUNT 5
#define DELAY_S 10

int main(int argc, char *argv[])
{
    FILE *filePtr = NULL;
    char log[LOG_SIZE] = {'\0'};

    system("sudo insmod taskInfoGetter.ko");
    
    for (int i = 0; i < LOGS_COUNT; i++)
    {
        filePtr = popen("cat /proc/processAnalyzer", "r");

        if (filePtr == NULL)
        {
            printf("Error: can't execute cat for process analyzer");
            return 1;
        }

        while (fgets(log, sizeof(log), filePtr) != NULL)
            printf("%s", log);

        pclose(filePtr);
        sleep(DELAY_S);
    }

    system("sudo rmmod taskInfoGetter");

    return EXIT_SUCCESS;
}
