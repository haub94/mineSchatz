/*[===============INFORMATIONEN=========================================================]
 Programmname: mineSCHATZ
 Autor: Markus Haubold
 Datum_Version: 2022-01-30_V1.0

[===============INCLUDE===============================================================]*/
#include <stdio.h>  /*Ein-/ Ausgabe*/
#include <stdlib.h> /*Standardlib*/
#include <string.h> /*Arbeiten mit Strings*/
#include <time.h>   /*Zufallszahl*/
#include <unistd.h> /*sleep()*/
#include <conio.h>  /*getch()*/

/*[===============DEFINE==============================================================]*/
#define JA 1                /*JA-Abfragen*/
#define NEIN 0              /*NEIN-Abfragen*/
#define FELDER_ZEILE 14     /*Kartenfelder_Zeile = 14 Felder horizontal (Spalten)*/
#define FELDER_SPALTE 14    /*Kartenfelder_Spalte =14 Felder vertikal (Zeilen)*/
#define ZEILE 1             /*Index1 für aktPos & vorPos */
#define SPALTE 2            /*Index2 für aktPos & vorPos*/
#define W 101               /*Abfrage Taste w*/
#define A 102               /*Abfrage Taste a*/
#define S 103               /*Abfrage Taste s*/
#define D 104               /*Abfrage Taste d*/
#define NEUSTART 99         /*Abfrage Neustart*/
#define ENTER 105           /*Abfrage Enter-Taste*/
#define MAX_SCHAETZE 10     /*Anzahl der Schätze, die maximal auf Karte versteckt sind*/
#define MAX_SCHATZGRSE 8    /*maximale Größe der einzelnen Schätze (zeile*spalte)*/
#define MAX_VERSUCHE 30     /*maxmimale Anzahl an Versuchen zum Graben*/
#define LISTENFELDER 64     /*Speicher aller Koordinaten des größtmgl. Schatzes (8*8)*/
#define TEXTLAENGE 41+1     /*maximale Länge des Hinweistextes (41 Zeichen + \0)*/
#define MAX_BUCHSTABEN 16   /*Anzahl Buchstaben für Anzeige "DU HAST GEWONNEN/VERLOREN"*/
#define ZP_LEVEL 1          /*Parameter für eingabe() -> Eingabe Level*/
#define ZP_SPIELEN 2        /*Parameter für eingabe() -> Eingabe während Spielens*/
#define ZP_ENDE 3           /*Parameter für eingabe() -> Eingabe nochmal ja/nein*/
#define FEHLER 999          /*etwas ist nicht korrekt abgelaufen*/

/*[===============STRUKTUREN==========================================================]*/
/*in felder kann die Position auf Karte gespeichert werden*/
struct feld
{
    int posZeile;
    int posSpalte;
    char buchstabe;
};
/*feldliste enthält struktur feld --> mehrere Felder in einer Liste*/
struct feldliste
{
    struct feld feld[LISTENFELDER];
};
 
/*[===============VARIABLEN===========================================================]*/
/*kartenspezifisch*/
char *karte[FELDER_SPALTE][FELDER_ZEILE];   /*2D-Array = Spielfeld*/
                                           
/*allg. Parameter*/
char infotext[TEXTLAENGE];                  /*char-array, in welchem der Infotext steht*/         
int level = 0;                              /*aktuell gewähltes Level*/
int anzSchaetze = 0;                        /*Gesamtanzahl zu findender Schätze*/
int gefSchaetze = 0;                        /*Anzahl bereits gefundener Schätze*/
int restVersuche = 0;                       /*noch zu suchende Schätze (wird beim Start
                                             *mit anzSchaetze beschrieben und dann 
                                             *nach jedem Finden verringert*/
int aktSchatzgroesse = 0;                   /*Anzahl Felder,die einen Schatz definieren
                                             *Größe=8 --> 1 Schatz = 8x8 Felder*/

/*Strukturen erzeugen*/
struct feldliste schatz[MAX_SCHAETZE];      
struct feldliste gefunden[MAX_SCHAETZE];
struct feld leergegraben[MAX_VERSUCHE];
struct feld statusAufKarte[MAX_BUCHSTABEN]; /*Enthält Koordinaten und Buchstaben für "DU
                                             * HAST GEWONNEN/VERLOREN" (Fkt noch offen)*/

/*Steuerung*/
int taste = 0;                              /*betätigte Taste als int-Wert*/      
int aktPos[2];                              /*aktuelle position auf der Karte -> 
                                             *wie Koordinatensystem vorstellen:
                                             *Index1=Zeile=y, Index2=Spalte=x*/
                                             

/*[===============ENTWICKLUNG=========================================================]*/
int const entwicklungOn = NEIN; /*aktivieren/deaktivieren von Funktionen, 
                                 * welche die Entwicklung vereinfachen*/

/*[===============PROTOTYPEN==========================================================]*/
/*Deklaration der Prototypen*/
void entwicklung(void);
void intro(void);
void spielparameter(void);
void layout(void);
int eingabe(int);
void bewegen(void);
void graben(void);
void hinweis(int);
int freiFeld(int, int);
void outro(void);
void fuelleStatusAufKarte(int);

/*[===============DEFINITION DER FUNKTIONEN===========================================]*/
void intro(void)
{
/*+-------------------------------------------------------------------------------------+
Name: intro
Autor: Markus Haubold
Zustand: i.O. 
Funktionsbeschreibung: 
    - zeigt die Begrüßung an und fordert die Benutzer*in auf, eine Schwierigkeit zu wählen  
Datum_Version: 2022-01-30_V1.0
ToDo: //
+--------------------------------------------------------------------------------------*/
    /*Konsole bereinigen*/
    system("cls");
   /*Begrüßung & Erklärung des Spiels*/
    printf("\n+-----------------------------------------------------+");
    printf("\n|         WILLKOMMEN ZUM SPIEL \"mine-Schatz\"          |\n");
    printf("+-----------------------------------------------------+\n");
    printf("| Soso...du willst dich also als ein Schatzsuchender  |\n");
    printf("| testen und das grosse Nugget finden?! Dann waehle   |\n");
    printf("| zuerst ein Level, was zu deinen Faehigkeiten passt! |\n");
    printf("|                                                     |\n");
    printf("|               Schwierigkeitsstufen                  |\n"); 
    printf("|     1-leicht  2-mittel  3-schwer  4-kein Spass!     |\n");
    printf("|                                                     |\n");
    printf("|    Mit der ESC-Taste kannst du das Spiel beenden    |\n");
    printf("+-----------------------------------------------------+\n");       

    return;
}
void entwicklung(void)
{
/*+-------------------------------------------------------------------------------------+
Name:entwicklung
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
    - wird nur während der Zeit der Spielentwicklung benötigt und kann mittels 
      Variable entwiklungOn=NEIN deaktiviert werden
    - Variablen die sonst anhand des gewählten Schwierigkeitsgrades bestimmt
      werden, können so manuell festgelegt werden
    - kann auch als Testbereich genutzt werden
Datum_Version: 2022-01-30_V1.0
ToDo: //
+-------------------------------------------------------------------------------------+*/
    
    /*Parameter schreiben*/
    /*restVersuche = 100;
    anzSchaetze = 10;
    gefSchaetze = 0;
    aktSchatzgroesse = 8;*/

    /*Testen, ob Schatz als Blog ausgegeben wird*/
    /*
    schatz[0].feld[0].posZeile = 11;
    schatz[0].feld[0].posSpalte = 4;
    schatz[0].feld[1].posZeile = 10;
    schatz[0].feld[1].posSpalte = 4;
    schatz[0].feld[2].posZeile = 11;
    schatz[0].feld[2].posSpalte = 5;
    schatz[0].feld[3].posZeile = 10;
    schatz[0].feld[3].posSpalte = 5;
    */   
    
    return;
}
void spielparameter(void)
{
/*+-------------------------------------------------------------------------------------+
Name: spielparameter
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
    - Initialisierung der Listen und rücksetzen der Spielparameter
    - entsprechend der gewählten Schwierigkeitsstufe, werden die Spielparameter
      festgelegt (anzSchaetze, aktSchatzgroesse, restVersuche)
    - somit werden zufällig die Schätze in entsprechender Größe auf Karte verteilt
    - Startposition (unten Links: Zeile 14 Spalte 1) wird auf aktPos geschrieben
Datum_Version: 2022-01-30_V1.0
ToDo: //
+--------------------------------------------------------------------------------------*/
    int startSchatzZeile = 0, startSchatzSpalte = 0, schatzNr = 0, feldNr = 0, 
        zeileNr = 0, speicherplatz = 0;
    
    /*Listen zurücksetzen -> Wichtig für erneuten Durchlauf des Spiels*/
    /*Schatzliste (angelegt und gefunden) leeren*/
    for(schatzNr = 0; schatzNr < MAX_SCHAETZE; schatzNr++)
    {
        for(feldNr = 0; feldNr < LISTENFELDER; feldNr++)
        {
            schatz[schatzNr].feld[feldNr].posZeile = 0;
            schatz[schatzNr].feld[feldNr].posSpalte = 0;
            gefunden[schatzNr].feld[feldNr].posZeile = 0;
            gefunden[schatzNr].feld[feldNr].posSpalte = 0;
        }
    }
    /*Liste Leerversuche-Graben leeren*/
    for(feldNr = 0; feldNr < MAX_VERSUCHE; feldNr++)
    {
        leergegraben[feldNr].posSpalte = 0;
        leergegraben[feldNr].posZeile = 0;
    }
    /*Spielparameter zurücksetzen*/
    gefSchaetze = 0;
    anzSchaetze = 0;
    restVersuche = 0;
    
    /*Parameter entsprechend Schwierigkeitslevel beschreiben*/
    switch (level)
    {
        case 1: /*Schwierigkeit = leicht*/
            restVersuche = 5;
            anzSchaetze = 1;
            aktSchatzgroesse = 8;
            break;

         case 2: /*Schwierigkeit = mittel*/
            restVersuche = 10;
            anzSchaetze = 2;
            aktSchatzgroesse = 4;
            break;
        
         case 3: /*Schwierigkeit = schwer*/
            restVersuche = 25;
            anzSchaetze = 5;
            aktSchatzgroesse = 2;
            break;
        
         case 4: /*Schwierigkeit = kein Spaß*/
            restVersuche = MAX_VERSUCHE;
            anzSchaetze = 10;
            aktSchatzgroesse = 1;
            break;

        default:
            break;
    }

    /*Startposition unten links -> 1.Spalte in letzter Zeile*/
    aktPos[ZEILE] = 14; 
    aktPos[SPALTE] = 1;  

    /*Schätze generieren*/
    /*Position zufällig für Zeile und Spalte bestimmen und prüfen, ob Position
     *verfügbar ist -> freiFeld()*/
    for(schatzNr = 0; schatzNr < anzSchaetze; schatzNr++)
    {
        generiere:
        startSchatzZeile = freiFeld((rand()%14+1), ZEILE);     
        startSchatzSpalte = freiFeld((rand()%14+1), SPALTE);
        /*wenn keine Position gefunden werden konnte (=FEHLER=999) nochmal neu erzeugen*/
        if((startSchatzZeile == FEHLER) || (startSchatzSpalte == FEHLER)) goto generiere;
        /*Schatz in entsprechende Felder in Liste schreiben*/
        for(zeileNr = 0; zeileNr < aktSchatzgroesse; zeileNr++)
        {
            for(feldNr = 0; feldNr < aktSchatzgroesse; feldNr++)
            {
                schatz[schatzNr].feld[speicherplatz].posZeile = 
                startSchatzZeile + zeileNr;
                
                schatz[schatzNr].feld[speicherplatz].posSpalte = 
                startSchatzSpalte + feldNr;
               
                speicherplatz++;
            }
        }
    }
    hinweis(1); /*Starthinweis schreiben: Auf gehts...*/

    return;
}
void layout(void)
{
/*+-------------------------------------------------------------------------------------+
Name: layout
Autor: Markus Haubold
Zustand: i.O. aber sekundäre ToDo´s
Funktionsbeschreibung: 
    - es werden die Spielrelevanten Infos angezeigt
    - es wird ein zweidimensionales array mit KARTE_spalte * KARTE_zeile
      Feldern erzeugt
    - entsprechend der Benutzeraktivität werden die Felder mit entsprechenden
      Symbolen beschrieben
    - dabei wird prinzipiell jedes Feld der Reihe nach einzeln durchlaufen und mit den 
      Listen abgeglichen, so wird erkannt, ob das Feld einen Schatz enthält, dort schon 
      einmal gegraben wurde oder der Benutzende sich aktuell auf dem Feld befindet -> 
      entsprechend wird das Feld mit einem Zeichen beschrieben  
Datum_Version: 2022-01-30_V1.0
ToDo: //
+-------------------------------------------------------------------------------------+*/
    int zeile, spalte, schatznr, feldnr, leernr;
    char einzugAnzSch[2+1]={"  "}, einzugGefSch[2+1]={"  "}, einzugRestVer[2+1]={"  "};
    /*char zeichenInKartenformat[4+1]; int buchstabenr für Fkt. die momentan noch
     * nicht genutzt wird*/

    /*Einzug zum Rand muss bei Zahlen >=10 verringert werden,sonst verschiebt sich Rand*/
    if(anzSchaetze == 10) strcpy(einzugAnzSch, " ");
    if(gefSchaetze >= 10) strcpy(einzugGefSch, " ");
    if(restVersuche >= 10) strcpy(einzugRestVer, " ");

    /*Konsole leeren*/
    system("cls"); 

    /*Kopfzeile mit allen zum Spielen benötigten Informationen*/
    printf("+-----------------------------------------------------+");
    printf("\n|       VIEL ERFOLG BEIM FINDEN DER SCHAETZE!         |\n");
    printf("+-----------------------------------------------------+\n");
    printf("| Schwierigkeitsgrad:                                %d|\n", level);
    printf("| Insgesamt zu findende Schatze:                   %s%d|\n",einzugAnzSch, 
                                                                        anzSchaetze);
    printf("| Davon bereits gefunden:                          %s%d|\n",einzugGefSch, 
                                                                        gefSchaetze);
    printf("| Verbleibende Versuche:                           %s%d|\n",einzugRestVer, 
                                                                        restVersuche);
    if(entwicklungOn == NEIN)   /*ausblenden für mehr Platz für Status am Ende*/
    {
        printf("+-----------------------------------------------------+\n");
        printf("| Steuerung: w - Feld nach oben                       |\n");
        printf("|            a - Feld nach links                      |\n");
        printf("|            s - Feld nach unten                      |\n");
        printf("|            d - Feld nach rechts                     |\n");
        printf("|        ENTER - Nach Schatz im akt. Feld graben      |\n");
        printf("|      ESC / z - Spiel beenden / neustarten           |\n");
    }
    printf("+-----------------------------------------------------+\n");
    printf("| Hinweis: %s |\n", infotext);
    printf("+-----------------------------------------------------+\n");

    /*2D-Array wird zeilenweise generiert, d.h. zuerst werden entsprechend viele
     *Felder (ein Feld = [space]space) der Reihe nach geschrieben.Ist die Zeile 
     *fertig wird ein Zeilenumbruch erzeugt und die nächste Zeile geschrieben. 
     *Der Vorgang wird entsprechend KARTE_ZEILE wiederholt!*/ 
    for(zeile = 1; zeile <= FELDER_ZEILE; zeile ++)
    {
        for(spalte = 1; spalte <= FELDER_SPALTE; spalte ++)
        {
            /*immer erstmal von leerem Feld ausgehen*/
            karte[zeile][spalte] = "    ";

            /*$-Zeichen für Schatz schreiben*/
            /*Liste der gef. Schätze durchgehen */
            for(schatznr = 0; schatznr < MAX_SCHAETZE; schatznr++)
            {
               for(feldnr = 0; feldnr < LISTENFELDER; feldnr++)
               {
                    /*wenn aktuelle Zeile und Spalte in gefunden vorh.
                     *dann wurde Schatz an Stelle auch gefunden*/
                    if((gefunden[schatznr].feld[feldnr].posZeile == zeile) &&
                       (gefunden[schatznr].feld[feldnr].posSpalte == spalte))
                            karte[zeile][spalte] = " $  ";
               }
            }

            /*X-Zeichen für ein "Graben ins Leere" schreiben*/
            for(leernr = 0; leernr < MAX_VERSUCHE; leernr++)
            {
                if((leergegraben[leernr].posZeile == zeile) && 
                   (leergegraben[leernr].posSpalte == spalte))
                        karte[zeile][spalte] = " X  ";
            }
            
            /*@-Zeichen an aktuelle Position schreiben */
            if((zeile == aktPos[ZEILE]) && (spalte == aktPos[SPALTE])) 
                karte[zeile][spalte] = " @  ";

            /*Wenn gewonnen / verloren dann auf Spielfeld anzeigen -> geht noch nicht
             *kein primäres Ziel, da das auch in Hinweis angezeigt wird*/
            /*if(gefSchaetze == anzSchaetze)
            {            
                for(buchstabenr = 0; buchstabenr < MAX_BUCHSTABEN; buchstabenr++)
                {
                   if((statusAufKarte[buchstabenr].posZeile == zeile) &&
                   (statusAufKarte[buchstabenr].posSpalte == spalte))
                    {
                        zeichenInKartenformat[0] = ' ';   
                        zeichenInKartenformat[2] = statusAufKarte[buchstabenr].buchstabe;
                        zeichenInKartenformat[2] = ' ';
                        zeichenInKartenformat[3] = ' ';
                        zeichenInKartenformat[4] = '\0';
                    }
                }
                karte[zeile][spalte] = zeichenInKartenformat;
            }*/

            /*Feld ausgeben*/
            printf("%s", karte[zeile][spalte]);

            /*Am Ende doppelten Zeilenumbruch*/
            if(spalte == FELDER_SPALTE) printf("\n\n");
        }
    } 
    printf("+=====================================================+\n");
    /*während des Entwickelns bestimme Variablen anzeigen*/
    if(entwicklungOn == JA)
    {
        printf("=======================[STATUS]========================");
        /*Schatzpos anzeigen*/
        printf("\nSchatzpos: Zeile=%d Spalte=%d", 
        schatz[0].feld[0].posZeile, schatz[0].feld[0].posSpalte);
        /*aktPos anzeigen*/
        printf("\nakt Zeile=%d | akt Spalte=%d\n", aktPos[ZEILE], aktPos[SPALTE]);
        /*Liste mit Schatzkoordinaten anzeigen*/
        printf("\n Schaetze:");
        printf("\n[Z|S]: ");
        for(schatznr = 0; schatznr < anzSchaetze; schatznr++)
        {
            for(feldnr = 0; feldnr < LISTENFELDER; feldnr++)
            {
                if(schatz[schatznr].feld[feldnr].posZeile != 0) 
                    printf("\n[%d|%d] ",schatz[schatznr].feld[feldnr].posZeile, 
                                        schatz[schatznr].feld[feldnr].posSpalte); 
            }
        }
    }

    return;
}
int eingabe(int zeitpunkt)
{
/*+-------------------------------------------------------------------------------------+
Name: eingabe
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
    - die Eingabe der Benutzer*in wird in eine Zahl umgewandelt und diese an main 
      übergebn --> Steuerung des weiteren Verlaufs 
    - entsprechend des altuellen Spielzustandes (zeitpunkt) = (Levelauswahl,Spielen,Ende)
      werden selektierte Tasten freigegeben -> sonst zB immer mit n beenden möglich  
Datum_Version: 2022-01-30_V1.0
ToDo: 
    - OPTIONAL cases für Pfeiltasten anlegen: <75 ^72 >77 v80
+-------------------------------------------------------------------------------------+*/
    int nochmal;
    
    /*Schleife solange ausführen bis erwartete Eingabemöglichkeit getätigt wird*/
    do
    {
        nochmal = NEIN;
        /*Eingabe der Schwierigkeitsstufe*/
        if(zeitpunkt == ZP_LEVEL)
        {
            switch (getch())
                {
                    case '1':       /*Schwierigkeit 1*/
                        return(1);
                        break;

                    case '2':       /*Schwierigkeit 2*/
                        return(2);
                        break;

                    case '3':       /*Schwierigkeit 3*/
                        return(3);
                        break;

                    case '4':       /*Schwierigkeit 4*/
                        return(4);
                        break;

                    case 27:        /*Spiel beenden mit der ESC-Taste (ASCII-Code = 27)*/
                        system("cls"); 
                        printf("Spiel wurde beendet - See you later alligator!");
                        sleep(2);
                        exit(0);
                        break;

                    default:
                        nochmal = JA;
                        break;
                }
        }

        /*Eingabe der Spielbedienung (laufen, graben etc.*/
        if(zeitpunkt == ZP_SPIELEN)
        {
            switch (getch())    
            {
                case 'w':           /*Ein Feld nach oben*/
                    return(W);
                    break;

                case 'a':            /*Ein Feld nach links*/
                    return(A);
                    break;

                case 's':           /*Ein Feld nach unten*/
                    return(S);
                    break;

                case 'd':           /*Ein Feld nach rechts*/
                    return(D);
                    break;

                case 13:            /*Graben mit Enter-Taste (ASCII-Code = 13*/
                    return(ENTER);
                    break;

                case 'z':           /*ja noch ein Spiel spielem*/
                    return(NEUSTART);
                    break;
                
                case 27:            /*Spiel beenden mit der ESC-Taste (ASCII-Code = 27)*/
                    system("cls"); 
                    printf("Spiel wurde beendet - See you later alligator!");
                    sleep(2);
                    exit(0);
                    break;

                default:
                    nochmal = JA;
                    break;      
            }
        }
        
        /*Eingabe, ob nochmal gespielt werden soll*/
        if(zeitpunkt == ZP_ENDE)
            {
                switch (getch())    
                    {
                        case 'j':        /*ja noch ein Spiel spielem*/
                            return(NEUSTART);
                            break;
                        
                        case 'n':        /*Spiel beenden*/
                            system("cls"); 
                            printf("Tschau Miau!");
                            sleep(2);
                            exit(0);
                            break;

                        default:
                            nochmal = JA;
                            break;      
                    }
            }
    } 
    while(nochmal == JA);

    return(0);
}
void bewegen(void)
{
/*+-------------------------------------------------------------------------------------+
Name: bewegen
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
   - es wird geprüft ob Bewegung in die angeforderte Richtung möglich ist
   - die aktPos wird anhand der Bewegung neu berechnet
   - Hinweistext wird geleert
Datum_Version: 2022-01-30_V1.0
ToDo: // 
+-------------------------------------------------------------------------------------+*/
    /*Kann in Richtung bewegt werden? --> Spielfeldrand beachten!*/    
    int richtungOk = 0;

    if(((taste == W) && (aktPos[ZEILE] != 1)) ||
       ((taste == S) && (aktPos[ZEILE] < FELDER_ZEILE)) ||
       ((taste == A) && (aktPos[SPALTE] != 1)) ||
       ((taste == D) && (aktPos[SPALTE] < FELDER_SPALTE))) richtungOk = JA;

    /*Positionen berechnen*/
    if(richtungOk)
    {        
        /*Position auf die bewegt werden soll berechnen und in aktPos schreiben*/
        if(taste == W) aktPos[ZEILE] = aktPos[ZEILE] - 1;    /*ein Feld oben*/
        if(taste == S) aktPos[ZEILE] = aktPos[ZEILE] + 1;    /*ein Feld unten*/
        if(taste == A) aktPos[SPALTE] = aktPos[SPALTE] - 1;  /*ein Feld links*/
        if(taste == D) aktPos[SPALTE] = aktPos[SPALTE] + 1;  /*ein Feld rechts*/
    }
    /*bei jedem Starten der Bewegung den vorherigen Hinweis löschen*/
    hinweis(4); /*"<leer>"*/

    return; 
}
void graben(void)
{
/*+-------------------------------------------------------------------------------------+
Name: graben
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
   - Zuerst wird geprüft, ob an aktueller Stelle schon ein Schatz gefunden wurde. Dafür
     wird die Liste der gefundenen Schätze durchlaufen und dabei geprüft, ob ein Eintrag
     der Koordinaten mit denen, der aktPos übereinstimmt. Ist das der Fall, so wurde an 
     der Stelle schon ein Schatz gefunden. Dies wird dem Benutzenden mitgeteilt.
   - Wird durch Benutzenden nach einem Schatz gegraben, wird die Liste der schätze durch-
     sucht, ob es einen Eintrag mit den Koordinaten der aktPos gibt. Ist dieser Eintrag 
     vorhanden, wird dieser Schatz als gefunden markiert. Andernfalls wird die aktPos in
     die Liste leergraben eingetragen.
    
Datum_Version: 2022-01-30_V1.0
ToDo: //
+-------------------------------------------------------------------------------------+*/
    int schatzNr, feldNr, gefFeldNr;

    /*Überprüfen, ob an aktueller Stelle schon gegraben wurde --> Hinweis ausgeben*/
    /*2. Mal Schatzposition*/
    for(schatzNr = 0; schatzNr < MAX_SCHAETZE; schatzNr++)
    {
        for(gefFeldNr = 0; gefFeldNr < LISTENFELDER; gefFeldNr++)
        {           
            if((gefunden[schatzNr].feld[gefFeldNr].posZeile == aktPos[ZEILE]) &&
               (gefunden[schatzNr].feld[gefFeldNr].posSpalte == aktPos[SPALTE]))
            {
                    hinweis(7); /*den schatz bereits gefunden*/
                    return;
            }
        }
    }
    /*2. Mal an Position ohne Schatz*/
    for(feldNr = 0; feldNr < MAX_VERSUCHE; feldNr++)
    {
        if((leergegraben[feldNr].posSpalte == aktPos[SPALTE]) &&
           (leergegraben[feldNr].posZeile == aktPos[ZEILE]))
        {
                hinweis(8); /*hier bereits gegraben*/
                return;
        }
    }

    /*Überprüfen, ob an aktueller Stelle ein Schatz ist -->jede Feldposition jeder Zeile 
     *der struktur schatz wird wird mit den Werten der aktuellen Position verglichen*/
    for(schatzNr = 0; schatzNr < MAX_SCHAETZE; schatzNr++)
    {
        for(feldNr = 0; feldNr < LISTENFELDER; feldNr++)
        {
            /*wenn Position von Zeile und Spalte übereinstimmen, ist Schatz gefunden*/
            if((schatz[schatzNr].feld[feldNr].posZeile == aktPos[ZEILE]) &&
               (schatz[schatzNr].feld[feldNr].posSpalte == aktPos[SPALTE]))
            {
                for(gefFeldNr = 0; gefFeldNr < LISTENFELDER; gefFeldNr++)
                {
                    gefunden[schatzNr].feld[gefFeldNr].posZeile = 
                    schatz[schatzNr].feld[gefFeldNr].posZeile;
                    
                    gefunden[schatzNr].feld[gefFeldNr].posSpalte = 
                    schatz[schatzNr].feld[gefFeldNr].posSpalte;
                }
                gefSchaetze++;
                restVersuche--;
                hinweis(3); /*Schatz gefunden!*/
                return;
            }   
        }
    }
    /*gibt es an aktPos einen Schatz, so wird Fkt nach dem Finden beendet, gibt es keinen
     *Schatz so werden nachfolgende Zeilen ausgeführt und die "Grabung ins Leere" in der 
     *Liste leergraben notiert*/
    restVersuche--;
    leergegraben[restVersuche].posSpalte = aktPos[SPALTE];
    leergegraben[restVersuche].posZeile = aktPos[ZEILE];
    hinweis(2); /*Pech, kein Schatz!*/

    return;
}
void hinweis(int nr)
{
/*+-------------------------------------------------------------------------------------+
Name: hinweis
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
    - Generiert die Texte, welche im Layout nach Hinweis ausgegeben werden
    - Text wird entsprechend nr selektiert
    - an den Text werden soviele Leerzeichen angehangen, dass der Rand vom Layout nicht
     verschoben wird
    - Text darf max 41 Zeichen lang sein!
Datum_Version: 2022-01-30_V1.0
ToDo: //
+-------------------------------------------------------------------------------------+*/
    char hinweis[TEXTLAENGE];
   
    /*Text zum Ausgeben auswählen*/
    switch (nr)
    {
        case 1:     /*Starttext*/
            strcpy(hinweis, "Auf gehts - such den ersten Schatz!");
            break;
        
        case 2:     /*keinen Schatz gefunden*/
            strcpy(hinweis, "Pech gehabt, hier ist kein Schatz!");
            break;
        
        case 3:     /*Schatz gefunden*/
            strcpy(hinweis, "Yeah! Du hast einen Schatz gefunden!");
            break;

        case 4:     /*Hinweis leeren*/
            strcpy(hinweis, " ");
            break;

        case 5:     /*alle Schätze gefunden*/
            strcpy(hinweis, "DU HAST GEWONNEN!!!!!");
            break;

        case 6:     /*alle Versuche aufgebraucht*/
            strcpy(hinweis, "DU HAST VERLOREN!!!!!");
            break;

        case 7:     /*an Schatzposition das 2. Mal gegraben*/
            strcpy(hinweis, "Diesen Schatz hast du bereits gefunden!");
            break;

        case 8:     /*2. Mal ins Leere gegraben*/
            strcpy(hinweis, "Hier hast du bereits gegraben!");
            break;

        default:    /*Text nicht vorhanden*/
            strcpy(hinweis, "!!Text nicht vorhanden!!");
            break;
    }
    /*benötigte Leerzeichen berechnen und einfügen, damit abschließender | (=Rahmen)
     *an richtiger Stelle positioniert ist*/  
    while((TEXTLAENGE - strlen(hinweis)) != 0)
    {
        strcat(hinweis, " ");
    }   
    strcpy(infotext, hinweis);

    return;
}
int freiFeld(int zufallFeld, int artAusfuehrung)
{
/*+-------------------------------------------------------------------------------------+
Name: freiFeld
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
    - Es wird überprüft ob der Schatz an der zufällig erzeugten Position platz hat. Dafür
      wird geprüft, ob der Schatz weit genug von den Rändern entfernt ist UND dass sich
      nicht schon ein Schatz an der Positon befindet.
    - Dafür wird zuerst geprüft, ob der Schatz über den Rand gehen würde. Ist dem so, 
      wird die zufällige Position verschoben.
    - Danach wird geprüft, ob die Positionen schon belegt sind. Ist dem so, wird die Fkt.
      beendet und gibt den Wert FEHLER (=999) zurück. Dadurch wird eine neue Zufallszahl
      erzeugt.
    - Ansonsten wird Schatz an die (berechnete) Position eingefügt.
    - Zufallszahl || ber.Position = Startposition zum Schatz generien = oberes linkes Feld
      von Schatz --> von dort aus wird nach links und nach unten in Karte aufgebaut
Datum_Version: 2022-01-30_V1.0
ToDo: //
+-------------------------------------------------------------------------------------+*/
    int berechnetFeld = 0, schatznr, feldnr, schatzgr;

    /*Passt Schatz in Felder? --> Kollision mit Spielfeldrand*/
    /*Wenn Rand überschritten werden würde --> entsprechend viele Felder in
     *entgegen gesetzte Richtung, sodass Schatz reinpasst (=Returnwert)
     *Nur auf Spalte prüfen, da Karte quadratisch*/
    if((zufallFeld + aktSchatzgroesse) > FELDER_SPALTE)
        berechnetFeld = zufallFeld - (aktSchatzgroesse - (FELDER_SPALTE - (zufallFeld - 1)));   
    /*Schatz passt in Zeile/Spalte*/
    else berechnetFeld = zufallFeld;

    /*Positionen schon durch anderen Schatz belegt?*/
    /*Positionen auf denen ein Schatz angelgt werden soll hochzählen*/
    for(schatzgr = 0; schatzgr < aktSchatzgroesse; schatzgr++)
    {
        /*alle Schätze der Reihe nach durchsuchen*/
        for(schatznr = 0; schatznr < MAX_SCHAETZE; schatznr++)
        {
            /*jedes Feld der einzelnen Schätze durchsuchen*/
            for(feldnr = 0; feldnr < MAX_SCHATZGRSE; feldnr++)
            {   
                /*abhängig vom übergebenen Parameter artAusführung werden die Zeilen- 
                 *oder Spaltenkoordinaten abgeglichen in Feld->Zeile wird geprüft: von 
                 *aktueller Pos nach rechts prüfen*/
                if(artAusfuehrung == ZEILE) 
                {
                    /*Position gefunden->return mit FEHLER*/
                    if(schatz[schatznr].feld[feldnr].posZeile == 
                       berechnetFeld + schatzgr) 
                            return(FEHLER); /*keine freie Pos gefunden*/
                }
                /*in Feld->Spalte wird geprüft: von aktueller Pos nach unten prüfen*/
                if(artAusfuehrung == SPALTE) 
                {
                    /*Position gefunden->return mit FEHLER*/
                    if(schatz[schatznr].feld[feldnr].posSpalte == 
                       berechnetFeld + schatzgr) 
                            return(FEHLER); /*keine freie Pos gefunden*/
                }      
            }
        }
    }   
    
    return(berechnetFeld); /*Platz für Schatz vorhanden --> kehrt zurück mit Pos von der
                            *aus der Schatz eingetragen wird*/
} 
void outro(void)
{
/*+-------------------------------------------------------------------------------------+
Name: outro
Autor: Markus Haubold
Zustand: i.O.
Funktionsbeschreibung: 
   - zeigt an, ob der Benutzende gewonnen oder verloren hat 
   - fragt, ob noch einmal gespielt werden möchte
Datum_Version: 2022-01-30_V1.0
ToDo: //
+-------------------------------------------------------------------------------------+*/
    /*Konsole bereinigen*/
    system("cls");
    
    printf("+------------------------------------------------------+\n");
    printf("|              DAS SPIEL IST ZU ENDE!                  |\n");
    printf("+------------------------------------------------------+\n");
    /*bei gewonnen wird Katzenbild ausgegeben*/
    if(gefSchaetze == anzSchaetze)
    {
        printf("| Glueckwunsch!! Du bist wirklich eine richtig gute    |\n");
        printf("| Goldgraeber*in und hast alle Schaetze finden koennen.|\n");
        printf("| Als Belohnung gibt es dafuer ein schoenes Bild.      |\n");
        printf("|                                                      |\n");
        printf("|              _                                       |\n");                       
        printf("|              \`*-.                                    |\n");          
        printf("|               )  _`-.                                |\n");     
        printf("|              .  : `. .                               |\n");
        printf("|              : _   '  \                               |\n");
        printf("|              ; *` _.   `*-._                         |\n");
        printf("|              `-.-'          `-.                      |\n");
        printf("|                ;       `       `.                    |\n");
        printf("|                :.       .        .                   |\n");
        printf("|                . \  .   :   .-'   .                   |\n");
        printf("|                '  `+.;  ;  '      :                  |\n");
        printf("|                :  '  |    ;       ;-.                |\n");
        printf("|                ; '   : :`-:     _.`* ;               |\n");
        printf("|             .*' /  .*' ; .*`- +'  `*'                |\n");
        printf("|             `*-*   `*-*  `*-*'                       |\n");
        printf("|                                                      |\n");
    }
    else
    {
        printf("| Schade!! Du hast verloren und musst wohl noch etwas  |\n");
        printf("| an deinen Skills arbeiten...oder beim naechsten Mal  |\n");
        printf("| mehr Glueck mitbringen.                              |\n");
    }
    printf("|                                                      |\n");
    printf("|   Moechtest du dieses tolle Spiel nochmal  spielen?  |\n");
    printf("|     j-> ja, klaro      n-> nope, ist zu aufregend    |\n");
    printf("+------------------------------------------------------+\n");

  return;
}
void fuelleStatusAufKarte(int gewonnen)
{
/*+-------------------------------------------------------------------------------------+
Name: fuelleStatusAufKarte
Autor: Markus Haubold
Zustand: nicht freigegeben
Funktionsbeschreibung: 
   - ob gewonnen oder verloren wurde, soll in der Karte angezeigt werden
   - liste statusAuf Karte wird mit entsprechenden Zeichen der Reihe nach gefüllt -->
     dabei wird jeder Buchstabe mit der entsprechenden Koordinate eingetragen
Datum_Version: 2022-30-01_V1.0
ToDo: 
    - statusAufKarte wird richtig gefüllt, jedoch werden nur E´s auf der Karte gen.
+-------------------------------------------------------------------------------------+*/
    char duHast[7] = {'D', 'U',' ', 'H', 'A', 'S', 'T'}, 
         textGew[9] = {'G', 'E', 'W', 'O', 'N', 'N', 'E', 'N', '!'},
         textVer[9] = {'V', 'E', 'R', 'L', 'O', 'R', 'E', 'N', '!'};
    int buchstabe, offset = 0;
   
   /*1. Textzeile schreiben*/
    for(buchstabe = 0; buchstabe < 7; buchstabe++)
    {
        statusAufKarte[buchstabe].posZeile = 6; 
        statusAufKarte[buchstabe].posSpalte = buchstabe + 4;
        statusAufKarte[buchstabe].buchstabe = duHast[buchstabe];
    }
    offset = buchstabe;
    
    /*2. Textzeile schreiben*/
    for(buchstabe = 0; buchstabe < 9; buchstabe++)
    {
        statusAufKarte[buchstabe + offset].posZeile = 7; 
        statusAufKarte[buchstabe + offset].posSpalte = buchstabe + 3;
        if(gewonnen == JA) statusAufKarte[buchstabe + offset].buchstabe = 
                           textGew[buchstabe];
        else statusAufKarte[buchstabe + offset].buchstabe = textVer[buchstabe];
    }

    if(entwicklungOn == JA)
    {
        /*TEST: zeigen was in statusAufKarte geschrieben wird*/
        for(buchstabe = 0; buchstabe < MAX_BUCHSTABEN; buchstabe++)
        {
            printf("\ni=%d Z=%d S=%d B=%c", buchstabe, 
                                            statusAufKarte[buchstabe].posZeile, 
                                            statusAufKarte[buchstabe].posSpalte, 
                                            statusAufKarte[buchstabe].buchstabe);
            exit(0);
        }
    }

    return;
}

/*[===============MAIN================================================================]*/
int main(void)
{
    start:
    srand(time(0));

    /*Intro anzeigen*/
    intro();
    
    /*Eingabe Schwierigkeitslevel*/
    level = eingabe(ZP_LEVEL);
   
    /*Spielparameter (anzahl Schätze, Positionen Schätze, etc.) werden festgelgt*/
    spielparameter();
    
    /*Funktion mit Entwicklungstools aufrufen (händisch Schätze verstecken, etc.)*/
    if(entwicklungOn) entwicklung();

    /*Spielbetrieb solange möglich wie es noch Versuche zum Graben gibt
     *und noch nicht alle Schätze gefunden sind*/
    while((restVersuche > 0) && (gefSchaetze != anzSchaetze))
    {  
        /*Spiellayout laden*/
        layout(); 
        
        /*auf Befehl der Benutzer*in warten und dann einlesen*/
        taste = eingabe(ZP_SPIELEN);
        /*Benutzer*in will "sich bewegen":  wasd -> Funktion bewegung()
         *Benutzer*in "gräbt nach Schatz": Enter -> Funktion graben()
         *Benutzer*in möchte Spiel neustarten: z -> Sprung an Anfang von Main*/
        if(taste > 0)
        {
            if((taste >= W) && (taste <= D)) bewegen();
            if(taste == ENTER) graben();
            if(taste == NEUSTART) 
            {
                system("cls");
                goto start; /*Sprung an Anfang von main*/
            }
        }
    }
    
    if(anzSchaetze == gefSchaetze) hinweis(5);  /*gewonnen!*/
    else hinweis(6); /*verloren!*/
    /*fuelleStatusAufKarte(JA); Später, da optional und Code nicht geht*/
    
    /*wenn Spiel zu Ende, dann nochmal Spiellayou neu generieren, damit 
     *gerade noch gefundener Schatz angezeigt wird*/
    layout(); 
    
    /*kurz warten, damit Benutzer*in nochmal die Karte sieht*/
    sleep(2);
   
    /*outro anzeigen */
    outro();
    
    /*auf Eingabe von Benutzer*in warten -> nochmal? ja/nein*/
    taste = eingabe(ZP_ENDE);
    /*Benutzer*in möchte nochmal spielen -> springe an Main-Anfang*/
    if(taste == NEUSTART) 
    {
        system("cls");
        goto start;
    }

    return(0);
}