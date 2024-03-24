// format: {BasedOnStyle: Google, UseTab: Never, IndentWidth: 8, TabWidth: 8,
// CloumnLimit: 80, BreakBeforeBraces: Stroustrup,
// AllowShortIfStatementsOnASingleLine: false}
#define _GNU_SOURCE
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool omoguci_log = false;

int tekuci_pr = -1;
int kontekst[4] = {0, 0, 0, 0};
int kontrolne_zastavice[4] = {0, 0, 0, 0};

/* Pretvori brojeve signala (5, 10, 12, 23) u brojeve (3, 2, 1, 0) */
int parsiraj_signal(int sig)
{
        switch (sig) {
                case 5:
                        return 3;
                case 10:
                        return 2;
                case 12:
                        return 1;
                case 23:
                        return 0;
                default:
                        return -1;
        }
}

/* Blokiraj/odblokiraj signal pod odredenim brojem */
void blokiraj_signal(bool blokiraj, int broj)
{
        sigset_t signali;
        sigemptyset(&signali);
        sigaddset(&signali, broj);
        if (blokiraj)
                pthread_sigmask(SIG_BLOCK, &signali, NULL);
        else
                pthread_sigmask(SIG_UNBLOCK, &signali, NULL);
}

/* Blokiraj/odblokiraj sve signale */
void blokiraj_sve(bool blokiraj)
{
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

/* Provjerava jesu li sve kontrolne zastavice na nuli */
bool prazne_kz()
{
        for (int i = 0; i < 4; i++)
                if (kontrolne_zastavice[i] != 0)
                        return false;
        return true;
}

/* Nalazi signal najviseg prioriteta */
int max_prioritet()
{
        int max = 0;
        for (int i = 0; i < 4; i++)
                if (kontrolne_zastavice[i] == 1)
                        max = i;
        return max;
}

/* Pretvori programsku logiku u iscitljivu logiku */
int transl_pr(int prioritet)
{
        switch (prioritet) {
                case 3:
                        return 1;
                case 2:
                        return 2;
                case 1:
                        return 3;
                case 0:
                        return 4;
                case -1:
                        return 0;
                default:
                        return -1;
        }
}

/* Ispisi log */
void print_log(int max_prioritet)
{
        printf("Kont. zastavice: ");
        for (int i = 0; i < 4; i++) printf("%d ", kontrolne_zastavice[i]);
        printf("\nTekuci prioritet: %d\n", transl_pr(tekuci_pr));
        printf("Max prioritet: %d\n", transl_pr(max_prioritet));
}

/* Potprogram (funkcija) najviseg prioriteta */
void fun_prvi()
{
        for (int i = 0; i < 11; i++) {
                printf("\t\t\t\tPotprogram prioriteta 1... [%d od 10]\n", i);
                sleep(1);
        }
}

/* Potprogram (funkcija) 2. prioriteta */
void fun_drugi()
{
        for (int i = 0; i < 11; i++) {
                printf("\t\t\tPotprogram prioriteta 2... [%d od 10]\n", i);
                sleep(1);
        }
}

/* Potprogram (funkcija) 3. prioriteta */
void fun_treci()
{
        for (int i = 0; i < 11; i++) {
                printf("\t\tPotprogram prioriteta 3... [%d od 10]\n", i);
                sleep(1);
        }
}

/* Potprogram (funkcija) najnizeg prioriteta */
void fun_cetvrti()
{
        for (int i = 0; i < 11; i++) {
                printf("\tPotprogram prioriteta 4... [%d od 10]\n", i);
                sleep(1);
        }
}

/* Handler koji upravlja signalima i prekidima */
void signal_handler(int sig)
{
        blokiraj_sve(true);
        int sig_id = parsiraj_signal(sig);
        kontrolne_zastavice[sig_id] = 1;

        if (max_prioritet() > tekuci_pr && tekuci_pr != -1) {
                printf("Prekid veceg prioriteta. Ulazim u njega.\n");
                printf("[Kuc. poslovi -> Kont. sust. stog + Isp. lanac]\n");
        }
        else if (max_prioritet() <= tekuci_pr && tekuci_pr != -1) {
                printf("Prekid manjeg/jednakog prioriteta. Pamtim.\n");
                printf("[Kuc. poslovi -> Kont. sust. stog + Isp. lanac]\n");
        }
        else {
                printf("Prekid. Ulazim u njega.\n");
                printf("[Kuc. poslovi -> Kont. sust. stog + Isp. lanac]\n");
        }
        while (!prazne_kz() && max_prioritet() > tekuci_pr) {
                printf("Spremam konekst... [Sig. id = %d]\n", sig_id);
                if (omoguci_log) {
                        print_log(max_prioritet());
                }
                int j = max_prioritet();
                kontrolne_zastavice[j] = 0;
                kontekst[j] = tekuci_pr;
                tekuci_pr = j;
                blokiraj_sve(false);
                switch (j) {
                        case 0:
                                fun_cetvrti();
                                break;
                        case 1:
                                fun_treci();
                                break;
                        case 2:
                                fun_drugi();
                                break;
                        case 3:
                                fun_prvi();
                                break;
                        default:
                                break;
                }
                printf("Obnavljam kontekst...\n");
                printf("[Kuc. poslovi -> Kont. sust. stog + Isp. lanac]\n");
                blokiraj_sve(true);
                tekuci_pr = kontekst[j];
        }
        blokiraj_sve(false);
}

int main(int argc, char **argv)
{
        /* Omoguci ispis dodatnog log-a u programu */
        if (argc > 2) {
                printf("Navedeno previse argumenata.\nZa log upisati 'log'.\n");
                return 0;
        }
        if (argc == 2 && strcmp("log", argv[1]) != 0) {
                printf("Nepoznati argument.\nZa log upisati 'log'.\n");
                return 0;
        }
        if (argc == 2 && strcmp("log", argv[1]) == 0) {
                omoguci_log = true;
                printf("Log omogucen.\n");
        }
        else {
                printf("Log onemogucen.\n");
        }

        struct sigaction act;

        act.sa_handler = signal_handler;
        sigemptyset(&act.sa_mask);       // Obrisati bitove maske
        act.sa_flags = 0;                // Dodatne zastavice onemoguciti
        sigaction(SIGTRAP, &act, NULL);  // (max prioritet)

        act.sa_handler = signal_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGUSR1, &act, NULL);

        act.sa_handler = signal_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGUSR2, &act, NULL);

        act.sa_handler = signal_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGURG, &act, NULL);  // (min prioritet)

        while (1) {
                printf("Glavni program radi [pid = %d]\n", (int)getpid());
                sleep(1);
        }
        return 0;
}