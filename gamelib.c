#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gamelib.h"

//Variabili statiche

static struct Giocatore* primoGiocatore = NULL;
static struct Giocatore* ultimoGiocatore = NULL;
static struct Giocatore* giocatori;

static struct Stanza* stanza_inizio = NULL;
static struct Stanza* lista_stanze;

static unsigned short quest_da_finire;

static int giocoImpostato;
static int numeroGiocatori, giocatori_eliminati, giocatori_effettivi;
static int counterStanze;
static int vittoria;

//FUNZIONI STATICHE

static const char* getStatoGiocatore(enum Stato_giocatore stato);															//l.101	Funzione d'appoggio che ritorna il nome del giocatore come stringa
static const char* getNomeGiocatore(enum Nome_giocatore nome);																//l.111 Funzione d'appoggio che ritorna lo stato del giocatore come stringa
static char* getPosizione(struct Stanza* posizione);																					//l.127 Funzione d'appoggio che ritorna la posizione del giocatore come stringa
static char* getTipoStanza(struct Stanza* posizione);																					//l.130 Funzione d'appoggio che ritorna il tipo della stanza come stringa
static void stampa_giocatori(struct Giocatore* First, struct Giocatore* Last);								//l.141
static void ins_giocatori(struct Giocatore **First, struct Giocatore **Last);									//l.154
static void ins_stanza(struct Stanza **First, struct Stanza **Previous, int direzione);				//l.207
static void reset();																																					//l.257
static void avanza(struct Giocatore* giocatore);																							//l.729
static void esegui_quest(struct Giocatore* giocatore);																				//l.772 Per risolvere le Quest (semplice e complicata) sono stati impostati due diversi mini-giochi
static char* getStato(enum Stati stato);																											//l.852 Funzione d'appoggio per domanda Quest complicata
static char* getCapitale(enum Stati stato);																										//l.863 Funzione d'appoggio per domanda Quest complicata
static int chiamata_emergenza(struct Giocatore* giocatore);																		//l.875
static void uccidi_astronauta(struct Giocatore* giocatore);																		//l.954
static void usa_botola(struct Giocatore* giocatore);																					//l.1124
static struct Stanza* trova_stanze_con_botola(struct Stanza* stanza_giocatore);								//l.1165 Funzione d'appoggio per usa_botola() usata anche nelle impostazioni iniziali di gioca(), ritorna un array di stanze
static int conta_botole(struct Stanza* stanza_giocatore);																			//l.1185 Funzione d'appoggio usata da usa_botola() e trova_stanze_con_botola(), ritorna un intero (numero di stanze con botola)
static void sabotaggio(struct Giocatore* giocatore);																					//l.1195 Per sabotare, l'impostore deve risolvere i giochi impostati per le Quest



void imposta_gioco() {
	// Se viene rilevato un gioco già impostato, viene data la possibilità di resettarlo e impostarne uno nuovo
	if(giocoImpostato != 0) {
		reset();
	}

	//Imposta numero quest
	quest_da_finire = 0;
	do {
		printf("\nScegliere il numero di Quest da completare per terminare il gioco (min 1 - max 50): \n");
		scanf("%hd", &quest_da_finire);
		while(getchar() != '\n');
	} while (quest_da_finire < 1 || quest_da_finire > 50);

	//Inizializza array stanze e imposta prima Stanza
	lista_stanze = (struct Stanza*) calloc(500, sizeof(struct Stanza));
	ins_stanza(&stanza_inizio, NULL, 0);

	//Inizializza array giocatori e imposta giocatori
	do {
		printf("\nQuanti giocatori? (min 4 - max 10)\n");
		scanf("%d", &numeroGiocatori);
    while(getchar() != '\n');
	} while (numeroGiocatori < 4 || numeroGiocatori > 10);
	giocatori = (struct Giocatore*) calloc(numeroGiocatori, sizeof(struct Giocatore));
  ins_giocatori(&primoGiocatore, &ultimoGiocatore);

	giocoImpostato = 1;
	int scelta;
	do {
		if (scelta == 1) {
			printf("\nScegli se mostrare di nuovo la lista dei giocatori (1) o iniziare a giocare (2)\n");
		}
		else {
			printf("\nGioco impostato. %d quest da finire e %d giocatori.\n Scegli come procedere:\n 1 -> Stampa lista giocatori\n 2 -> Inizia gioco (torna al Menu principale)\n",
							quest_da_finire, numeroGiocatori);
		}
		scanf("%d", &scelta);
		while(getchar() != '\n');
		switch (scelta) {
			case 1:
				stampa_giocatori(primoGiocatore, ultimoGiocatore);
				break;
			case 2:
				printf("\nVerrai ora reindirizzato al Menu princiale (premi Enter)\n");
				while(getchar() != '\n');
				printMenu();
				break;
			default:
				printf("\nScelta non valida.\n");
		}
	} while (scelta != 2);
	return;
}

static const char* getStatoGiocatore(enum Stato_giocatore stato) {
	char* statoGiocatore;
  switch (stato) {
    case astronauta: statoGiocatore = "Astronauta"; break;
    case impostore: statoGiocatore = "Impostore"; break;
    case assassinato: statoGiocatore = "Assassinato"; break;
    case defenestrato: statoGiocatore = "Defenestrato"; break;
  }
	return statoGiocatore;
}
static const char* getNomeGiocatore(enum Nome_giocatore nome) {
	char* nomeGiocatore;
  switch (nome) {
    case rosso: nomeGiocatore = "Rosso"; break;
    case blu: nomeGiocatore = "Blu"; break;
    case giallo: nomeGiocatore = "Giallo"; break;
    case verde: nomeGiocatore = "Verde"; break;
    case marrone: nomeGiocatore = "Marrone"; break;
    case nero: nomeGiocatore = "Nero"; break;
    case bianco: nomeGiocatore = "Bianco"; break;
    case arancio: nomeGiocatore = "Arancio"; break;
    case rosa: nomeGiocatore = "Rosa"; break;
		case viola: nomeGiocatore = "Viola"; break;
  }
	return nomeGiocatore;
}
static char* getPosizione(struct Stanza* posizione) {
	return posizione->nomeStanza;
}
static char* getTipoStanza(struct Stanza* posizione) {
	char* tipo_stanza;
	switch (posizione->tipoStanza) {
		case vuota: tipo_stanza = "Vuota"; break;
		case quest_semplice: tipo_stanza = "Quest semplice"; break;
		case quest_complicata: tipo_stanza = "Quest complicata"; break;
		case botola: tipo_stanza = "Botola"; break;
	}
	return tipo_stanza;
}

static void stampa_giocatori(struct Giocatore* First, struct Giocatore* Last) {
  int counter = 1;
	for (int i = 0; i < numeroGiocatori; i++) {
		  printf("Giocatore %d: Stato = %s, Nome = %s, Posizione = %s\n",
		          counter,
		          getStatoGiocatore((giocatori[i]).statoGiocatore),
		          getNomeGiocatore((giocatori[i]).nomeGiocatore),
							getPosizione((giocatori[i]).posizione));
		  counter++;
	}
  return;
}

static void ins_giocatori(struct Giocatore **First, struct Giocatore **Last) {
	int totImpostori = 0;
	int* nomiAssegnati = (int*) malloc(numeroGiocatori * sizeof(int));
	for(int i = 0; i < numeroGiocatori; i++) {
		struct Giocatore* New = &giocatori[i];
		New->next = NULL;
    if (totImpostori < 3) {
      if (i == (numeroGiocatori - 1) && totImpostori == 0) {
        New->statoGiocatore = 1;
				totImpostori++;
      }
      else {
        New->statoGiocatore = rand() % 2;
				if (New->statoGiocatore == 1) {
					totImpostori++;
				}
      }
    }
    else {
      New->statoGiocatore = 0;
    }

    int nomeRandom;
    int nomeDuplicato;
		do {
			nomeDuplicato = 0;
			nomeRandom = (rand() % 10) + 1;
			for (int j = 0; j < numeroGiocatori; j++) {
				if (nomeRandom == nomiAssegnati[j]) {
					nomeDuplicato = 1;
					break;
				}
			}
		} while (nomeDuplicato);
		New->nomeGiocatore = nomeRandom;
		nomiAssegnati[i] = New->nomeGiocatore;

		New->posizione = stanza_inizio;

    if (*Last == NULL) {
      *First = New;
      *Last = New;
    }
    else {
      (*Last)->next = New;
      *Last = New;
    }
  }
	free(nomiAssegnati);

	return;
}

static void ins_stanza(struct Stanza **First, struct Stanza **Previous, int direzione) {
	struct Stanza* New = &lista_stanze[counterStanze];
	New->stanza_avanti = NULL;
	New->stanza_dx = NULL;
	New->stanza_sx = NULL;
	New->stanza_precedente = NULL;
	int token = (rand() % 100) + 1;
	if (token <= 30) {
		New->tipoStanza = 0;
	}
	else if (token <= 60) {
		New->tipoStanza = 1;
	}
	else if (token <= 75) {
		New->tipoStanza = 2;
	}
	else {
		New->tipoStanza = 3;
	}

	New->emergenza_chiamata = 0;

	if (counterStanze == 0) {
		sprintf(New->nomeStanza, "Stanza inizio");
	}
	else {
		sprintf(New->nomeStanza, "Stanza %d", counterStanze);
	}
	counterStanze++;

	if (!Previous) {
		*First = New;
	}
	else {
		New->stanza_precedente = *Previous;
		switch (direzione) {
			case 4:
				(*Previous)->stanza_sx = New;
				break;
			case 6:
				(*Previous)->stanza_dx = New;
				break;
			case 8:
				(*Previous)->stanza_avanti = New;
				break;
		}
	}
	return;
}

static void reset() {
	int scelta;
	do {
		printf("\nE' stato rilevato un gioco precedentemente impostato in memoria:\n premi 1 per impostarne uno nuovo o 2 per tornare al menu princiale\n");
		scanf("%d", &scelta);
		while(getchar() != '\n');
	} while(scelta != 1 && scelta != 2);
	switch (scelta) {
		case 1:
			free(giocatori);
			free(lista_stanze);
			numeroGiocatori = 0;
			counterStanze = 0;
			giocoImpostato = 0;
			printf("\nLe impostazioni sono state resettate, puoi ora impostare un nuovo gioco (premi Enter)\n");
			while(getchar() != '\n');
			break;
		case 2:
			printMenu();
			break;
	}
	return;
}

void gioca() {
	if (giocoImpostato != 1) {
		printf("\nE' necessario impostare il gioco prima di giocare.\n");
		printMenu();
		return;
	}
	else {
		int giocatore_eliminato = 0; //Check per do while

		do {
			giocatore_eliminato = 0;
			int giocatori_assassinati = 0;
			int giocatori_defenestrati = 0;
			for (int i = 0; i < numeroGiocatori; i++) {
				if (giocatori[i].statoGiocatore == assassinato) {
					giocatori_assassinati++;
				}
			}
			for (int i = 0; i < numeroGiocatori; i++) {
				if (giocatori[i].statoGiocatore == defenestrato) {
					giocatori_defenestrati++;
				}
			}

			giocatori_eliminati = giocatori_assassinati + giocatori_defenestrati;
			giocatori_effettivi = numeroGiocatori - giocatori_eliminati;

			int* turni = (int*) malloc(giocatori_effettivi * sizeof(int));
			int turnoRandom = 0, i = 0;
			while (i < giocatori_effettivi) {
				turnoRandom = rand() % (numeroGiocatori);
				int ok = 1;
				if (i > 0) {
					for (int j = 0; j < i; j++) {
						if (turnoRandom == turni[j]) {
							ok = 0;
							break;
						}
					}
				}
				if (ok != 0) {
					if (giocatori[turnoRandom].statoGiocatore == 2 || giocatori[turnoRandom].statoGiocatore == 3) {
						ok = 0;
					}
				}
				if (ok == 1) {
					turni[i] = turnoRandom;
					i++;
				}
			}
			for (int i = 0; i < giocatori_effettivi; i++) {
				printf("\nTurno %d: %s\n", i + 1, getNomeGiocatore(giocatori[turni[i]].nomeGiocatore));
			}
			while(getchar() != '\n');

			for (int i = 0; i < giocatori_effettivi; i++) {
				if (giocatore_eliminato == 1) {
					break;
				}
				printf("\nE' il turno del giocatore %s, %s.\n",
								getNomeGiocatore(giocatori[turni[i]].nomeGiocatore),
								getStatoGiocatore(giocatori[turni[i]].statoGiocatore));
				while(getchar() != '\n');
				printf("\nTi trovi nella stanza %s, di tipo %s.\n",
								getPosizione(giocatori[turni[i]].posizione),
								getTipoStanza(giocatori[turni[i]].posizione));
				while(getchar() != '\n');

				//Trova giocatori nella stanza
				int counter = 0;
				for (int j = 0; j < numeroGiocatori; j++) {
					if (giocatori[j].posizione == giocatori[turni[i]].posizione && giocatori[j].statoGiocatore != defenestrato && !(giocatori[j].statoGiocatore == assassinato && (giocatori[j].posizione)->emergenza_chiamata == 1)) {
						counter++;
					}
				}
				printf("\nNella stanza ci sono %d giocatori oltre a te:\n", counter - 1);
				struct Giocatore* giocatoriNellaStanza = (struct Giocatore*) malloc(counter * sizeof(struct Giocatore));

				int k = 0;
				while (k < counter) {
					for (int j = 0; j < numeroGiocatori; j++) {
						if (giocatori[j].posizione == giocatori[turni[i]].posizione && giocatori[j].statoGiocatore != defenestrato && !(giocatori[j].statoGiocatore == assassinato && (giocatori[j].posizione)->emergenza_chiamata == 1)) {
							giocatoriNellaStanza[k] = giocatori[j];
							k++;
						}
					}
				}
				for (int j = 0; j < counter; j++) {
					if (giocatoriNellaStanza[j].nomeGiocatore != giocatori[turni[i]].nomeGiocatore) {
						if (giocatoriNellaStanza[j].statoGiocatore == assassinato) {
							printf("%s (%s)\n", getNomeGiocatore(giocatoriNellaStanza[j].nomeGiocatore), getStatoGiocatore(giocatoriNellaStanza[j].statoGiocatore));
						}
						else {
							printf("%s\n", getNomeGiocatore(giocatoriNellaStanza[j].nomeGiocatore));
						}
					}
				}
				while(getchar() != '\n');

				int mortoNellaStanza = 0;
				for (int i = 0; i < counter; i++) {
					if (giocatoriNellaStanza[i].statoGiocatore == assassinato) {
						mortoNellaStanza = 1;
						break;
					}
				}
				int astronautaNellaStanza = 0;
				for (int i = 0; i < counter; i++) {
					if (giocatoriNellaStanza[i].statoGiocatore == astronauta) {
						astronautaNellaStanza = 1;
						break;
					}
				}
				int quest = 0;
				if ((giocatori[turni[i]].posizione)->tipoStanza == quest_semplice || (giocatori[turni[i]].posizione)->tipoStanza == quest_complicata) {
					quest = 1;
				}
				int botola = 0;

				if ((giocatori[turni[i]].posizione)->tipoStanza == 3 && trova_stanze_con_botola(giocatori[turni[i]].posizione) != NULL) {
					botola = 1;
				}

				if (mortoNellaStanza == 1 && (giocatori[turni[i]].posizione)->emergenza_chiamata != 1) {
					printf("\nATTENZIONE! E' stato rilevato un astronauta assassinato nella stanza!\n");
					while(getchar() != '\n');
				}

				int _opzioni = 1;
				if (mortoNellaStanza == 1 && (giocatori[turni[i]].posizione)->emergenza_chiamata != 1) {
					_opzioni++;
				}
				if (quest == 1) {
					_opzioni++;
				}
				if (giocatori[turni[i]].statoGiocatore == impostore) {
					if (botola == 1) {
						_opzioni++;
					}
					if (astronautaNellaStanza == 1) {
						_opzioni++;
					}
				}
				char** opzioni = (char**) malloc(_opzioni * sizeof(char*));
				opzioni[0] = "1 -> Avanza";
				if (mortoNellaStanza == 1 && (giocatori[turni[i]].posizione)->emergenza_chiamata != 1) {
					opzioni[1] = "2 -> Chiamata di emergenza";
					if (quest == 1) {
						if (giocatori[turni[i]].statoGiocatore == astronauta) {
							opzioni[2] = "3 -> Esegui quest";
						}
						else {
							opzioni[2] = "3 -> Sabotaggio";
							if (botola == 1) {
								opzioni[3] = "4 -> Usa botola";
								if (astronautaNellaStanza == 1) {
									opzioni[4] = "5 -> Uccidi astronauta";
								}
							}
							else {
								if (astronautaNellaStanza == 1) {
									opzioni[3] = "4 -> Uccidi astronauta";
								}
							}
						}
					}
					else {
						if (giocatori[turni[i]].statoGiocatore == impostore) {
							if (botola == 1) {
								opzioni[2] = "3 -> Usa botola";
								if (astronautaNellaStanza == 1) {
									opzioni[3] = "4 -> Uccidi astronauta";
								}
							}
							else {
								if (astronautaNellaStanza == 1) {
									opzioni[2] = "3 -> Uccidi astronauta";
								}
							}
						}
					}
				}
				else {
					if (quest == 1) {
						if (giocatori[turni[i]].statoGiocatore == astronauta) {
							opzioni[1] = "2 -> Esegui quest";
						}
						else {
							opzioni[1] = "2 -> Sabotaggio";
							if (botola == 1) {
								opzioni[2] = "3 -> Usa botola";
								if (astronautaNellaStanza == 1) {
									opzioni[3] = "4 -> Uccidi astronauta";
								}
							}
							else {
								if (astronautaNellaStanza == 1) {
									opzioni[2] = "3 -> Uccidi astronauta";
								}
							}
						}
					}
					else {
						if (giocatori[turni[i]].statoGiocatore == impostore) {
							if (botola == 1) {
								opzioni[1] = "2 -> Usa botola";
								if (astronautaNellaStanza == 1) {
									opzioni[2] = "3 -> Uccidi astronauta";
								}
							}
							else {
								if (astronautaNellaStanza == 1) {
									opzioni[1] = "2 -> Uccidi astronauta";
								}
							}
						}
					}
				}

				int scelta;
				int scelta_effettuata = 0;
				do {
					printf("\nCome vuoi procedere?\n");
					for (int i = 0; i < _opzioni; i++) {
						printf("%s\n", opzioni[i]);
					}
					scanf("%d", &scelta);
					while(getchar() != '\n');
					switch (scelta) {
						case 1:
							avanza(&giocatori[turni[i]]);
							scelta_effettuata = 1;
							break;
						case 2:
							if (mortoNellaStanza == 1 && (giocatori[turni[i]].posizione)->emergenza_chiamata != 1) {
								if (chiamata_emergenza(&giocatori[turni[i]]) == 1) {
									giocatore_eliminato = 1;
								}
								scelta_effettuata = 1;
								break;
							}
							else {
								if (quest == 1) {
									if (giocatori[turni[i]].statoGiocatore == astronauta) {
										esegui_quest(&giocatori[turni[i]]);
										scelta_effettuata = 1;
										break;
									}
									else {
										sabotaggio(&giocatori[turni[i]]);
										scelta_effettuata = 1;
										break;
									}
								}
								else {
									if (giocatori[turni[i]].statoGiocatore == impostore) {
										if (botola == 1) {
											usa_botola(&giocatori[turni[i]]);
											scelta_effettuata = 1;
											break;
										}
										else {
											if (astronautaNellaStanza == 1) {
												uccidi_astronauta(&giocatori[turni[i]]);
												scelta_effettuata = 1;
												giocatore_eliminato = 1;
												break;
											}
											else {
												printf("\nScelta non valida.\n");
												break;
											}
										}
									}
									else {
										printf("\nScelta non valida.\n");
										break;
									}
								}
							}
						case 3:
							if (mortoNellaStanza == 1) {
								if (quest == 1) {
									if (giocatori[turni[i]].statoGiocatore == astronauta) {
										esegui_quest(&giocatori[turni[i]]);
										scelta_effettuata = 1;
										break;
									}
									else {
										sabotaggio(&giocatori[turni[i]]);
										scelta_effettuata = 1;
										break;
									}
								}
								else {
									if (giocatori[turni[i]].statoGiocatore == impostore) {
										if (botola == 1) {
											usa_botola(&giocatori[turni[i]]);
											scelta_effettuata = 1;
											break;
										}
										else {
											if (astronautaNellaStanza == 1) {
												uccidi_astronauta(&giocatori[turni[i]]);
												scelta_effettuata = 1;
												giocatore_eliminato = 1;
												break;
											}
											else {
												printf("\nScelta non valida.\n");
												break;
											}
										}
									}
									else {
										printf("\nScelta non valida.\n");
										break;
									}
								}
							}
							else {
								if (quest == 1) {
									if (giocatori[turni[i]].statoGiocatore == impostore) {
										if (botola == 1) {
											usa_botola(&giocatori[turni[i]]);
											scelta_effettuata = 1;
											break;
										}
										else {
											if (astronautaNellaStanza == 1) {
												uccidi_astronauta(&giocatori[turni[i]]);
												scelta_effettuata = 1;
												giocatore_eliminato = 1;
												break;
											}
											else {
												printf("\nScelta non valida.\n");
												break;
											}
										}
									}
									else {
										printf("\nScelta non valida.\n");
										break;
									}
								}
								else {
									if (giocatori[turni[i]].statoGiocatore == impostore && botola == 1 && astronautaNellaStanza == 1) {
										uccidi_astronauta(&giocatori[turni[i]]);
										scelta_effettuata = 1;
										giocatore_eliminato = 1;
										break;
									}
									else {
										printf("\nScelta non valida.\n");
										break;
									}
								}
							}
						case 4:
							if (mortoNellaStanza == 1) {
								if (quest == 1) {
									if (giocatori[turni[i]].statoGiocatore == impostore) {
										if (botola == 1) {
											usa_botola(&giocatori[turni[i]]);
											scelta_effettuata = 1;
											break;
										}
										else {
											if (astronautaNellaStanza == 1) {
												uccidi_astronauta(&giocatori[turni[i]]);
												scelta_effettuata = 1;
												giocatore_eliminato = 1;
												break;
											}
											else {
												printf("\nScelta non valida.\n");
												break;
											}
										}
									}
									else {
										printf("\nScelta non valida.\n");
										break;
									}
								}
								else {
									if (giocatori[turni[i]].statoGiocatore == impostore && botola == 1 && astronautaNellaStanza == 1) {
										uccidi_astronauta(&giocatori[turni[i]]);
										scelta_effettuata = 1;
										giocatore_eliminato = 1;
										break;
									}
									else {
										printf("\nScelta non valida.\n");
										break;
									}
								}
							}
							else {
								if (quest == 1 && giocatori[turni[i]].statoGiocatore == impostore && botola == 1 && astronautaNellaStanza == 1) {
									uccidi_astronauta(&giocatori[turni[i]]);
									scelta_effettuata = 1;
									giocatore_eliminato = 1;
									break;
								}
								else {
									printf("\nScelta non valida.\n");
									break;
								}
							}
						case 5:
							if (mortoNellaStanza == 1 && quest == 1 && giocatori[turni[i]].statoGiocatore == impostore && botola == 1 && astronautaNellaStanza == 1) {
								uccidi_astronauta(&giocatori[turni[i]]);
								scelta_effettuata = 1;
								giocatore_eliminato = 1;
								break;
							}
							else {
								printf("\nScelta non valida.\n");
								break;
							}
						default:
							printf("\nScelta non valida.\n");
							break;
					}
					if (giocatore_eliminato == 1 || vittoria == 1) {
						break;
					}
				} while(scelta_effettuata != 1 || (scelta != 1 && scelta != 2 && scelta != 3 && scelta != 4 && scelta != 5));

				if (i == giocatori_effettivi - 1) {
					i = -1;
				}
				free(giocatoriNellaStanza);
				free(opzioni);
				if (vittoria == 1) {
					break;
				}
			}
			if (vittoria == 1) {
				break;
			}
		} while(giocatore_eliminato == 1);
		return;
	}
}

static void avanza(struct Giocatore* giocatore) {
	int direzione = 0;
	do {
		printf("\nIn che direzione vuoi avanzare\n 2 -> Rimani fermo\n 4 -> Sinistra\n 6 -> Destra\n 8 -> Avanti\n");
		scanf("%d", &direzione);
		while(getchar() != '\n');
		switch (direzione) {
			case 2:
				break;
			case 4:
				if ((giocatore->posizione)->stanza_sx == NULL) {
					ins_stanza(&stanza_inizio, &(giocatore->posizione), direzione);
				}
				giocatore->posizione = (giocatore->posizione)->stanza_sx;
				break;
			case 6:
				if ((giocatore->posizione)->stanza_dx == NULL) {
					ins_stanza(&stanza_inizio, &(giocatore->posizione), direzione);
				}
				giocatore->posizione = (giocatore->posizione)->stanza_dx;
				break;
			case 8:
				if ((giocatore->posizione)->stanza_avanti == NULL) {
					ins_stanza(&stanza_inizio, &(giocatore->posizione), direzione);
				}
				giocatore->posizione = (giocatore->posizione)->stanza_avanti;
				break;
			default:
				printf("\nScelta non valida.\n");
				break;
		}
	} while(direzione != 2 && direzione != 4 && direzione != 6 && direzione != 8);

	if (direzione != 2) {
		printf("\nTi trovi ora nella stanza %s, di tipo %s. (Premi Enter per passare al turno successivo)\n", getPosizione(giocatore->posizione), getTipoStanza(giocatore->posizione));
	}
	else {
		printf("\nTi trovi ancora nella stanza %s, di tipo %s. (Premi Enter per passare al turno successivo)\n", getPosizione(giocatore->posizione), getTipoStanza(giocatore->posizione));
	}
	while(getchar() != '\n');
	return;
}

static void esegui_quest(struct Giocatore* giocatore) {
	int quest_risolta;
	if ((giocatore->posizione)->tipoStanza == quest_semplice) {
		int a = rand() % 10, b = rand() % 10, risultato;
		printf("\nQual e' il risultato di %d X %d?\n", a, b);
		scanf("%d", &risultato);
		if (risultato == a * b) {
			quest_risolta = 1;
		}
	}
	else {
		enum Stati stato = rand() % 5;
		if (stato < 3) {
			printf("\nQual e' la capitale dell'%s?\n", getStato(stato));
		}
		else if (stato == 3) {
			printf("\nQual e' la capitale della %s?\n", getStato(stato));
		}
		else {
			printf("\nQual e' la capitale di %s?\n", getStato(stato));
		}
		for (int i = 0; i < 5; i++) {
			printf("%d -> %s\n", i + 1, getCapitale(i));
		}
		int risultato;
		scanf("%d", &risultato);
		while(getchar() != '\n');
		if (risultato - 1 == stato) {
			quest_risolta = 1;
		}
	}

	if (giocatore->statoGiocatore == astronauta) {
			if (quest_risolta == 1) {
				if ((giocatore->posizione)->tipoStanza == quest_semplice) {
					quest_da_finire--;
				}
				else {
					if (quest_da_finire > 1) {
						quest_da_finire -= 2;
					}
					else {
						quest_da_finire--;
					}
				}
				for (int i = 0; i < counterStanze; i++) {
					if (strcmp(lista_stanze[i].nomeStanza, (giocatore->posizione)->nomeStanza) == 0) {
						lista_stanze[i].tipoStanza = vuota;
					}
				}
				if (quest_da_finire > 0) {
					printf("\nQuest eseguita con successo. (Premi Enter per passare al turno successivo)\n");
					while(getchar() != '\n');
					printf("\nRimangono ancora %d quest da finire.\n", quest_da_finire);
					while(getchar() != '\n');
				}
				else {
					//VITTORIA DEGLI ASTRONAUTI!
					vittoria = 1;
					printf("\nQuest eseguita con successo.\n");
					while(getchar() != '\n');
					printf("\nCOMPLIMENTI %s! Le quest sono terminate e la tua squadra ha vinto!\n", getNomeGiocatore(giocatore->nomeGiocatore));
					while(getchar() != '\n');
					termina_gioco();
				}
			}
			else {
				printf("\nRisposta sbagliata! La quest non e' ancora stata risolta. (Premi Enter)\n");
				while(getchar() != '\n');
			}
	}
	else {
		if (quest_risolta == 1) {
			(giocatore->posizione)->tipoStanza = vuota;
		}
	}

	return;
}

static char* getStato(enum Stati stato) {
	char* nome_stato;
	switch (stato) {
		case Estonia: nome_stato = "Estonia"; break;
		case Ucraina: nome_stato = "Ucraina"; break;
		case Australia: nome_stato = "Australia"; break;
		case Turchia: nome_stato = "Turchia"; break;
		case Malta: nome_stato = "Malta"; break;
	}
	return nome_stato;
}
static char* getCapitale(enum Stati stato) {
	char* capitale;
	switch (stato) {
		case Estonia: capitale = "Tallin"; break;
		case Ucraina: capitale = "Kiev"; break;
		case Australia: capitale = "Canberra"; break;
		case Turchia: capitale = "Ankara"; break;
		case Malta: capitale = "La Valletta"; break;
	}
	return capitale;
}

static int chiamata_emergenza(struct Giocatore* giocatore) {
	for (int i = 0; i < counterStanze; i++) {
		if (strcmp(lista_stanze[i].nomeStanza, (giocatore->posizione)->nomeStanza) == 0) {
			lista_stanze[i].emergenza_chiamata = 1;
		}
	}

	int astronauti = 0, impostori = 0;
	for (int i = 0; i < numeroGiocatori; i++) {
		if (giocatori[i].statoGiocatore == astronauta && giocatori[i].posizione == giocatore->posizione) {
			astronauti++;
		}
	}
	for (int i = 0; i < numeroGiocatori; i++) {
		if (giocatori[i].statoGiocatore == impostore && giocatori[i].posizione == giocatore->posizione) {
			impostori++;
		}
	}

	int prob_astronauti = 0, prob_impostori = 0;
	if (astronauti > 0) {
		prob_astronauti = 30 + (impostori * 20) - ((astronauti - 1) * 30);
	}
	if (impostori > 0) {
		prob_impostori = 30 + (astronauti * 20) - ((impostori - 1) * 30);
	}

	int defenestrato_random;
	do {
		defenestrato_random = rand() % numeroGiocatori;
	} while(giocatori[defenestrato_random].statoGiocatore == 2 || giocatori[defenestrato_random].statoGiocatore == 3 || giocatori[defenestrato_random].posizione != giocatore->posizione);

	printf("\nE' stato sorteggiato il giocatore %s, %s.\n", getNomeGiocatore(giocatori[defenestrato_random].nomeGiocatore), getStatoGiocatore(giocatori[defenestrato_random].statoGiocatore));
	while(getchar() != '\n');

	int token = (rand() % 100) + 1, eliminato = 0;
	if (giocatori[defenestrato_random].statoGiocatore == astronauta) {
		printf("\n%s ha il %d%% di probabilita' di essere defenestrato\n", getNomeGiocatore(giocatori[defenestrato_random].nomeGiocatore), prob_astronauti);
		while(getchar() != '\n');
		if (token <= prob_astronauti) {
			giocatori[defenestrato_random].statoGiocatore = defenestrato;
			eliminato = 1;
		}
	}
	else {
		printf("\n%s ha il %d%% di probabilita' di essere defenestrato\n", getNomeGiocatore(giocatori[defenestrato_random].nomeGiocatore), prob_impostori);
		while(getchar() != '\n');
		if (token <= prob_impostori) {
			giocatori[defenestrato_random].statoGiocatore = defenestrato;
			eliminato = 1;
			if (impostori - 1 == 0) {
				vittoria = 1;
			}
		}
	}

	if (eliminato == 1) {
		printf("\nIl giocatore %s e' stato %s a seguito della chiamata d'emergenza. (Premi Enter)\n", getNomeGiocatore(giocatori[defenestrato_random].nomeGiocatore), getStatoGiocatore(giocatori[defenestrato_random].statoGiocatore));
		while(getchar() != '\n');
		if (vittoria == 1) {
			if (giocatore->statoGiocatore == astronauta) {
				printf("\nCOMPLIMENTI %s! Grazie alla tua chiamata e' stato eliminato l'ultimo impostore e la tua squadra ha vinto!\n", getNomeGiocatore(giocatore->nomeGiocatore));
				while(getchar() != '\n');
			}
			else {
				printf("\nMi dispiace, %s! Eri l'ultimo impostore rimasto e la tua squadra ha perso. Gli astronauti hanno vinto!\n", getNomeGiocatore(giocatore->nomeGiocatore));
				while(getchar() != '\n');
			}
			termina_gioco();
		}
	}
	else {
		printf("\nNessuno e' stato defenestrato a seguito della chiamata d'emergenza. (Premi Enter)\n");
		while(getchar() != '\n');
	}

	return eliminato;
}

static void uccidi_astronauta(struct Giocatore* giocatore) {
	//Conta e trova astronauti nella stanza
	struct Giocatore* astronauti_nella_stanza;
	int astronauti = 0;
	for (int i = 0; i < numeroGiocatori; i++) {
		if (giocatori[i].statoGiocatore == astronauta && giocatori[i].posizione == giocatore->posizione) {
			astronauti++;
		}
	}

	if (astronauti > 0) {
		astronauti_nella_stanza = (struct Giocatore*) malloc(astronauti * sizeof(struct Giocatore));
	}
	else {
		astronauti_nella_stanza = NULL;
	}

	int i = 0;
	while (i < astronauti) {
		for (int j = 0; j < numeroGiocatori; j++) {
			if (giocatori[j].statoGiocatore == astronauta && giocatori[j].posizione == giocatore->posizione) {
				astronauti_nella_stanza[i] = giocatori[j];
				i++;
			}
		}
	}

	//Conta astronauti nella stanza precedente
	int astronauti_st_prec = 0;
	for (int i = 0; i < numeroGiocatori; i++) {
		if (giocatori[i].statoGiocatore == astronauta && giocatori[i].posizione == (giocatore->posizione)->stanza_precedente) {
			astronauti_st_prec++;
		}
	}

	//Uccidi
	int scelta, astronauta_assassinato;
	do {
		printf("\nScegli un astronauta da uccidere\n");
		for (int i = 0; i < astronauti; i++) {
			printf("%d -> %s\n", i + 1, getNomeGiocatore(astronauti_nella_stanza[i].nomeGiocatore));
		}
		scanf("%d", &scelta);
		if (scelta > 0 && scelta <= astronauti) {
			for (int i = 0; i < numeroGiocatori; i++) {
				if (giocatori[i].nomeGiocatore == astronauti_nella_stanza[scelta - 1].nomeGiocatore) {
					giocatori[i].statoGiocatore = assassinato;
					astronauta_assassinato = 1;
					printf("\nIl giocatore %s e' stato %s. (Premi Enter)\n", getNomeGiocatore(giocatori[i].nomeGiocatore), getStatoGiocatore(giocatori[i].statoGiocatore));
					while(getchar() != '\n');
					//Sei stato scoperto?
					int token, impostore_scoperto = 0;
					switch (astronauti - 1) {
						case 0:
							switch (astronauti_st_prec) {
								case 0:
									break;
								case 1:
									token = (rand() % 100) + 1;
									if (token <= 20) {
										impostore_scoperto = 1;
									}
									break;
								case 2:
									token = (rand() % 100) + 1;
									if (token <= 40) {
										impostore_scoperto = 1;
									}
									break;
								case 3:
									token = (rand() % 100) + 1;
									if (token <= 60) {
										impostore_scoperto = 1;
									}
									break;
								case 4:
									token = (rand() % 100) + 1;
									if (token <= 80) {
										impostore_scoperto = 1;
									}
									break;
								default:
									impostore_scoperto = 1;
									break;
							}
							break;
						case 1:
							switch (astronauti_st_prec) {
								case 0:
									token = (rand() % 100) + 1;
									if (token <= 50) {
										impostore_scoperto = 1;
									}
									break;
								case 1:
									token = (rand() % 100) + 1;
									if (token <= 70) {
										impostore_scoperto = 1;
									}
									break;
								case 2:
									token = (rand() % 100) + 1;
									if (token <= 90) {
										impostore_scoperto = 1;
									}
									break;
								default:
									impostore_scoperto = 1;
									break;
							}
							break;
						default:
							impostore_scoperto = 1;
							break;
					}
					if (impostore_scoperto == 1) {
						for (int j = 0; j < numeroGiocatori; j++) {
							if (giocatori[j].nomeGiocatore == giocatore->nomeGiocatore) {
								giocatori[j].statoGiocatore = defenestrato;
								for (int k = 0; k < counterStanze; k++) {
									if (strcmp(lista_stanze[k].nomeStanza, (giocatore->posizione)->nomeStanza) == 0) {
										lista_stanze[k].emergenza_chiamata = 1;	//Se l'impostore viene defenestrato, non si può più fare chiamata d'emergenza nella stanza
									}
								}
							}
						}
						printf("\nSei stato scoperto e di conseguenza %s. (Premi Enter)\n", getStatoGiocatore(giocatore->statoGiocatore));
						while(getchar() != '\n');
						int impostori = 0;
						for (int i = 0; i < numeroGiocatori; i++) {
							if (giocatori[i].statoGiocatore == impostore && giocatori[i].posizione == giocatore->posizione) {
								impostori++;
							}
						}
						if (impostori == 0) {
							vittoria = 1;
							printf("\nMi dispiace %s, eri l'ultimo impostore rimasto. La tua squadra ha perso. Gli astronauti hanno vinto! (Premi Enter)\n", getNomeGiocatore(giocatore->nomeGiocatore));
							while(getchar() != '\n');
							termina_gioco();
							return;
						}
					}
					//
				}
			}
		}
		else {
			printf("\nScelta non valida.\n");
			while(getchar() != '\n');
		}
	} while(astronauta_assassinato != 1);

	//Verifica astronauti rimasti nell'astronave
	int astronauti_rimasti = 0;
	for (int i = 0; i < numeroGiocatori; i++) {
		if (giocatori[i].statoGiocatore == astronauta) {
			astronauti_rimasti++;
		}
	}
	if (astronauti_rimasti == 0) {
		//VITTORIA DEGLI IMPOSTORI
		vittoria = 1;
		printf("\nCOMPLIMENTI %s! Hai ucciso l'ultimo astronauta rimasto e la tua squadra ha vinto!\n", getNomeGiocatore(giocatore->nomeGiocatore));
		while(getchar() != '\n');
		termina_gioco();
	}

	return;
}

static void usa_botola(struct Giocatore* giocatore) {
	struct Stanza* stanze_con_botola = trova_stanze_con_botola(giocatore->posizione);
	int* opzioni = (int*) malloc(conta_botole(giocatore->posizione) * sizeof(int));

	int scelta, spostamento_effettuato;
	do {
		printf("\nIn quale stanza ti vuoi spostare?\n");
		for (int i = 0; i < conta_botole(giocatore->posizione); i++) {
			if (stanze_con_botola[i].nomeStanza != (giocatore->posizione)->nomeStanza) {
				printf("%d -> %s\n", i + 1, getPosizione(&(stanze_con_botola[i])));
				opzioni[i] = i + 1;
			}
		}
		scanf("%d", &scelta);
		while(getchar() != '\n');
		for (int i = 0; i < conta_botole(giocatore->posizione); i++) {
			if (scelta == opzioni[i]) {
				for (int j = 0; j < numeroGiocatori; j++) {
					for (int k = 0; k < counterStanze; k++) {
						if (giocatori[j].nomeGiocatore == giocatore->nomeGiocatore && strcmp(lista_stanze[k].nomeStanza, stanze_con_botola[i].nomeStanza) == 0) {
							giocatori[j].posizione = &lista_stanze[k];
						}
					}
				}
				spostamento_effettuato = 1;
			}
		}
		if (spostamento_effettuato == 1) {
			printf("\nTi trovi ora nella stanza %s. (Premi Enter per passare al turno successivo)\n", getPosizione(giocatore->posizione));
			while(getchar() != '\n');
		}
		else {
			printf("\nScelta non valida.\n");
		}
	} while(spostamento_effettuato != 1);

	free(stanze_con_botola);
	free(opzioni);
	return;
}

static struct Stanza* trova_stanze_con_botola(struct Stanza* stanza_giocatore) {
	struct Stanza* stanze_con_botola;
	if (conta_botole(stanza_giocatore) > 0) {
		stanze_con_botola = (struct Stanza*) malloc(conta_botole(stanza_giocatore) * sizeof(struct Stanza));
	}
	else {
		stanze_con_botola = NULL;
	}
	int i = 0;
	while (i < conta_botole(stanza_giocatore)) {
		for (int j = 0; j < counterStanze; j++) {
			if (lista_stanze[j].tipoStanza == 3 && lista_stanze[j].nomeStanza != (stanza_giocatore->nomeStanza)) {
				stanze_con_botola[i] = lista_stanze[j];
				i++;
			}
		}
	}
	return stanze_con_botola;
}

static int conta_botole(struct Stanza* stanza_giocatore) {
	int botole = 0;
	for (int j = 0; j < counterStanze; j++) {
		if (lista_stanze[j].tipoStanza == 3 && lista_stanze[j].nomeStanza != (stanza_giocatore->nomeStanza)) {
			botole++;
		}
	}
	return botole;
}

static void sabotaggio(struct Giocatore* giocatore) {
	printf("\nPer poter procedere al sabotaggio della stanza, devi risolvere tu la Quest. (Premi Enter)\n");
	while(getchar() != '\n');
	esegui_quest(giocatore);
	if ((giocatore->posizione)->tipoStanza == vuota) {
		printf("\nSabotaggio riuscito! La stanza %s ora e' vuota. (Premi Enter per passare al turno successivo)\n", getPosizione(giocatore->posizione));
		while(getchar() != '\n');
	}
	else {
		printf("\nRisposta sbagliata. Sabotaggio non effettuato. (Premi Enter)\n");
		while(getchar() != '\n');
	}
	return;
}


void termina_gioco() {
	if (giocoImpostato != 0) {
		free(giocatori);
		free(lista_stanze);
		counterStanze = 0;
		giocoImpostato = 0;
	}
	printf("\nTorna presto Trannoi!\n");
	while(getchar() != '\n');
	return;
}
