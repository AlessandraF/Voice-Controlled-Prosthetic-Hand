/*
* Politecnico di Milano
* Corso Informatica Medica 2013/2014 - Prof. Santambrogio
* Progetto Alternativo di Informatica
* Alessandra Fusco
* E-mail: alessandra1.fusco@mail.polimi.it
* Lucia Ignoti
* E-mail: lucia.ignoti@mail.polimi.it
* ------------------------------------
*/

/*
===============================================
  CONTROLLO VOCALE PER PROTESI MANO
===============================================
*/


#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #include "Platform.h"
  #include "SoftwareSerial.h"
#ifndef CDC_ENABLED
  // Jumper posizionato su SW
  SoftwareSerial port(12,13);
#else
  // Jumper posizionato HW
  // (necessario per Arduino Leonardo)
  #define port Serial1
#endif
#else 
// altrimenti usa le nuove porte seriali, definite 12, 13
  #include "WProgram.h"
  #include "NewSoftSerial.h"
  NewSoftSerial port(12,13);
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

#include <Servo.h>
#include <wire.h>

//Variabili di tipo Servo connesse a ciascun dito.
Servo pollice, indice, medio, anulare, mignolo;

int led = 8;

//Posizioni standard per ciascun dito.
int disteso = 0; 
int piegato = 180;
int meta = 90;

//GRUPPI
enum Groups
{
  GROUP_0  = 0,
  GROUP_1  = 1,
};

//GROUP_0: viene usato come started per il controllo vocale.
enum Group0 
{
  G0_AVVIA = 0,
};

enum Group1 
{
  G1_POLLICE = 0,
  G1_INDICE = 1,
  G1_MEDIO = 2,
  G1_ANULARE = 3,
  G1_MIGNOLO = 4,
  G1_APERTA = 5,
  G1_CHIUSA = 6,
  G1_STRINGI = 7,
  G1_MATITA = 8,
};


EasyVRBridge bridge;

int8_t group, x;

//Collega ciascuna variabile servo al PWM corrispondente.
void setup()
{
  pollice.attach(3);
  indice.attach(5);
  medio.attach(6);
  anulare.attach(9);
  mignolo.attach(10);
  delay(500);
  
  pinMode(led, OUTPUT);

/*
  EasyVR Bridge
  Permette il collegamento del modulo EasyVR alla porta seriale del PC,
  usando Arduino come un adattatore USB/seriale.
  <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef CDC_ENABLED
  if (bridge.check())
  {
    cli();
    bridge.loop(0, 1, 12, 13);
  }
  Serial.begin(9600);
  Serial.println("Bridge not started!");
  
#else
  if (bridge.check())
  {
    port.begin(9600);
    bridge.loop(port);
  }
  Serial.println("Bridge connection aborted!");
#endif
  port.begin(9600);

  //Errore nella rilevazione di EasyVR
  while (!easyvr.detect())
  {
    Serial.println("Forse sarebbe meglio collegare EasyVR! :)");
    delay(1000);
  }
  
  Serial.println("EasyVR rilevata!");
  Serial.println("Iniziamo a divertirci! :)");
  easyvr.setTimeout(5);
  easyvr.setLanguage(0);

  //Definiamo un gruppo di partenza per la lettura.
  group = EasyVR::TRIGGER;
}

void action();

void loop()
{
  
  Serial.print("Pronuncia un comando nel Gruppo ");
  Serial.println(group);
  easyvr.recognizeCommand(group);

  do
  {
    digitalWrite(led, HIGH);
  }
  while (!easyvr.hasFinished());

  x = easyvr.getWord();
  if (x >= 0)
  {
    return;
  }
  x = easyvr.getCommand();
  if (x >= 0)
  {
    //Segnale luminoso per segnalare la rilevazione di un comando.
    lampeggia();
    //Segnale sonoro per segnalare la rilevazione di un comando.
    beep();
    uint8_t train = 0;
    char name[32];
    Serial.print("Il comando pronunciato e': ");
    Serial.print(x);
    
    if (easyvr.dumpCommand(group, x, name, train))
    {
      Serial.print(" = ");
      Serial.println(name);
    }
    else
      Serial.println();
    action();
  }
  else 
  // Errore di Time out
  {
    if (easyvr.isTimeout())
      Serial.println("Timed out, riprova!;)");   
    int16_t err = easyvr.getError();
    if (err >= 0)
    {
      Serial.print("Errore ");
      Serial.println(err, HEX);
    }
  }
}


/*
===============================================
	DEFINIZIONE DELLE FUNZIONI
===============================================
*/

void action()
{
    switch (group)
    {
    case GROUP_0:
      switch (x)
      {
      case G0_AVVIA:
        muovi_avvia();
        break;
      }
      break;
    case GROUP_1:
      switch (x)
      {
      case G1_POLLICE:
        muovi_pollice();
        break;
      case G1_INDICE:
        muovi_indice();
        break;
      case G1_MEDIO:
        muovi_medio();
        break;
      case G1_ANULARE:
        muovi_anulare();
        break;
      case G1_MIGNOLO:
        muovi_mignolo();
        break;
      case G1_APERTA:
        muovi_aperta();
        break;
      case G1_CHIUSA:
        muovi_chiusa();
        break;
      case G1_STRINGI:
        muovi_stringi();
        break;
      case G1_MATITA:
        muovi_matita();
        break;
      }
      break;
    }
}


void beep()
{   
    for (int i=0; i<3; i++)
    easyvr.playSound(0, EasyVR::VOL_DOUBLE);
}


void lampeggia()
{
  delay(1500);
  for(int i=0; i<3 ; i++) 
  {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);  
    delay(100);
  }
}

//Funzione 'password' - ci permette di saltare al gruppo successivo.
void muovi_avvia()
{
  group=GROUP_1;
  delay(3000);
  Serial.println("Password corretta!;)");
}

void moto(Servo s, int angolo)
{
  s.write(angolo);
}

void muovi_pollice()
{
  moto (pollice, disteso);
  moto (indice, piegato);
  moto (medio, piegato);
  moto (anulare, piegato);
  moto (mignolo, piegato);
  delay(4000);
  
  moto (pollice, piegato);
  moto (indice, disteso);
  moto (medio, disteso);
  moto (anulare, disteso);
  moto (mignolo, disteso);
  delay(4000);
}

void muovi_indice()
{
  moto (indice, disteso);
  moto (pollice, piegato);
  moto (medio, piegato);
  moto (anulare, piegato);
  moto (mignolo, piegato);
  delay(4000);
  
  moto (indice, piegato);
  moto (pollice, disteso);
  moto (medio, disteso);
  moto (anulare, disteso);
  moto (mignolo, disteso);
  delay(4000);
}

void muovi_medio()
{
  moto (medio, disteso);
  moto (pollice, piegato);
  moto (indice, piegato);
  moto (anulare, piegato);
  moto (mignolo, piegato);
  Serial.println("What?!");
  delay(4000);
  
  moto (medio, disteso);
  moto (pollice, disteso);
  moto (indice, disteso);
  moto (anulare, disteso);
  moto (mignolo, disteso);
  delay(4000);
}

void muovi_anulare()
{
  moto (anulare, disteso);
  moto (pollice, piegato);
  moto (indice, piegato);
  moto (medio, piegato);
  moto (mignolo, piegato);
  delay(4000);
  
  moto (anulare, piegato);
  moto (pollice, disteso);
  moto (indice, disteso);
  moto (medio, disteso);
  moto (mignolo, disteso);
  delay(4000);
}

void muovi_mignolo()
{
  moto (mignolo, disteso);
  moto (pollice, piegato);
  moto (indice, piegato);
  moto (medio, piegato);
  moto (anulare, piegato);
  delay(4000);
  
  moto (mignolo, piegato);
  moto (pollice, disteso);
  moto (indice, disteso);
  moto (medio, disteso);
  moto (anulare, disteso);
  delay(4000);
}

void muovi_aperta()
{
  moto(mignolo, disteso);
  moto(anulare, disteso);
  moto(medio, disteso);
  moto(indice, disteso);
  moto(pollice, disteso);
  delay(4000);
}

void muovi_chiusa()
{
  moto(mignolo, piegato);
  moto(anulare, piegato);
  moto(medio, piegato);
  moto(indice, piegato);
  moto(pollice, piegato);
  delay(4000);
}

void muovi_stringi()
{
  moto(mignolo, meta);
  moto(anulare, meta);
  moto(medio, meta);
  moto(indice, meta);
  moto(pollice, meta);
  delay(4000);
}

void muovi_matita()
{
  moto(mignolo, disteso);
  moto(anulare, disteso);
  moto(medio, piegato);
  moto(indice, piegato);
  moto(pollice, piegato);
  delay(4000);
}
