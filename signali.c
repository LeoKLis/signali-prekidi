#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<stdbool.h>

int tekuci_prioritet = -1;
int kontekst[4] = {0, 0, 0, 0};
int kontrolne_zastavice[4] = {0, 0, 0, 0};
int kontekst_kz[4] = {0, 0, 0, 0};

/* Pretvori brojeve signala u brojeve izmedu 0 i 3 */
int parsiraj_signal(int sig){
    switch (sig){
        case 5: return 3;
        case 10: return 2;
        case 12: return 1;
        case 23: return 0;
        default: return -1;
    }
}

/* Blokiraj/odblokiraj signal pod odredenim brojem */
void blokiraj_signal(bool blokiraj, int broj){
    sigset_t signali;
    sigemptyset(&signali);
    sigaddset(&signali, broj);
    if (blokiraj)
        pthread_sigmask(SIG_BLOCK, &signali, NULL);
    else
        pthread_sigmask(SIG_UNBLOCK, &signali, NULL);
}

void blokiraj_sve(bool blokiraj){
    sigset_t signali;
    sigemptyset(&signali);
    sigaddset(&signali, 5);
    sigaddset(&signali, 10);
    sigaddset(&signali, 12);
    sigaddset(&signali, 23);
    if (blokiraj)
        pthread_sigmask(SIG_BLOCK, &signali, NULL);
    else
        pthread_sigmask(SIG_UNBLOCK, &signali, NULL);
}

bool prazne_kz(){
    for (int i = 0; i < 4; i++)
    {
        if(kontrolne_zastavice[i] != 0) return false; 
    }
    return true;
}

int najvisi_prioritet(){
    int max = 0;
    for (int i = 0; i < 4; i++)
        if(kontrolne_zastavice[i] == 1)
            max = i;
    return max;    
}

void spremi_kz_kontekst(){
    for (int i = 0; i < 4; i++)
        kontekst_kz[i] = kontrolne_zastavice[i];
}

void obnovi_kz_kontekst(){
    for (int i = 0; i < 4; i++)
        kontrolne_zastavice[i]  = kontekst_kz[i];
}

void prvi_prioritet(){
    for (int i = 0; i < 11; i++)
    {
        printf("\tPotprogram prioriteta 1... [%d od 10]\n", i);
        sleep(1);
    }    
}
void drugi_prioritet(){
    for (int i = 0; i < 11; i++)
    {
        printf("\t\tPotprogram prioriteta 2... [%d od 10]\n", i);
        sleep(1);
    }    
}
void treci_prioritet(){
    for (int i = 0; i < 11; i++)
    {
        printf("\t\t\tPotprogram prioriteta 3... [%d od 10]\n", i);
        sleep(1);
    }    
}
void cetvrti_prioritet(){
    for (int i = 0; i < 11; i++)
    {
        printf("\t\t\t\tPotprogram prioriteta 4... [%d od 10]\n", i);
        sleep(1);
    }    
}

void signal_rukovatelj(int sig){
    blokiraj_sve(true);
    int sig_id = parsiraj_signal(sig);
    kontrolne_zastavice[sig_id] = 1;

    printf("Spremam konekst...\n");
    printf("Sig id = %d\n", sig_id);
    for (int i = 0; i < 4; i++)
        printf("%d ", kontrolne_zastavice[i]);
    printf("\nNajvisi prioritet %d\n", najvisi_prioritet());
    
    while(!prazne_kz() && najvisi_prioritet() > tekuci_prioritet){
        int j = najvisi_prioritet();
        kontrolne_zastavice[j] = 0;
        kontekst[j] = tekuci_prioritet;
        tekuci_prioritet = j;
        blokiraj_sve(false);
        if(j == 0)
            cetvrti_prioritet();
        else if(j == 1)
            treci_prioritet();
        else if(j == 2)
            drugi_prioritet();
        else
            prvi_prioritet();
        blokiraj_sve(true);
        tekuci_prioritet = kontekst[j];
    }
    blokiraj_sve(false);
}

int main(){
    struct sigaction act;

    // Inicijalizacija signala
    act.sa_handler = signal_rukovatelj;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTRAP, &act, NULL); // SIGTRAP - 5 (min prioritet)

    act.sa_handler = signal_rukovatelj;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL); // SIGUSR1 - 10

    act.sa_handler = signal_rukovatelj;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR2, &act, NULL); // SIGUSR2 - 12

    act.sa_handler = signal_rukovatelj;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGURG, &act, NULL); // SIGURG - 23 (max prioritet)

    while (1){
        printf("Glavni program radi [pid = %d]\n", (int) getpid());
        sleep(1);
    }
    return 0;
}