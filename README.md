**Podział pracy:**
- Eryk Ś:
- [x] sprawdza parametry 
- [x] kopiowanie plików(narazie bez rekurencji i data nie jest datą modyfikacji) 
- [x] sprawdzanie tego czy podany parametr to katalog
- [x] usuwanie plików

- Mateusz I.
- [x] realizacja sleepa 
- [x] obsługa sygnałów (wybudzenie, uruchomienie i uśpienie) 
- [x] logowanie informacji o demononie
- [x] Podział na oddzielne pliki wykonawcze
- [x] praca w tle (zamiast pracy przy uruchomionym procesie)
     
-  Michał K.
- [ ]  Zapisywanie i obsługa katalogów (zaglądanie rekurencyjne)
 
-  Norbert K.
- [ ] Dokumentacja


Przydatne komendy:
- gcc -o demon demon1.c log.c      (kompilacja demona)
- ./demon kat1 kat2 &              (uruchomienie demona z tle)
- pkill demon                      (zakończenie pracy demona)


Zaglądanie do logów:
- var/log/syslog na Ubuntu
- journalctl --since "10 minutes  ago" na Kali Linux, dziennik z ostatnich 10 minut


