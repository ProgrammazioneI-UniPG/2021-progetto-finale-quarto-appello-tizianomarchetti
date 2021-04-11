//Enum
enum Stato_giocatore { astronauta, impostore, assassinato, defenestrato };
enum Nome_giocatore {
	rosso = 1,
	blu,
	giallo,
	verde,
	marrone,
	nero,
	bianco,
	arancio,
	rosa,
	viola
};
enum Tipo_stanza { vuota, quest_semplice, quest_complicata, botola };

enum Stati { Estonia, Ucraina, Australia, Turchia, Malta };	//Enum non richiesto, per risoluzione Quest complicata

//Struct
struct Giocatore {
	struct Stanza* posizione;
	enum Stato_giocatore statoGiocatore;
	enum Nome_giocatore nomeGiocatore;
  struct Giocatore *next;
};

struct Stanza {
	struct Stanza* stanza_avanti;
	struct Stanza* stanza_dx;
	struct Stanza* stanza_sx;
	struct Stanza* stanza_precedente;
	enum Tipo_stanza tipoStanza;
	int emergenza_chiamata;
	char nomeStanza[20];	//Voce aggiunta per identificare univocamente la stanza
};

//Funzioni

void imposta_gioco();
void gioca();
void termina_gioco();
void printMenu();
