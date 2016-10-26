﻿// Projeto 10 -  Controle de temperatura com LCD e Cooler
#include <EEPROM.h>
#include <LiquidCrystal.h>


// Pino analógico em que o sensor de temperatura está conectado
const int sensorTemp = 1;
const int sensorLuz = 2;
const int fan = 12;                             // Declara o pino digital 13 para acionar o cooler
const int led = 13;
int count = 0;
int luz;

// Serial protocol
const unsigned char readHeader = 72;
const unsigned char writeHeader = 27;
const unsigned char type_temp = 1;
const unsigned char type_light = 2;
const unsigned char type_specie = 4;
const unsigned char type_status = 8;
const unsigned char type_time = 16;
const unsigned char type_log = 64;

struct protocol {
    unsigned char header;
    unsigned char type;
    unsigned char size;
    unsigned char data[20];
    uint16_t checksum;
};


// Variáveis dos sensores
int valorSensorTemp;                            // Variável usada para ler o valor de temperatura
int valorTemp = 0;                                  // Variável usada para armazenar o menor valor de temperatura
int valorSensorLuz;                             // Variável usada para ler o valor de luminosidade
int valorLuz = 0;
int fatorTemp;
int fatorLuz;

//temporarios para teste EEPROM
int valorTemp_ = 24;
int valorLuz_ = 33;

// Variáveis dos profiles
typedef struct profile {
    char *specie;
    int minTemp;
    int maxTemp;
    int minLight;
    int maxLight;                     // array com os profiles
} profile;
profile currentProfile;
profile newProfile;
short currentProfileID;
profile profiles[100];
short profileCount;
short profilePart;


//DEBUG MODE 1 = true, 0 = false
int debugMode = 0;

//Outras variaveis
int botao;              // BOTAO
int segue;              // Relacionado ao Botao e a planta selecionada
int writeAdress;        //Enderesso para escrever na EEPROM
int writeAdressH;       //Enderesso para escrever na EEPROM Histórico
int readAdress;         //Enderesso para ler da EEPROM
int readAdressH;        //Enderesso para ler da EEPROM Histórico
String especie_;
char letra;

// Cria um objeto LCD e atribui os pinos
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {

    //adiciona os perfis na memória
    profileCount = 0;
    profiles[profileCount++] = {"Asplenio", 18, 22, 30, 32};
    profiles[profileCount++] = {"Azaleia", 16, 9, 20, 22};
    profiles[profileCount++] = {"Bromelia", 22, 25, 21, 25};
    profiles[profileCount++] = {"Camedorea", 26, 28, 15, 17};
    profiles[profileCount++] = {"Camelia", 27, 29, 20, 23};
    profiles[profileCount++] = {"Dracenas", 22, 25, 24, 26};
    profiles[profileCount++] = {"MiniSamambaia", 22, 25, 20, 22};

    //define o perfil inicial
    currentProfileID = 0;
    currentProfile = profiles[currentProfileID];

    profilePart = 0;

    Serial.begin(9600, SERIAL_8N2);
    pinMode(fan, OUTPUT);                       // Define o pino 13 como saída
    pinMode(led, OUTPUT);
    lcd.begin(16, 2);                           // Define o display com 16 colunas e 2 linhas

    //Salvando especies iniciais na EEPROM
//    setEspeciesIni();

    fatorTemp = 255 / (6);                      // 6 velocidades diferentes para o fan
    fatorLuz = 255 / (20);                      // 20 intensidades diferentes para o led

    segue = 0;
    lcd.setCursor(0, 0);
    lcd.print("Left   - Especie");
    lcd.setCursor(0, 1);
    lcd.print("Select - Define");

    //Escreve valores de config na EEPROM
    //if(EEPROM.read(1) < 500){
    EEPROM.write(0, 2);               //pra lista de Especies começa a escrever na posição 2
    EEPROM.write(1, 500);              //pra lista de Historico começa a escrever na posição 500
    //}

    writeAdress = EEPROM.read(0);      //começa a escrever a partir da ultimo caracter gravado para especies
    writeAdressH = EEPROM.read(1);    //começa a escrever a partir do ultimo caracter gravado no histórico
    readAdress = 2;         //Começa a ler EEPROM na posicao 2, para pegar as opções de plantas
    readAdressH = 500;      //A partir da posição 500 da EEPROM guarda o histórico
}

void loop() {

    /*Para evitar as grandes variações de leitura dos componentes
    LM35 e LDR são feitas 6 leituras e o menor valor lido prevalece*/

    //Limpa histórico
    leBotao();

    if (segue != 0) {

        //for (int i = 1; i <= 12; i++) {              // Lendo o valor do sensor de temperatura
        valorSensorTemp = analogRead(sensorTemp);
        valorSensorLuz = analogRead(sensorLuz);

        //valorTemp += valorSensorTemp;
        //valorLuz += valorSensorLuz;
        valorTemp = 24;
        valorLuz = 20;
        count++;

        //if para pegar a média dos sensores a cada 15 leituras
        if (count % 15 == 0) {
            //valorTemp = valorTemp/15;
            //valorLuz = valorLuz/15;

            delay(20);

            if (valorTemp > currentProfile.maxTemp) {                       // Indica condição para acionamento do cooler
                // Define a coluna 0 e linha 1 do LCD onde será impresso a string
                lcd.setCursor(0, 1);
                analogWrite(fan, (fatorTemp * valorTemp));                // Quando condição verdadeira, liga o cooler
            }
            else if (valorTemp <= currentProfile.minTemp) {
                lcd.setCursor(0, 1);
                // Desliga cooler quando este estiver a baixo da valorTemp, indicando...
                //no LCD que esta desligado
                digitalWrite(fan, LOW);
            }

            //int luz = (255 / (valorLuz));
            luz = (500 / (valorLuz)) * 250;
            analogWrite(led, luz);


            printLCD();

            //valorTemp = 0;
            //valorLuz = 0;
        }

        checkSerial();
        leBotao();

        //Gravando valores na EEPROM
        EEPROM.write(writeAdressH++, valorTemp_);
        EEPROM.write(writeAdressH++, valorLuz_);

        //Conversão String to CHAR para salvar na EEPROM
        especie_ = (String) currentProfile.specie;
        for (int i = 0; i < especie_.length(); i++) {
            letra = (byte) especie_.charAt(i);
            EEPROM.write(writeAdressH++, letra);
        }

        //Caracter de fechamento= # =35; depois disso é uma nova sequencia de Temperatura, Lumnosidade e Especie
        EEPROM.write(writeAdressH++, 35);

        //Registra na posição 1 da EEPROM a posicao do ultimo caracter de Historico salvo
        EEPROM.write(1, writeAdressH);


        //DEBUG
        if (debugMode) {
            //Serial.write(valorTemp);
            Serial.print(valorTemp);
            Serial.write(" - ");
            Serial.print(valorLuz);
            Serial.write(" - ");
            Serial.print(luz);
            Serial.write("\n");
            Serial.print(profiles[0].specie);
            Serial.write("\n");
            /*
              ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
              //Lendo EEPROM e imprimindo no Serial
              Serial.println(EEPROM.read(readAdressH++));
              Serial.println(EEPROM.read(readAdressH++));

              leBotao();

              while(letra != 35){
                letra=EEPROM.read(readAdressH++);
                Serial.print("ADD= ");
                Serial.print(readAdressH-1);
                Serial.print("- VALOR= ");
                Serial.println(letra);
              }
              ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
              */
        }
        delay(100);
    }//fechando if
}

void printLCD() {
    // Exibindo valor da leitura do sensor de temperatura no display LCD
    lcd.clear();                                // Limpa o display do LCD
    lcd.print("T:");                  // Imprime a string no display do LCD
    lcd.print(valorTemp);
    lcd.write(B11011111);                       // Símbolo de graus Celsius
    lcd.print("C");
    lcd.setCursor(7, 0);
    lcd.print("Lum:");
    lcd.print(valorLuz);
    lcd.setCursor(0, 1);
    lcd.print("Esp: ");
    lcd.write(currentProfile.specie);
}

void checkSerial() {
    //Check if there is any data available to read
    if (Serial.available() > 0) {

        //Serial.write("CHEGOU!!");
        //read only one byte at a time
        unsigned char header = Serial.read();

        //Check if header is found
        if (header == readHeader) {

            struct protocol read;

            read.header = header;
            read.type = Serial.read();
            unsigned char size1 = Serial.read();
            unsigned char size2 = Serial.read();
            read.size = (size1 << 8) | (size2 & 0xff);

            if (read.type == type_temp) {
                read.size = 1;
                read.data[0] = valorTemp;
            }
            else if (read.type == type_light) {
                read.size = 1;
                read.data[0] = valorLuz;
            }
            else if (read.type == type_specie) {
                int i = 0;
                while (true) {
                    if (currentProfile.specie[i] != 0) {
                        read.data[i] = currentProfile.specie[i];
                        i++;
                        read.size = i;
                    }
                    else { break; }
                }
            }
            else if (read.type == type_status) {

            }
            else if (read.type == type_time) {

            }
            else if (read.type == type_log) {

            }

            clearSerial();
            writeSerial(read);

            if (debugMode) {
                Serial.write("Leitura\n");
            }
        }
        else if (header == writeHeader) {

            struct protocol read;

            read.header = readHeader;
            read.type = Serial.read();
            unsigned char size1 = Serial.read();
            unsigned char size2 = Serial.read();
            read.size = 7;//(size1 << 8) | (size2 & 0xff);

            char content[read.size];

            int j = read.size;

            for (int i = 0; i < j; i++) {
                content[i] = Serial.read();
            }

            if (read.type == type_specie) {
                newProfile.specie = content;
                lcd.setCursor(0, 1);
                lcd.print(newProfile.specie);
                //clearSerial();
                profilePart++;
            }
            else if (read.type == type_temp) {
                newProfile.maxTemp = content[0];
                newProfile.minTemp = content[1];
                //clearSerial();
                profilePart++;
            }
            else if (read.type == type_light) {
                newProfile.maxLight = content[0];
                newProfile.minLight = content[1];
                //clearSerial();
                profilePart++;

                if (profilePart >= 3) {
                    //profiles[profileCount++] = newProfile;
                    profiles[profileCount++] = {newProfile.specie, newProfile.minTemp, newProfile.maxTemp, newProfile.minLight, newProfile.maxLight};
                    profilePart = 0;
                    Serial.write("EU SO FODAAAA\n");
                }
            }
            if (debugMode) {
                Serial.write("Gravação\n");
            }

            struct protocol write;

            write.header = writeHeader;
            write.type = type_status;
            write.size = 10;
            write.data[0] = 1;

            writeSerial(write);
        }
        else {
            if (debugMode) {
                Serial.write("EU MAS N ENTENDEU\n");
            }
            //clearSerial();
        }
    }
}

void writeSerial(struct protocol toWrite) {
    unsigned char serialBuffer[toWrite.size + 5];

    serialBuffer[0] = toWrite.header;
    serialBuffer[1] = toWrite.type;
    serialBuffer[2] = toWrite.size;
    serialBuffer[3] = 0;


    int i;
    for (i = 0; i < toWrite.size; i++) {
        serialBuffer[i + 4] = toWrite.data[i];
    }
    serialBuffer[i + 5] = 0;

    int size = toWrite.size + 5;

    Serial.write(serialBuffer, size);
}

void clearSerial() {
    //unsigned char temp = Serial.available();

    //for (int i = temp; i >= 0; i--) {
    //  Serial.read();
    //}
}

void setEspeciesIni() {
    writeAdress = 2;
    profileCount = 0;

    while (writeAdress < 116) {

        //Registrando ESPECIE
        especie_ = (String) profiles[profileCount].specie;

        for (int i = 0; i < especie_.length(); i++) {
            letra = (byte) especie_.charAt(i);
            EEPROM.write(writeAdress++, letra);
        }
        //Caracter de fechamento de especie= $ =36; depois disso é uma sequencia de Temperatura min/max, Lumnosidade min/max
        EEPROM.write(writeAdress++, 36);

        //TEMPERATURA MIN/MAX
        EEPROM.write(writeAdress++, profiles[profileCount].minTemp);
        EEPROM.write(writeAdress++, profiles[profileCount].maxTemp);

        //LUMINOSIDADE MIN/MAX
        EEPROM.write(writeAdress++, profiles[profileCount].minLight);
        EEPROM.write(writeAdress++, profiles[profileCount].maxLight);

        //Caracter de fechamento= # =35; depois disso é uma nova sequencia de Temperatura, Lumnosidade e Especie
        EEPROM.write(writeAdress++, 35);
        profileCount++;
    }

    //Registra na posição 1 da EEPROM a posicao do ultimo caracter de Historico salvo
    EEPROM.write(0, writeAdress);
}

void leBotao() {
    botao = analogRead(0);  //Leitura do valor da porta analógica A0

    // Botao DOWN => Limpa histórico
    if (botao > 200 && botao < 400) {
        //Serial.println("Limpando Histtttttttttttttttttttttttttttttttttt");
        for (int i = 500; i < writeAdressH; i++) {
            EEPROM.write(i, 0);
        }
        writeAdressH = 500;
        readAdressH = 500;
        delay(200);
    }

    botao = analogRead(0);  //Leitura do valor da porta analógica A0
    // Botao LEFT => Vai passando especies para seleção
    if (botao > 400 && botao < 600) {
        if (currentProfileID >= profileCount) {
            currentProfileID = 0;
        }
        currentProfile = profiles[currentProfileID++];

        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Esp: ");

        lcd.print(currentProfile.specie);

        lcd.setCursor(7, 0);
        lcd.print(currentProfileID);

        botao = 0;
        segue = 0;
        delay(200);
    }

    botao = analogRead(0);  //Leitura do valor da porta analógica A0
    // Botao SELECT => Seleciona uma espécie
    if (botao > 600 && botao < 800) {
        segue = 1;
        delay(200);
    }
}
