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
#include <syslog.h>

#define SIGALRM 14
#define SIGUSR1 10
#define SMALL_FILE_SIZE 1024*1024
#define STANDARD_SLEEP_TIME 10//300 do testow na razie co 5 sekund zamiast 5 minut

void Init(int argc, char* argv[]);
int CopyFile(const char* srcFile, const char* dstFile);
void SynchroniseDirectories(const char* sourceDir, const char* destinationDir);
int ChangeTime(const char* input);
int ChangeSize(const char* input);
int IsDirectoryExists(const char *path);
void WriteErrorAtributes(const char *programName);
void SignalHandler(int sig);
void AlarmHandler(int sig);

char* sourceDir;
char* destinationDir;
int sleepTime = 0; // określa czas spania demona
int recursion = 0; // 0 - nie rekurecyjne kopiowanie, 1 - rekurecyjne kopiowanie
int sizeFile = 0; // Przetrzymuje rozmiar pliku ograniczający jaką metodę użyć do kopiowania

int main(int argc, char* argv[])
{
    openlog("DemonSynchronizujący", LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_INFO,"Demon Kopiujący Pliki został uruchomiony.\n");
    // Ustawienie obsługi sygnałów
    signal(SIGALRM, AlarmHandler);
    signal(SIGUSR1, SignalHandler);
    Init(argc, argv);
    daemon(0, 1);
    // Pobranie pid
    pid_t pid = getpid();
    if(sleepTime ==0)
    {
        sleepTime = STANDARD_SLEEP_TIME;
    }
    if(sizeFile == 0)
    {
        sizeFile = SMALL_FILE_SIZE;
    }
    alarm(sleepTime);

    closelog();
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
	alarm(sleepTime);
	openlog("DemonSynchronizujący", LOG_NDELAY, LOG_DAEMON);
    	syslog(LOG_INFO,"Uspano Demona.\n");
	closelog();
 }
}

void SignalHandler(int sig) {
    if (sig == SIGUSR1) {
        // Wykonaj synchronizację
        openlog("DemonSynchronizujący", LOG_NDELAY, LOG_DAEMON);
        syslog(LOG_INFO,"Wysłano sygnał SIGUSR1.");
        syslog(LOG_INFO,"Wybudzono Demona.\n");
        SynchroniseDirectories(sourceDir, destinationDir);
	syslog(LOG_INFO,"Zakończono synchronizację katalogów.\n");
	closelog();
	
    }
}


// Inizcjalizacja demona podanymi parametrami
void Init(int argc, char* argv[]) 
{
    if (argc >= 3 && argc <= 6) 
    {
        sourceDir = argv[1];
        destinationDir = argv[2];

        if (IsDirectoryExists(sourceDir) || IsDirectoryExists(destinationDir)) 
        {
            printf("\nNie można otworzyć katalogu/katalogów\n");
            exit(1);
        }

        switch (argc) 
        {
            case 4:
                if (ChangeTime(argv[3])) 
                {} 
                else if (strcmp(argv[3], "-R") == 0) 
                {
                    recursion = 1;
                }
                else 
                {
                    WriteErrorAtributes(argv[0]);
                }
                break;
            case 5:
                if (ChangeTime(argv[3]) ) 
                {
                    if (strcmp(argv[4], "-R") == 0) 
                    {
                        recursion = 1;
                    } 
                    else if(ChangeSize(argv[4]))
                    {}
                    else
                    {
                        WriteErrorAtributes(argv[0]);
                    }
                } 
                else 
                {
                    if (strcmp(argv[3], "-R") == 0) 
                    {
                        recursion = 1;
                        if (ChangeSize(argv[4])) 
                        {} 
                        else 
                        {
                            WriteErrorAtributes(argv[0]);
                        }

                    }
                    else 
                    {
                        WriteErrorAtributes(argv[0]);
                    }
                }
                break;
            case 6:
                if (ChangeTime(argv[3]) && (strcmp(argv[4], "-R") == 0) && (ChangeSize(argv[5])) ) 
                {} 
                else 
                {
                    WriteErrorAtributes(argv[0]);
                }
                break;
            default:
                WriteErrorAtributes(argv[0]);
                break;
        }
    } 
    else 
    {
        printf("Zła liczba parametrów: %s <src_dir> <dest_dir> [sleepTime] [-R]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

// Kopiuje plik między dwo folderami
int CopyFile(const char* srcFile, const char* dstFile) 
{
    int fileToRead = open(srcFile, O_RDONLY);
    int copyMethod = 0;
    struct stat stats;
    fstat(fileToRead, &stats);
    
    if (stats.st_size <= sizeFile) // Metodą read/write, jeżeli plik jest mniejszy  niż parametr sizeFile
    {
        char buf[sizeFile];
        int bytesNumber = read(fileToRead, buf, sizeFile);
        int fileToSave = open(dstFile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode);
        write(fileToSave, buf, bytesNumber);
        close(fileToSave);
        copyMethod = 1;
    } 
    else // Metodą mmap/write
    {
        void *srcPTR = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fileToRead, 0);
        int fileToSave = open(dstFile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode);
        write(fileToSave, srcPTR, stats.st_size);
        munmap(srcPTR, stats.st_size);
        close(fileToSave);
        copyMethod = 2;
    }
    close(fileToRead);
    return copyMethod;
}

void SynchroniseDirectories(const char* sourceDir, const char* destinationDir) 
{
    DIR *srcDIR = opendir(sourceDir);
    struct dirent *checkAll;
    while ((checkAll = readdir(srcDIR)) != NULL) 
    {
        if (strcmp(checkAll->d_name, ".") == 0 || strcmp(checkAll->d_name, "..") == 0) 
        {
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
        
        // Uruchomienie opcji kopiowania rekurecyjnego
        if (S_ISDIR(srcStats.st_mode) && recursion == 1) 
        {
            // Implement recursion -R dla Michala
        } 
        else if (S_ISREG(srcStats.st_mode)) 
        {
            if (stat(dstFile, &dstStats) == -1 || srcStats.st_mtime > dstStats.st_mtime) 
            {
                
            // Wykonuj tylko wtedy, gdy plik źródłowy jest nowszy niż plik docelowy
            int copyMethod = CopyFile(srcFile, dstFile); 

            struct utimbuf time;
            time.modtime = srcStats.st_mtime; 
            utime(dstFile ,&time); // Zmiana czasu modyfikacji

            // Dodanie informacji do logu
            char logMsg[500];
            if(copyMethod == 1) // Metodą read/write dla mniejszych plików
            {
                snprintf(logMsg, sizeof(logMsg), "Skopiowano plik %s do %s metodą read/write\n", srcFile, dstFile);
            }
            else if(copyMethod == 2) // Metodą mmap/write dla większych plików
            {
                snprintf(logMsg, sizeof(logMsg), "Skopiowano plik %s do %s metodą mmap/write \n", srcFile, dstFile);
            }
            syslog(LOG_INFO,"%s",logMsg);
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
        

        // Usuwa plik
        if (access(srcFile, F_OK) == -1) {
            unlink(dstFile);
            char logMsg[500];
            snprintf(logMsg, sizeof(logMsg), "Usunieto plik %s, poniewaz nie istnieje on już w %s\n", dstFile, srcFile);
            syslog(LOG_INFO,"%s",logMsg);
        }
        free(srcFile);
        free(dstFile);
    }
    closedir(dstDIR);
}

// Sprawdza czy podana wartość jest liczbą i zapisuje jako czas między pobudzeniem demona
int ChangeTime(const char* input) 
{
    for (int i = 0; i < strlen(input); i++) 
    {
        if (!isdigit(input[i])) 
        {
            return 0;
        }
    }
    sleepTime = atoi(input);
    return 1;
}

// Sprawdza czy podana wartość jest liczbą i zapisuje jako nową granice dla metod kopiujących pliki
int ChangeSize(const char* input) 
{
    for (int i = 0; i < strlen(input); i++) 
    {
        if (!isdigit(input[i])) 
        {
            return 0;
        }
    }
    sizeFile = atoi(input);
    return 1;
}

// Sprawdza czy istnieją takie katalogii
int IsDirectoryExists(const char *path) {
    struct stat s;
    if (stat(path, &s) == -1 || !S_ISDIR(s.st_mode)) {
        return 1;
    }
    return 0;
}


// Zamyka demona oraz informuje o błędach w podanych parametrach
void WriteErrorAtributes(const char *programName) 
{
    printf("Błąd w parametrach: %s <src_dir> <dest_dir> [sleepTime] [-R] [sizeFile]\n", programName);
    char logMsg[500];
    snprintf(logMsg, sizeof(logMsg), "Demon został zamknięty z powodu błędnych parametrów");
    syslog(LOG_INFO,"%s",logMsg);
    exit(EXIT_FAILURE);
}
