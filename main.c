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
#define SMALL_FILE_SIZE (1024*1024)
#define STANDARD_SLEEP_TIME 10//300 do testow na razie co 10 sekund zamiast 5 minut

extern void Init(int argc, char* argv[]);
extern int CopyFile(const char* srcFile, const char* dstFile);
extern void SynchroniseDirectories(const char* sourceDir, const char* destinationDir);
extern int ChangeTime(const char* input);
extern int ChangeSize(const char* input);
extern int IsDirectoryExists(const char *path);
extern void WriteErrorAttributes(const char *programName);
extern void SignalHandler(int sig);
extern void AlarmHandler(int sig);
extern void RemoveDirectoryRecursively(const char *path, const char *srcFile);

char* sourceDir;
char* destinationDir;
int sleepTime = 0; // określa czas spania demona
int recursion = 0; // 0 - nie rekurecyjne kopiowanie, 1 - rekurecyjne kopiowanie
int sizeFile = 0; // Przetrzymuje rozmiar pliku ograniczający jaką metodę użyć do kopiowania

int main(int argc, char* argv[])
{
    openlog("DemonSynchronizujący", LOG_PID, LOG_DAEMON);
    // Ustawienie obsługi sygnałów
    signal(SIGALRM, AlarmHandler);
    signal(SIGUSR1, SignalHandler);
    Init(argc, argv);
    daemon(0, 1);
    syslog(LOG_INFO, "Demon Kopiujący Pliki został uruchomiony.\n");
    // Pobranie pid
    pid_t pid = getpid();
    if(sleepTime == 0)
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
