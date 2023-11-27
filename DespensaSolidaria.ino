#include "EEPROM.h"
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>

#define pinRx 2
#define pinTx 3
#define pinRele 13
#define pinSwitch 8

#define LIBERADO 3

SoftwareSerial mySerial(pinTx, pinRx, false);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int id;
int funcao;
int proximoCadastro;

void setup() {
  pinMode(pinRele, OUTPUT);
  pinMode(pinSwitch, INPUT);
  Serial.begin(9600);
  finger.begin(57600);

  delay(100);
  Serial.println("\nDespensa Solidaria");

  fecharRele();

  if (finger.verifyPassword()) Serial.println("Sensor encontrado!");
  else {
    Serial.println("Sensor nao encontrado, verifique a ligacao!");
    while (1) {
      delay(1);
    }
  }

  funcao = 2; // 1 PARA CADASTRAR E 2 PARA LER
  //finger.emptyDatabase(); // LIMPAR BASE DE DIGITAIS
  proximoCadastro = 25;
}

void alterarEstado() {
  int leitura = digitalRead(pinSwitch);
  if(leitura == 1)
    funcao = 1;
  else
    funcao = 2;
}

void loop() {
  alterarEstado();
  if (funcao == 1) {
    //Serial.println("Digite o ID # (entre 1 e 127) da digital que vai gravar...");
    //id = readnumber();
    id = proximoCadastro;
    if (id == 0) {
      return;
    }
    Serial.print("Cadastrando ID #");
    Serial.println(id);

    uint8_t registration = getFingerprintEnroll();

    if (registration == true) {
      Serial.println("CADASTROU CERTINHO");
      proximoCadastro++;
      abrirRele();
      fecharRele();
    }
  } else {
    int idCadastrado = getFingerprintID();
    if(idCadastrado > 0) {
      abrirRele();
      fecharRele();
    }
  }
}

int getFingerprintEnroll() {

  int p = -1;

  Serial.print("Aguardando digital para cadastro #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagem capturada");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Erro de comunicacao");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Erro na captura da imagem");
        break;
      default:
        Serial.println("Erro desconhecido");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem embassada");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro de comunicacao");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Nao foi possivel acessar as funcionalidades do leitor");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Nao foi possivel acessar as funcionalidades do leitor");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
  }

  Serial.println("Retire o dedo");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Coloque o mesmo dedo novamente");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagem capturada");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Erro de comunicacao");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Erro na captura da imagem");
        break;
      default:
        Serial.println("Erro desconhecido");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem embassada");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro de comunicacao");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Nao foi possivel acessar as funcionalidades do leitor");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Nao foi possivel acessar as funcionalidades do leitor");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
  }

  // OK converted!
  Serial.print("Criando modelo para ID #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Digitais conferem!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro de comunicação");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Digitais não batem!");
    return p;
  } else {
    Serial.println("Erro desconhecido");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Armazenado com sucesso!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro de comunicação");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Nao foi possivel armazenar no local indicado");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Erro de memoria");
    return p;
  } else {
    Serial.println("Erro desconhecido");
    return p;
  }

  return true;
}

int readnumber(void) {
  int num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

int getFingerprintID() {
  int fp = finger.getImage();
  if (fp != FINGERPRINT_OK)  return -1;

  fp = finger.image2Tz();
  if (fp != FINGERPRINT_OK)  return -1;

  fp = finger.fingerFastSearch();
  if (fp != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("\nEncontrado ID #"); Serial.println(finger.fingerID);
  return finger.fingerID;
}

void abrirRele() {
  digitalWrite(pinRele, LOW);
  delay(5000);
}

void fecharRele() {
  digitalWrite(pinRele, HIGH);
}
