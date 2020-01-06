
#include <SPI.h>  // Inclui a biblioteca SPI de comunicação
#include <MFRC522.h>  // Inclui a biblioteca do leitor RFID
#include <Wire.h> // Inclui a biblioteca I2C de comunicação
#include <DS3231.h> // Inclui a biblioteca do módulo RTC - Real Time Clock

#define SS_PIN 53 // Pino SDA - Slave Selection
#define RST_PIN 5 // Pino Reset

DS3231 Clock; //Configura a variavel de acesso aos dados do RTC

MFRC522 rfid(SS_PIN, RST_PIN); // Lê os pinos que vai usar

MFRC522::MIFARE_Key key; // Configuração da chave de acesso dos cartões MIFARE

byte nuidPICC[4]; // Vetor com 4 slots para guardar o NUID dos cartões


// Setando os pinos usados no led 7 segmentos
int segA = 35;
int segB = 31;  
int segC = 34;
int segD = 32;
int segE = 30;
int segF = 33;
int segG = 36;
//int segPt = 4; //Não usamos a casa decimal neste experimento
int d1 = 25;
int d2 = 27;
int d3 = 24;
int d4 = 26;
//--------------------------------------//

// Pinos usados para os dois leds
int ledvermelho = 47; 
int ledverde = 49;

int delayTime = 800; // Delay que do tempo em que os dígitos ficam ligados
int tela = 0; // Configura a tela no modo inicial em que todos os segmentos ficam acessos

bool entradaesaida[5] = {0, 0, 0, 0, 0}; // Vetor para controle de entrada e saída

// Configurando o RTC
bool Century = false; 
bool h12;             
bool PM;
//--------------------------------------//


void mostrar(int t);
void checkHex(byte *buffer, byte bufferSize);
void imprimirdata();
void linha(); 

void setup() {
  SPI.begin(); // Inicia SPI bus de comunicação
  Serial.begin(9600); // Inicia o Serial
  Wire.begin(); // Inicia a comunicação I2C
  rfid.PCD_Init(); // Inicia MFRC522 (RFID)
  delay(3000); // Tempo pra configurar

  pinMode(segA, OUTPUT); // Pinagem usada setada como saida
  pinMode(segB, OUTPUT); // Pinagem usada setada como saida
  pinMode(segC, OUTPUT); // Pinagem usada setada como saida
  pinMode(segD, OUTPUT); // Pinagem usada setada como saida
  pinMode(segE, OUTPUT); // Pinagem usada setada como saida
  pinMode(segF, OUTPUT); // Pinagem usada setada como saida
  pinMode(segG, OUTPUT); // Pinagem usada setada como saida
  pinMode(d1, OUTPUT);   // Pinagem usada setada como saida
  pinMode(d2, OUTPUT);   // Pinagem usada setada como saida
  pinMode(d3, OUTPUT);   // Pinagem usada setada como saida
  pinMode(d4, OUTPUT);   // Pinagem usada setada como saida
  pinMode(ledvermelho, OUTPUT); // Pinagem usada setada como saida
  pinMode(ledverde, OUTPUT);    // Pinagem usada setada como saida
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF; // Chave para acesso dos dados do cartão RFID - Setada como {FF FF FF FF FF FF}
  }

}

void loop() {
  mostrar(tela); // Mostra no led  7segmentos - 4 digítos a tela selecionada
  led_acesso(); // Acende os LEDs vermelho e verde dependendo da situação


  if ( ! rfid.PICC_IsNewCardPresent()) // Verifica se tem um cartao presente
    return;                            // Reseta o loop se nao tiver cartao presente

 
  if ( ! rfid.PICC_ReadCardSerial()) // Verifica se o cartão (NUID) foi lido
    return;                          // Retorna se o cartão nao foi lido

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak); // Acessa e guarda o ID do cartão lido

  if (rfid.uid.uidByte[0] != nuidPICC[0] || // Verifica se o valor lido é igual ao ultimo lido
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3] ) {

  
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i]; // Armazena NUID no vetor nuidPICC
    }

  }

  checkHex(rfid.uid.uidByte, rfid.uid.size); // Verifica o valor lido com os valores cadastrados, confere se algum cartão de acesso permitido foi detectado ou um não permitido
  imprimirdata(); // Mostra a data de entrada ou de saida dependendo da situação
  linha(); // Imprime uma linha no Serial para separar as informações de acesso
  rfid.PICC_HaltA(); // Para de ler o RFID

  rfid.PCD_StopCrypto1(); // Interrompe a criptografia no PCD
}

int p = 0; // Variável auxiliar usada para acender o led
int d = 0; // Variável auxiliar usada para acender o led
int c = 0; // Variável auxiliar usada para acender o led

void led_acesso()
{
  if (tela != 0 && tela != 5) {
    digitalWrite(ledvermelho, LOW); // Apaga o Led vermelho
    if (p < 20) {
      digitalWrite(ledverde, HIGH); // Acende o verde até p ser igual a 20
      p++;
    } else if (d < 10) {
      digitalWrite(ledverde, LOW); // Apaga o verde até d ser igual a 10
      d++;
    } else {
      if (c < 6) {  // Repete o ciclo 6 vezes
        c++;
        p = 0;
        d = 0;
      } else { // Volta ao estado inicial
        tela = 0;
        c = 0;
      }
    }

  } else if (tela == 5) {
    digitalWrite(ledverde, LOW); // Apaga o Led verde
    if (p < 20) {
      digitalWrite(ledvermelho, HIGH); // Acende o vermelho até p ser igual a 20
      p++;
    } else if (d < 10) {
      digitalWrite(ledvermelho, LOW); // Apaga o vermelho até d ser igual a 10
      d++;
    } else {
      if (c < 3) {  // Repete o ciclo 3 vezes
        c++;
        p = 0;
        d = 0;
      } else { // Volta ao estado inicial
        tela = 0;
        c = 0;
      }
    }
  } else { // Deixa o Led vermelho aceso e o Led Verde apagado em condições iniciais
    digitalWrite(ledvermelho, HIGH);
    digitalWrite(ledverde, LOW);
  }

}


void selectDigit(int d) // Seleciona qual digito do display ele deverá ligar dependendo do input
{

  switch (d)
  {
    case 1:
      digitalWrite(d1, LOW); //ATIVA O PRIMEIRO DIGITO DO 7 seg
      break;
    case 2:
      digitalWrite(d2, LOW); //ATIVA O SEGUNDO DIGITO DO 7 seg
      break;
    case 3:
      digitalWrite(d3, LOW); //ATIVA O TERCEIRO DIGITO DO 7 seg
      break;
    default:
      digitalWrite(d4, LOW); //ATIVA O QUARTO DIGITO DO 7 seg
      break;
      break;
  }
}

void mostrar(int t) { // Mostra no led  7segmentos - 4 digítos a tela selecionada

  switch (t) {
    case 0:
      selectDigit(1);  // Seleciona o digito
      digitalWrite(segA, HIGH); // Seleciona o estado do segmento A
      digitalWrite(segB, HIGH); // Seleciona o estado do segmento B
      digitalWrite(segC, HIGH); // Seleciona o estado do segmento C
      digitalWrite(segD, HIGH); // Seleciona o estado do segmento D
      digitalWrite(segE, HIGH); // Seleciona o estado do segmento E
      digitalWrite(segF, HIGH); // Seleciona o estado do segmento F
      digitalWrite(segG, HIGH); // Seleciona o estado do segmento G
      delayMicroseconds(delayTime); //Seta o delay de tempo q o digito ficara ligado
      digitalWrite(d1, HIGH); // Desliga o digito usado
      selectDigit(2);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d2, HIGH); //DESLIGA O DÍGITO
      selectDigit(3);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d3, HIGH); //DESLIGA O DÍGITO
      selectDigit(4);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d4, HIGH);
      break;
    case 1:
      selectDigit(1);
      digitalWrite(segA, LOW);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d1, HIGH);
      selectDigit(2);
      digitalWrite(segA, LOW);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d2, HIGH);
      selectDigit(3);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d3, HIGH);
      selectDigit(4);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d4, HIGH);
      break;
    case 2:
      selectDigit(1);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d1, HIGH);
      selectDigit(2);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d2, HIGH);
      selectDigit(3);
      digitalWrite(segA, LOW);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d3, HIGH);
      selectDigit(4);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, LOW);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d4, HIGH);
      break;
    case 3:
      selectDigit(1);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d1, HIGH);
      selectDigit(2);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d2, HIGH);
      selectDigit(3);
      digitalWrite(segA, LOW);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, LOW);
      digitalWrite(segE, LOW);
      digitalWrite(segF, LOW);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d3, HIGH);
      selectDigit(4);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d4, HIGH);
      break;
    case 4:
      selectDigit(1);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d1, HIGH);
      selectDigit(2);
      digitalWrite(segA, LOW);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d2, HIGH);
      selectDigit(3);
      digitalWrite(segA, LOW);
      digitalWrite(segB, LOW);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d3, HIGH);
      selectDigit(4);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, LOW);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d4, HIGH);
      break;
    case 5:
      selectDigit(1);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(d1, HIGH);
      selectDigit(2);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d2, HIGH);
      selectDigit(3);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, LOW);
      digitalWrite(segC, LOW);
      digitalWrite(segD, LOW);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d3, HIGH);
      selectDigit(4);
      digitalWrite(segA, HIGH);
      digitalWrite(segB, HIGH);
      digitalWrite(segC, HIGH);
      digitalWrite(segD, HIGH);
      digitalWrite(segE, HIGH);
      digitalWrite(segF, HIGH);
      digitalWrite(segG, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(d4, HIGH);
      break;
  }
}

//Verifica a tela e o vetor controle de entrada para mostrar o horario de entrada ou saída
void imprimirdata() {
  if (tela != 0 && tela != 5) {

    if (entradaesaida[tela] == 0) { // Se for a primeira vez que passa o cartão ele identifica como entrada, caso contrário ele identifica como saída
      Serial.print("Horário de entrada:  ");
      entradaesaida[tela] = 1;
    } else {
      Serial.print("Horário de saída:  ");
      entradaesaida[tela] = 0;
    }
    Serial.print(Clock.getHour(h12, PM), DEC);
    Serial.print(':');
    Serial.print(Clock.getMinute(), DEC);
    Serial.print(':');
    Serial.print(Clock.getSecond(), DEC);
    Serial.print("  ");
    Serial.print(Clock.getDate(), DEC);
    Serial.print('/');
    Serial.print(Clock.getMonth(Century), DEC);
    Serial.print('/');
    Serial.print("20");
    Serial.println(Clock.getYear(), DEC);
  }
}
//---------------------------------------//


void linha() {  // Imprime uma linha para separar as informações
  if (tela != 0) {
    Serial.println();
    Serial.println("---------------------------------------------");
    Serial.println();
  }
}

//Verifica se o código lido é de alguem cadastrados
void checkHex(byte *buffer, byte bufferSize)  { // Tem como input o tamanho do ID e os valores de cada bloco
  if (buffer[1] == 0xb9 && buffer[2] == 0x01) { // Compara se o segundo e terceiro Hexadecimal é igual ao do cartão do RA do participante do grupo
    Serial.println(F("Hugo Fusinato 101201"));
    tela = 1;
  } else if (buffer[1] == 0x06 && buffer[2] == 0x5d) {
    Serial.println(F("Felipe Tsukuda 101205"));
    tela = 2;
  } else if (buffer[1] == 0x84 && buffer[2] == 0x55) {
    Serial.println(F("Caio Caetano 98425"));
    tela = 3;
  } else if (buffer[1] == 0xc0 && buffer[2] == 0x02) {
    Serial.println(F("Professor Rubão"));
    tela = 4;
  } else {
    Serial.println(F("Acesso Negado"));
    tela = 5;
  }

}
