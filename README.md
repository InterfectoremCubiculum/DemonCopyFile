**Podział pracy:**
- Eryk Ś:
- [x] sprawdza parametry 
- [x] kopiowanie plików
- [x] sprawdzanie tego czy podany parametr to katalog
- [x] usuwanie plików
- [x] Usuwanie rekurencyjne

- Mateusz I.
- [x] realizacja sleepa 
- [x] obsługa sygnałów (wybudzenie, uruchomienie i uśpien) 
- [x] logowanie informacji o demononie
- [x] Podział na oddzielne pliki wykonawcze
- [x] praca w tle (zamiast pracy przy uruchomionym procesie. Innymi słowy zrobienie faktycznego "Demona" z naszego programu)
     
-  Michał K.
- [x]  Otwieranie i Tworzenie Katalogów Rekurencyjnie (Przeglądanie zawartości katalogów)
-  Norbert K.
- [x] Dokumentacja


Przydatne komendy:
- gcc -o demon main.c demon.c                               (kompilacja demona)
- ./demon /home/student/kat1 /home/student/kat2             (uruchomienie demona z tle)
- pkill demon                                               (zakończenie pracy demona)
- kill -SIGUSR1 *pid programu*                              (zdalne wysłanie sygnału wybudzającego demona)                                        

Co więcej teraz demon osadzony będzie zawsze w katalogu głównym zatem i położenie naszych katalogów względem niego zostało zmienione, można to zauważyć w przydatne komendy powyżej 

Zaglądanie do logów:
- cat var/log/syslog na Ubuntu
- journalctl --since "10 minutes  ago" na Kali Linux, dziennik z ostatnich 10 minut

przykładowe wywołania Demona

- ./demon /home/student/katalog_test /home/student/katalog_test2
- ./demon /home/student/katalog_test /home/student/katalog_test2 50 -R 300
- ./demon /home/student/katalog_test /home/student/katalog_test2 -R
- ./demon /home/student/katalog_test /home/student/katalog_test2 -R 300
- ./demon /home/student/katalog_test /home/student/katalog_test2 10 300
