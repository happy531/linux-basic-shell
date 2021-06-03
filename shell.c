/* COMPILE IN TERMINAL: gcc shell.c -o shell -lreadline -lncurses */
/* GITHUB: github.com/happy531 */

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#define BUFF_SIZE 1024
#define TRUE 1
#define FALSE -1
#define BLUE "\001\x1b[34m\002"
#define YELLOW "\001\x1b[33m\002"
#define WHITE "\001\x1b[0m\002"
#define BOLD "\001\e[1m\002"
#define GREEN "\e[0;32m"

void help(char *argv1)
{
    if (argv1 != NULL)
        printf("Failure: unknown command, did u mean \"help\"?\n");
    else
    {
        printf(GREEN BOLD "\nAuthor: " WHITE "Jędrzej Wesołowski\n");
        printf(GREEN BOLD "My implemented commands: " WHITE "help, cd, exit, mkdir, rm, rmdir and others with fork and execvp use\n");
        printf(GREEN BOLD "Addons: " WHITE "command history, tab completion for my commands, signal ctrl-c\n");
        printf(GREEN BOLD "mkdir" WHITE " and " GREEN BOLD "rm " WHITE "works the same way like in Linux shell. Try this: \n");
        printf(WHITE "mkdir one two\nmkdir -p one/two/three\nmkdir -p one/{two,three,four}\nmkdir \"My garden\"\n");
        printf(WHITE "rm one\nrm -i one\nrm -r one\nrmdir one two\nAll of this works with path too, example: rm -r Desktop/microshell/one\n\n");
    }
}

void exitShell(char *argv1)
{
    if (argv1 != NULL)
        printf("Failure: unknown command, did u mean \"exit\"?\n");
    else
        exit(EXIT_SUCCESS);
}

void cd(char *cdArg)
{
    char *homePath = getenv("HOME");
    if (cdArg == NULL)
        printf("Failure: no given directory\n");
    else if (strcmp(cdArg, ".") == 0)
        printf("Failure: unknown command, did u mean \"cd ..\"?\n");
    else if (strcmp(cdArg, "-") == 0)
    {
    }
    else if (strcmp(cdArg, "~") == 0)
        chdir(homePath);

    else if (chdir(cdArg) == FALSE)
        printf("Failure: no such file directory\n");
}

void rmFunctionRecursive(char *argv2)
{
    DIR *mainFolder = opendir(argv2);
    struct dirent *foldersInside;

    while ((foldersInside = readdir(mainFolder)) != NULL)
    {
        if (strcmp(foldersInside->d_name, ".") == 0 || strcmp(foldersInside->d_name, "..") == 0)
            continue;

        char *argv2Copy = malloc(BUFF_SIZE);
        sprintf(argv2Copy, "%s/%s", argv2, foldersInside->d_name);

        struct stat fileType;
        stat(argv2Copy, &fileType);

        if (S_ISDIR(fileType.st_mode))
        {
            strcat(argv2Copy, "/");
            if (rmdir(argv2Copy) == FALSE)
                rmFunctionRecursive(argv2Copy);
        }
        else
            remove(argv2Copy);

        free(argv2Copy);
    }

    remove(argv2);
}

void signalDetecting(int signalName)
{
    if (signalName == SIGINT)
    {
        printf("\nSignal ctrl-c detected, executing...\n");
        exit(EXIT_SUCCESS);
    }
}

char **character_name_completion(const char *, int, int);
char *character_name_generator(const char *, int);

char *character_names[] = {"cd", "exit", "rm", "rmdir", "mkdir", "help", NULL};

int main()
{
    system("clear");
    printf("Type \"help\" to see functionalities\n");
    rl_attempted_completion_function = character_name_completion;
    char *input = malloc(BUFF_SIZE);

    while (TRUE)
    {
        signal(SIGINT, signalDetecting);
        while (TRUE)
        {
            char *login = getlogin();
            char currentPath[BUFF_SIZE];
            getcwd(currentPath, sizeof(currentPath));
            char *path = malloc(BUFF_SIZE);
            sprintf(path, BLUE BOLD "%s" WHITE ":" YELLOW BOLD "~%s" WHITE "$ ", login, currentPath);

            input = readline(path);

            if (strcmp(input, "") == 0) /*gdy nie na inputu*/
            {
                break;
            }
            else
            {
                add_history(input);
                char *argv[BUFF_SIZE / 2];
                argv[0] = strtok(input, " ");
                int argc = 0;
                while (argv[argc] != NULL)
                    argv[++argc] = strtok(NULL, " ");

                if (strcmp(argv[0], "help") == 0)
                {
                    help(argv[1]);
                }
                else if (strcmp(argv[0], "exit") == 0)
                {
                    exitShell(argv[1]);
                }
                else if (strcmp(argv[0], "cd") == 0)
                {
                    cd(argv[1]);
                }
                else if (strcmp(argv[0], "rm") == 0)
                {
                    if (argv[1] == NULL)
                        printf("Failure: not enough arguments for rm\n");
                    else if (strcmp(argv[1], "-r") == 0) /* rekurencyjne usuwanie katalogów -r*/
                    {
                        if (argv[2] == NULL)
                            printf("Failure: no nessessary argument\n");
                        else
                        {
                            FILE *exists;
                            if ((exists = fopen(argv[2], "rwx")) == NULL)
                                printf("Failure: directory \"%s\" does not exist\n", argv[2]);
                            else
                            {
                                struct stat fileType;
                                stat(argv[2], &fileType);

                                if (S_ISDIR(fileType.st_mode))
                                {
                                    if (rmdir(argv[2]) == FALSE)
                                        rmFunctionRecursive(argv[2]);
                                }
                                else
                                {
                                    remove(argv[2]);
                                }
                            }
                        }
                    }
                    else if (strcmp(argv[1], "-i") == 0) /*pytanie czy chcę usunąć -i*/
                    {
                        if (argc < 3)
                            printf("Failure: too few arguments for rm -i\n");
                        else if (argc > 3)
                            printf("Failure: too much arguments for rm -i\n");
                        else
                        {
                            FILE *exists;
                            if ((exists = fopen(argv[2], "rwx")) == NULL)
                                printf("Failure: directory \"%s\" does not exist\n", argv[2]);
                            else
                            {
                                printf("Do you really want to delete %s? ", argv[2]);
                                printf("Type [Y/N]: ");
                                char *answear = malloc(1);
                                size_t size = 3;
                                getline(&answear, &size, stdin);

                                if (strcmp(answear, "Y\n") == 0)
                                {
                                    struct stat fileType;
                                    stat(argv[2], &fileType);

                                    if (S_ISDIR(fileType.st_mode))
                                    {
                                        if (rmdir(argv[2]) == FALSE)
                                            printf("Failure: this file is not empty use rm -r\n");
                                    }
                                    else
                                    {
                                        remove(argv[2]);
                                    }
                                }
                                else if (strcmp(answear, "N\n") == 0)
                                {
                                    continue;
                                }
                                else
                                {
                                    printf("Failure: invalid value\n");
                                }
                                free(answear);
                            }
                        }
                    }
                    else /* rm bez flag */
                    {
                        FILE *exists;
                        if ((exists = fopen(argv[1], "rwx")) == NULL)
                            printf("Failure: directory \"%s\" does not exist\n", argv[1]);
                        else
                        {
                            struct stat fileType;
                            stat(argv[1], &fileType);

                            if (S_ISDIR(fileType.st_mode))
                            {
                                if (rmdir(argv[1]) == FALSE)
                                    printf("Failure: this file is not empty use rm -r\n");
                            }
                            else
                            {
                                remove(argv[1]);
                            }
                        }
                    }
                }
                else if (strcmp(argv[0], "rmdir") == 0) /*usuwanie pustych katalogów */
                {
                    if (argc < 2)
                        printf("Failure: too few arguments for rmdir\n");
                    else
                    {
                        int i = 1;
                        while (i < argc)
                        {
                            FILE *rem;
                            if ((rem = fopen(argv[i], "rwx")) == NULL)
                                printf("Failure: directory \"%s\" does not exist\n", argv[i]);
                            else if (rmdir(argv[i]) == FALSE)
                                printf("Failure: directory \"%s\" is not empty\n", argv[i]);
                            i++;
                        }
                    }
                }
                else if (strcmp(argv[0], "mkdir") == 0)
                {
                    if (argv[1] == NULL)
                        printf("Failure: not enough arguments for mkdir\n");
                    else if (strcmp(argv[1], "-p") == 0) /* mkdir -p */
                    {
                        if (argv[2] == NULL)
                            printf("Failure: not enough arguments for mkdir -p\n");
                        else
                        {
                            char *last = argv[2];
                            last += (strlen(argv[2]) - 1);
                            if ((strcmp(last, "}") == 0) && strlen(argv[2]) > 2) /* tworzenie kilku zagnieżdżonych katalogoów obok siebie */
                            {
                                char *mkdirArgs[BUFF_SIZE / 4];
                                mkdirArgs[0] = strtok(argv[2], "/");
                                int mkdirArgsNum = 0;
                                while (mkdirArgs[mkdirArgsNum] != NULL)
                                    mkdirArgs[++mkdirArgsNum] = strtok(NULL, ",");
                                if (mkdirArgs[1] == NULL)
                                {
                                    if (mkdir(mkdirArgs[0], 0700) == FALSE)
                                        printf("Warning: this directory \"%s\" cannot be created code: \"%d\" \n", mkdirArgs[0], errno);
                                }
                                else
                                {
                                    mkdirArgs[1] += 1;
                                    if (mkdir(mkdirArgs[0], 0700) == FALSE)
                                        printf("Warning: this directory \"%s\" already exists\n", mkdirArgs[0]);
                                    chdir(mkdirArgs[0]);
                                    int i = 1;
                                    while (i < mkdirArgsNum - 1)
                                    {
                                        if (mkdir(mkdirArgs[i], 0700) == FALSE)
                                            printf("Warning: this directory \"%s\" already exists\n", mkdirArgs[i]);
                                        i++;
                                    }
                                    char *lastMkdirArg = mkdirArgs[mkdirArgsNum - 1];
                                    lastMkdirArg[strlen(lastMkdirArg) - 1] = '\0';
                                    if (mkdir(lastMkdirArg, 0700) == FALSE)
                                        printf("Warning: this directory \"%s\" already exists\n", lastMkdirArg);
                                    chdir("..");
                                }
                            }
                            else /* zagnieżdżone katalogi */
                            {
                                if ((argv[3]) != NULL)
                                {
                                    printf("Failure: too many arguments\n");
                                }
                                else
                                {
                                    char *mkdirArgs[BUFF_SIZE / 4];
                                    mkdirArgs[0] = strtok(argv[2], "/");
                                    int mkdirArgsNum = 0;
                                    while (mkdirArgs[mkdirArgsNum] != NULL)
                                        mkdirArgs[++mkdirArgsNum] = strtok(NULL, "/");
                                    int i = 0;
                                    while (mkdirArgs[i] != NULL)
                                    {
                                        if (mkdir(mkdirArgs[i], 0700) == FALSE)
                                            printf("Warning: this directory \"%s\" already exists\n", mkdirArgs[i]);
                                        chdir(mkdirArgs[i]);
                                        i++;
                                    }
                                    while (i > 0)
                                    {
                                        chdir("..");
                                        i--;
                                    }
                                }
                            }
                        }
                    }
                    else /*tworzenie katalogu ze spacją */
                    {
                        char *last = argv[argc - 1];
                        last += strlen(last) - 1;
                        if ((strncmp(argv[1], "\"", 1) == 0) && (strcmp(last, "\"") == 0))
                        {
                            if (argc < 3)
                            {
                                if (mkdir(argv[1], 0700) == FALSE)
                                    printf("Warning: this directory \"%s\" already exists\n", argv[1]);
                            }
                            else if (argc == 3)
                            {
                                int len = strlen(argv[1] + strlen(argv[2]));
                                char *cat = malloc(len);
                                argv[1] += 1;
                                char *secondStr = argv[2];
                                secondStr[strlen(secondStr) - 1] = '\0';
                                sprintf(cat, "%s %s", argv[1], secondStr);
                                if (mkdir(cat, 0700) == FALSE)
                                    printf("Warning: this directory \"%s\" already exists\n", cat);
                                free(cat);
                            }
                            else
                            {
                                printf("Failure: name is too long\n");
                            }
                        }
                        else /* tworzenie kilku katalogów */
                        {
                            int i = 1;
                            while (i < argc)
                            {
                                if (mkdir(argv[i], 0700) == FALSE)
                                    printf("Failure: this directory \"%s\" cannot be created errno: %d\n", argv[i], errno);
                                i++;
                            }
                        }
                    }
                }
                else /* polecenia fork i exec */
                {
                    pid_t pid;
                    int st;
                    pid = fork();
                    if (pid < 0)
                        perror("Failure: fork error");
                    else if (pid > 0)
                        waitpid(-1, &st, 0);
                    else if (pid == 0)
                    {
                        if (execvp(argv[0], argv) == FALSE)
                        {
                            printf("Failure: unknown command\n");
                            exit(1);
                        }
                        break;
                    }

                    break;
                }
            }
            free(path);
            free(input);
            break;
        }
    }
    return EXIT_SUCCESS;
}

char **character_name_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, character_name_generator);
}

char *character_name_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = character_names[list_index++]))
    {
        if (strncmp(name, text, len) == 0)
        {
            return strdup(name);
        }
    }

    return NULL;
}
