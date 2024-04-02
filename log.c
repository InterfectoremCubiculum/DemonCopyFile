#include <stdio.h>
#include <time.h>

void writeToLog(const char* message) {
    FILE* logFile = fopen("daemon_log.txt", "a");
    if (logFile == NULL) {
        perror("Błąd podczas otwierania pliku dziennika");
        return;
    }

    time_t currentTime;
    struct tm* timeInfo;
    char timeBuffer[26];

    time(&currentTime);
    timeInfo = localtime(&currentTime);

    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);

    fprintf(logFile, "[%s] %s\n", timeBuffer, message);

    fclose(logFile);
}