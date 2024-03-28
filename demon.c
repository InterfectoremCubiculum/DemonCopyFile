#include <stdio.h>
#include <errno.h> // do obłsugi błędów
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <utime.h>
#define SMALL_FILE_SIZE 1024*1024
#define STANDARD_SLEEP_TIME 300

void Init(int argc,char* argv[]); //Inicjalizuje demona
void CopyFile(const char* srcFile, const char* dstFile); //Kopiuje pliki
void SynchroniseDirectories(const char* sourceDir, const char* destinationDir, int isRecursive); // Synchronizuje katalogii
int ChangeTime(const char* input); //Sprawdza czy talbica char jest liczbą
int IsDirectoryExists(const char *path); //Sprawdza czy path odnosi się do katalogu
void WriteErrorAtributes(const char *programName); // Wypisuje błąd z parametrami
int ChangeSize(const char* input); //Sprawdza czy talbica char jest liczbą
int sleepTime;
int sizeFile;
int main(int argc, char* argv[])
{
	sleepTime = STANDARD_SLEEP_TIME;
	sizeFile = SMALL_FILE_SIZE;
	Init(argc, argv);
	

	/*while(1)
	{
		sleep(sleepTime);
		SynchroniseDirectories(argv[1], argv[2], 0);
	}*/
	printf("\nWykonało kopiowanie plików\n");

}


//Inicjalizuje demona
void Init(int argc,char* argv[])
{
	if(argc >= 3 && argc <= 6) //Sprawdzanie liczby parametrów
	{
		char* sourceDir = argv[1];
		char* destinationDir = argv[2];

		//Sprawdzanie poprawność przejścia do katalogów
		if( IsDirectoryExists(sourceDir) || IsDirectoryExists(destinationDir) )
		{
			printf("\nNie można otworzyć katalogu/katalogów\n");
			exit(1);
		}

		switch(argc) //Sprawdza poprawność podanych parametrów
		{
			// <src_dir> <dest_dir> 
			case 3:
				SynchroniseDirectories(argv[1], argv[2], 0);
				break;
			// <src_dir> <dest_dir> [sleepTime] 
			// <src_dir> <dest_dir> [-R]
			case 4: 
				if(ChangeTime(argv[3])) //Wykonaj podstawowe zadanie z zmienionym czasie
				{
					SynchroniseDirectories(argv[1], argv[2], 0);
				}
				else if(strcmp(argv[3],"-R") == 0) //Wykonuje opcje -R
				{
					SynchroniseDirectories(argv[1], argv[2], 0);
				}
				else
				{
					WriteErrorAtributes(argv[0]);
				}
				break;
			// <src_dir> <dest_dir> [sleepTime] [-R]
			// <src_dir> <dest_dir> [-R] [sizeFile]
			case 5: 
				if(ChangeTime(argv[3]))
				{}
				else
				{
					if(strcmp(argv[3],"-R") == 0) //Wykonuje opcje -R ale z zmienionym czasie
					{	
						SynchroniseDirectories(argv[1], argv[2], 1);	
					}
					else
					{
						WriteErrorAtributes(argv[0]);
					}
					if(ChangeSize(argv[4])) 
					{}	
					else
					{
						WriteErrorAtributes(argv[0]);
					}
				}
				if(strcmp(argv[4],"-R") == 0) //Wykonuje opcje -R ale z zmienionym czasie
				{	
						SynchroniseDirectories(argv[1], argv[2], 1);	
				}	
				else
				{
					WriteErrorAtributes(argv[0]);
				}
				break;
			// <src_dir> <dest_dir> [sleepTime] [-R] [sizeFile]
			case 6:
				if(ChangeTime(argv[3]))
				{}
				else
				{
					WriteErrorAtributes(argv[0]);
				}
				if(strcmp(argv[4],"-R") == 0) //Wykonuje opcje -R ale z zmienionym czasie
				{	
					SynchroniseDirectories(argv[1], argv[2], 1);	
				}
				else
				{
					WriteErrorAtributes(argv[0]);
				}
				if(ChangeSize(argv[5]))
				{}
				else
				{
					WriteErrorAtributes(argv[0]);
				}
			default:
				break;
		}
	}
	else
	{
		printf("Zła liczba parametrów: %s <src_dir> <dest_dir> [sleepTime] [-R]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

//Kopiuje pliki
void CopyFile(const char* srcFile, const char* dstFile)
{
    	int fileToRead = open(srcFile, O_RDONLY);
	struct stat stats; //Przechowuje informacje o pliku
	fstat(fileToRead, &stats);
	
	if(stats.st_size <= sizeFile) //Kopiowanie dla małych plików
	{
	
		//Wczytywanie
		char buf[sizeFile];
		int bytesNumber = read(fileToRead, buf, sizeFile);
		
		//Zapisywanie
		int fileToSave = open(dstFile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode);
		write(fileToSave, buf, bytesNumber);
		close(fileToSave);
	}
	else //Kopiowanie dla większych plików
	{
		//Mapowanie
		void *srcPTR = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fileToRead, 0);
		
		//Zapisywanie
		int fileToSave = open(dstFile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode);
		write(fileToSave, srcPTR, stats.st_size);
		
		//Usuwanie mapowania oraz zamykanie pliku
       		munmap(srcPTR, stats.st_size);
        	close(fileToSave);
	}
	close(fileToRead);
}

// Synchronizuje katalogii
void SynchroniseDirectories(const char* sourceDir, const char* destinationDir, int isRecursive)
{
	DIR *srcDIR = opendir(sourceDir);
	struct dirent *checkAll;
	while ((checkAll = readdir(srcDIR)) != NULL) 
	{
		//Przeszukuje wszystkie pliku pomijając plik . - bieżący katalog oraz .. - nadrzędny
		if (strcmp(checkAll->d_name, ".") == 0 || strcmp(checkAll->d_name, "..") == 0) 
		{
		    continue;
        	}

        	
            	//Tworzenie scieżek do plików źródłowych i docelowych
		char* srcFile = malloc((strlen(sourceDir)+strlen(checkAll->d_name)+2)*sizeof(char));
		strcpy(srcFile, sourceDir);
		strcat(srcFile, "/");
		strcat(srcFile, checkAll->d_name);
		
		char* dstFile = malloc((strlen(destinationDir)+strlen(checkAll->d_name)+2)*sizeof(char));
	  	strcpy(dstFile, destinationDir);
		strcat(dstFile, "/");
		strcat(dstFile, checkAll->d_name);

		//Pobieranie właściwości o pliku źródłowym
        	struct stat srcStats, dstStats;
		stat(srcFile, &srcStats);
		
		//Jeżeli plik jest katalogiem oraz mamy opcje rekurencjii to wykonujemy też synchronizacje dla elementów w danym katalogu
		if (S_ISDIR(srcStats.st_mode) && isRecursive == 1) 
		{
			// TODO: Tu implementuje rekurencje -R
		} 
		else if (S_ISREG(srcStats.st_mode)) 
		{
			//Jeżeli plik nie istnieje lub jest starszy dokonaj jego kopiowania
			if (stat(dstFile, &dstStats) == -1 || srcStats.st_mtime > dstStats.st_mtime) 
			{
				CopyFile(srcFile, dstFile); 

				struct utimbuf time; //Zmieniamy czas modyfikacji pliku przy użyciu funkcji utime
				time.modtime = srcStats.st_mtime;
				utime(dstFile ,&time);
			}
		}
		free(dstFile);
		free(srcFile);
	}
    	closedir(srcDIR);
    	
	DIR *dstDIR = opendir(destinationDir);
   	while ((checkAll = readdir(dstDIR)) != NULL) 
   	{
   		//Przeszukuje wszystkie pliku pomijając plik . - bieżący katalog oraz .. - nadrzędny
		if (strcmp(checkAll->d_name, ".") == 0 || strcmp(checkAll->d_name, "..") == 0) 
		{
		    continue;
        	}

        	//Tworzenie scieżek do plików źródłowych i docelowych
		char* srcFile = malloc((strlen(sourceDir)+strlen(checkAll->d_name)+2)*sizeof(char));
		strcpy(srcFile, sourceDir);
		strcat(srcFile, "/");
		strcat(srcFile, checkAll->d_name);
		
		char* dstFile = malloc((strlen(destinationDir)+strlen(checkAll->d_name)+2)*sizeof(char));
	  	strcpy(dstFile, destinationDir);
		strcat(dstFile, "/");
		strcat(dstFile, checkAll->d_name);
		
		//Jeżeli plik nie istnieje w katalogu źródłowym to go usuń
		if (access(srcFile, F_OK) == -1) 
		{

			printf("%s",srcFile);
			unlink(dstFile);
			perror("unlink");
		}
		free(srcFile);
		free(dstFile);

    	}
    	closedir(dstDIR);
}

//Sprawdza czy tablica char jest liczbą
int ChangeTime(const char* input)
{
	for(int i=0; i < strlen(input); i++)
	{
		if(!isdigit(input[i]))
		{
			return 0;
		}
	}
	sleepTime = atoi(input);
	return 1;
}
int ChangeSize(const char* input)
{
	for(int i=0; i < strlen(input); i++)
	{
		if(!isdigit(input[i]))
		{
			return 0;
		}
	}
	sizeFile = atoi(input);
	return 1;
}
//Sprawdza czy path odnosi się do katalogu
int IsDirectoryExists(const char *path) 
{
	struct stat s;
	if (stat(path, &s) == -1 || !S_ISDIR(s.st_mode)) 
		return 1;
	return 0;
}

void WriteErrorAtributes(const char *programName)
{
	printf("Błąd w parametrach: %s <src_dir> <dest_dir> [sleepTime] [-R] [sizeFile]\n", programName);
	exit(EXIT_FAILURE);
}
