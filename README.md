# Upotreba deljenog memorisjksog segmenta sa semaforima u C-u

Ovaj kod demonstrira korišćenje deljenog memorijskog segmenta u operativnom sistemu Kali Linux koristeći C jezik. Deljeni memorisjki segment koristi se za komunikaciju između dva procesa putem semafora.

## Instalacija

1. Klonirajte repozitorijum:

   git clone https://github.com/alexazivkovic/aros.git

2.1. Komplajlirajte kod koristeci 'gcc' kompajler

   gcc shmsmp.c -o shmsmp.exe -pthread

2.2. Kompajlirajte kod koristeci 'makefile'

   make shmsmp

3. Pokrenite program

   ./shmsmp.exe 
                                                               
