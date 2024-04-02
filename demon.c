#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <utime.h>
#include <signal.h>

#define SIGALRM 14
#define SIGUSR1 10
#define SMALL_FILE_SIZE 1024*1024
#define STANDARD_SLEEP_TIME 5//300 do testow na razie co 5 sekund zamiast 5 minut

void Init(int argc, char* argv[]);
void CopyFile(const char* srcFile, const char* dstFile);
void SynchroniseDirectories(const char* sourceDir, const char* destinationDir, int isRecursive);
int ChangeTime(const char* input);
int ChangeSize(const char* input);
int IsDirectoryExists(const char *path);
void WriteErrorAtributes(const char *programName);
void SignalHandler(int sig);
void AlarmHandler(int sig);

char* sourceDir;
char* destinationDir;

int main(int argc, char* argv[])
{
    printf("Demon Kopiujący Pliki został uruchomiony.\n");
    // Ustawienie obsługi sygnałów
    signal(SIGALRM, AlarmHandler);
    signal(SIGUSR1, SignalHandler);
    Init(argc, argv);
    // Pobranie pid
    pid_t pid = getpid();
    alarm(STANDARD_SLEEP_TIME);
    // Pętla nieskończona, aby program mógł otrzymywać sygnały
    while (1) {
         pause();
    }

    return 0;
}

void AlarmHandler(int sig) {
    if (sig == SIGALRM) {
        // Wyślij sygnał SIGUSR1
        pid_t pid = getpid();
        kill(pid, SIGUSR1);
    alarm(STANDARD_SLEEP_TIME);
 }
}

void SignalHandler(int sig) {
    if (sig == SIGUSR1) {
        // Wykonaj synchronizację
        SynchroniseDirectories(sourceDir, destinationDir, 0);
	printf("Synchronizacja zakonczona.\n");
    }
}

void Init(int argc, char* argv[]) {
    if (argc >= 3 && argc <= 6) {
        sourceDir = argv[1];
        destinationDir = argv[2];

        if (IsDirectoryExists(sourceDir) || IsDirectoryExists(destinationDir)) {
            printf("\nNie można otworzyć katalogu/katalogów\n");
            exit(1);
        }

        switch (argc) {
            case 3:
                SynchroniseDirectories(argv[1], argv[2], 0);
                break;
            case 4:
                if (ChangeTime(argv[3])) {
                    SynchroniseDirectories(argv[1], argv[2], 0);
                } else if (strcmp(argv[3], "-R") == 0) {
                    SynchroniseDirectories(argv[1], argv[2], 0);
                } else {
                    WriteErrorAtributes(argv[0]);
                }
                break;
            case 5:
                if (ChangeTime(argv[3])) {
                } else {
                    if (strcmp(argv[3], "-R") == 0) {
                    } else {
                        WriteErrorAtributes(argv[0]);
                    }
                    if (ChangeSize(argv[4])) {
                        SynchroniseDirectories(argv[1], argv[2], 1);
                    } else {
                        WriteErrorAtributes(argv[0]);
                    }
                }
                if (strcmp(argv[4], "-R") == 0) {
                    SynchroniseDirectories(argv[1], argv[2], 1);
                } else {
                    WriteErrorAtributes(argv[0]);
                }
                break;
            case 6:
                if (ChangeTime(argv[3])) {
                } else {
                    WriteErrorAtributes(argv[0]);
                }
                if (strcmp(argv[4], "-R") == 0) {
                } else {
                    WriteErrorAtributes(argv[0]);
                }
                if (ChangeSize(argv[5])) {
                    SynchroniseDirectories(argv[1], argv[2], 1);
                } else {
                    WriteErrorAtributes(argv[0]);
                }
            default:
                break;
        }
    } else {
        printf("Zła liczba parametrów: %s <src_dir> <dest_dir> [sleepTime] [-R]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void CopyFile(const char* srcFile, const char* dstFile) {
    int fileToRead = open(srcFile, O_RDONLY);
    struct stat stats;
    fstat(fileToRead, &stats);
    
    if (stats.st_size <= SMALL_FILE_SIZE) {
        char buf[SMALL_FILE_SIZE];
        int bytesNumber = read(fileToRead, buf, SMALL_FILE_SIZE);
        int fileToSave = open(dstFile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode);
        write(fileToSave, buf, bytesNumber);
        close(fileToSave);
    } else {
        void *srcPTR = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fileToRead, 0);
        int fileToSave = open(dstFile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode);
        write(fileToSave, srcPTR, stats.st_size);
        munmap(srcPTR, stats.st_size);
        close(fileToSave);
    }
    close(fileToRead);
}

void SynchroniseDirectories(const char* sourceDir, const char* destinationDir, int isRecursive) {
    DIR *srcDIR = opendir(sourceDir);
    struct dirent *checkAll;
    while ((checkAll = readdir(srcDIR)) != NULL) {
        if (strcmp(checkAll->d_name, ".") == 0 || strcmp(checkAll->d_name, "..") == 0) {
            continue;
        }

        char* srcFile = malloc((strlen(sourceDir)+strlen(checkAll->d_name)+2)*sizeof(char));
        strcpy(srcFile, sourceDir);
        strcat(srcFile, "/");
        strcat(srcFile, checkAll->d_name);
        
        char* dstFile = malloc((strlen(destinationDir)+strlen(checkAll->d_name)+2)*sizeof(char));
        strcpy(dstFile, destinationDir);
        strcat(dstFile, "/");
        strcat(dstFile, checkAll->d_name);

        struct stat srcStats, dstStats;
        stat(srcFile, &srcStats);
        
        if (S_ISDIR(srcStats.st_mode) && isRecursive == 1) {
            // Implementuj rekurencję -R
        } else if (S_ISREG(srcStats.st_mode)) {
            if (stat(dstFile, &dstStats) == -1 || srcStats.st_mtime > dstStats.st_mtime) {
                CopyFile(srcFile, dstFile); 
                struct utimbuf time;
                time.modtime = srcStats.st_mtime;
                utime(dstFile ,&time);
            }
       }
        free(dstFile);
        free(srcFile);
    }
    closedir(srcDIR);
    
    DIR *dstDIR = opendir(destinationDir);
    while ((checkAll = readdir(dstDIR)) != NULL) {
        if (strcmp(checkAll->d_name, ".") == 0 || strcmp(checkAll->d_name, "..") == 0) {
            continue;
        }

        char* srcFile = malloc((strlen(sourceDir)+strlen(checkAll->d_name)+2)*sizeof(char));
        strcpy(srcFile, sourceDir);
        strcat(srcFile, "/");
        strcat(srcFile, checkAll->d_name);
        
        char* dstFile = malloc((strlen(destinationDir)+strlen(checkAll->d_name)+2)*sizeof(char));
        strcpy(dstFile, destinationDir);
        strcat(dstFile, "/");
        strcat(dstFile, checkAll->d_name);
        
        if (access(srcFile, F_OK) == -1) {
            unlink(dstFile);
            perror("unlink");
        }
        free(srcFile);
        free(dstFile);
    }
    closedir(dstDIR);
}

int ChangeTime(const char* input) {
    for (int i = 0; i < strlen(input); i++) {
        if (!isdigit(input[i])) {
            return 0;
        }
    }
    return 1;
}

int ChangeSize(const char* input) {
    int size = atoi(input);
    if (size > 0) {
        return 1;
    }
    return 0;
}

int IsDirectoryExists(const char *path) {
    struct stat s;
    if (stat(path, &s) == -1 || !S_ISDIR(s.st_mode)) {
        return 1;
    }
    return 0;
}

void WriteErrorAtributes(const char *programName) {
    printf("Błąd w parametrach: %s <src_dir> <dest_dir> [sleepTime] [-R] [sizeFile]\n", programName);
    exit(EXIT_FAILURE);
}
