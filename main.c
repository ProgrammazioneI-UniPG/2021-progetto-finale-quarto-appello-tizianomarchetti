#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gamelib.h"

int scelta;

void printMenu() {
  do {
    printf("\nMenu' principale \n 1 -> Imposta gioco \n 2 -> Gioca \n 3 -> Termina gioco\n");
		scanf("%d", &scelta);
    while(getchar() != '\n');

    switch (scelta) {
			case 1:
				imposta_gioco();
				break;
			case 2:
        gioca();
				break;
			case 3:
				termina_gioco();
				break;
			default:
				printf("\nScelta non valida.\n");
        break;
		}
	} while (scelta != 1 && scelta != 2 && scelta != 3);

  return;
}

int main() {
  printf("\nUn gruppo di astronauti si trova in viaggio sull\'astronave Skelt, e il loro obiettivo e\' riuscire a\ncompletare tutte le attivita\' previste (le quest) per il mantenimento della nave, arrivando cosi\' a destinazione.\nTra di loro si celano pero\' anche degli impostori, il cui scopo e\' eliminare di nascosto gli astronauti senza farsi\nscoprire da essi. Riusciranno ad arrivare a destinazione prima di essere decimati?\n");
  while(getchar() != '\n');
  time_t t;
	srand((unsigned) time(&t));

	printMenu();

  return 0;
}
