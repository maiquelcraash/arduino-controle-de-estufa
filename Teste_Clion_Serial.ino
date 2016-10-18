// Projeto 10 -  Controle de temperatura com LCD e Cooler

#include <LiquidCrystal.h>


// Pino analógico em que o sensor de temperatura está conectado
const int sensorTemp = 1;
const int sensorLuz = 2;
const int fan = 12;                             // Declara o pino digital 13 para acionar o cooler
const int led = 13;

// Serial protocol
const unsigned char readHeader = 'r'; //72;
const unsigned char writeHeader = 'w';//27;
const unsigned char type_temp = '1';
const unsigned char type_light = '2';
const unsigned char type_specie = '4';
const unsigned char type_status = '8';
const unsigned char type_time = 16;
const unsigned char type_log = 64;

struct protocol {
    unsigned char header;
    unsigned char type;
    int size;
    unsigned char data[];
    uint16_t checksum;
};


// Variáveis dos sensores
int valorSensorTemp;                            // Variável usada para ler o valor de temperatura
int valorTemp;                                  // Variável usada para armazenar o menor valor de temperatura
int valorSensorLuz;                             // Variável usada para ler o valor de luminosidade
int valorLuz;
int fatorTemp;
int fatorLuz;

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
profile profiles[100];
short profileCount;
short profilePart;


//DEBUG MODE 1 = true, 0 = false
int debugMode = 1;


// Cria um objeto LCD e atribui os pinos
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {

    //adociona os perfis na memória
    profileCount = 0;
    profiles[profileCount++] = {"asplenio", 18, 22, 30000, 32000};
    profiles[profileCount++] = {"azaleia", 16, 9, 20000, 22000};
    profiles[profileCount++] = {"bromelia", 22, 25, 21000, 25000};
    profiles[profileCount++] = {"camedorea", 26, 28, 15000, 17000};
    profiles[profileCount++] = {"camelia", 27, 29, 20000, 23000};
    profiles[profileCount++] = {"dracenas", 22, 25, 24000, 26000};
    profiles[profileCount++] = {"miniSamambaia", 22, 25, 20000, 22000};

    //define o perfil inicial
    currentProfile = profiles[0];

    profilePart = 0;


    Serial.begin(9600);
    pinMode(fan, OUTPUT);                       // Define o pino 13 como saída
    pinMode(led, OUTPUT);
    lcd.begin(16, 2);                           // Define o display com 16 colunas e 2 linhas

    fatorTemp = 255 / (6);                      // 6 velocidades diferentes para o fan
    fatorLuz = 255 / (20);                      // 20 intensidades diferentes para o led
}

void loop() {

    /*Para evitar as grandes variações de leitura dos componentes
    LM35 e LDR são feitas 6 leituras e o menor valor lido prevalece*/

    // Inicializando a variável com o maior valor int possível
    valorTemp = 10000;
    valorLuz = 10000;

    for (int i = 1; i <= 6; i++) {              // Lendo o valor do sensor de temperatura
        valorSensorTemp = analogRead(sensorTemp);
        valorSensorLuz = analogRead(sensorLuz);

        // Transforma o valor lido do sensor de temperatura em graus celsius aproximados
        valorSensorTemp *= 0.54;

        // Mantendo sempre a menor temperatura lida
        if (valorSensorTemp < valorTemp) {
            valorTemp = valorSensorTemp;
        }

        if (valorSensorLuz < valorLuz) {
            valorLuz = valorSensorLuz;
        }

        delay(100);
    }


    if (valorTemp > currentProfile.maxTemp) {                       // Indica condição para acionamento do cooler
        // Define a coluna 0 e linha 1 do LCD onde será impresso a string
        lcd.setCursor(0, 1);
        lcd.write("Fan ligado !");              // Imprime no LCD
        analogWrite(fan, (fatorTemp * valorTemp));                // Quando condição verdadeira, liga o cooler
    }
    else if (valorTemp <= currentProfile.minTemp) {
        lcd.setCursor(0, 1);
        lcd.write("Fan desligado !");
        // Desliga cooler quando este estiver a baixo da valorTemp, indicando...
        //no LCD que esta desligado
        digitalWrite(fan, LOW);
    }

    int luz = (255 / (valorLuz));
    analogWrite(led, luz);

    checkSerial();


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
    }
    printLCD();

    delay(1000);
}

void printLCD() {
    // Exibindo valor da leitura do sensor de temperatura no display LCD
    lcd.clear();                                // Limpa o display do LCD
    lcd.print("Temperatura:");                  // Imprime a string no display do LCD
    lcd.print(valorTemp);
    lcd.write(B11011111);                       // Símbolo de graus Celsius
    lcd.print("C");
}

void checkSerial() {
    //Check if there is any data available to read
    if (Serial.available() > 0) {
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

            }
            else if (read.type == type_status) {

            }
            else if (read.type == type_time) {

            }
            else if (read.type == type_log) {

            }

            Serial.write("Leitura\n");

            clearSerial();
            writeSerial(read);
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
                clearSerial();
                profilePart++;
            }
            else if (read.type == type_temp) {
                newProfile.maxTemp = content[0];
                clearSerial();
                profilePart++;
            }
            else if (read.type == type_light) {
                newProfile.maxLight = content[0];
                clearSerial();
                profilePart++;
            }
            Serial.write("GRAVACAO\n");

            struct protocol write;

            write.header = writeHeader;
            write.type = type_status;
            write.size = 1;
            write.data[0] = 1;

            writeSerial(write);

            if(profilePart >= 3){
                profiles[profileCount++] = newProfile;
                profilePart = 0;
            }
        }
        else {
            Serial.write("LEU MAS N ENTENDEU\n");
        }
    }
}

void writeSerial(struct protocol toWrite) {

    int buffer_size = sizeof(toWrite);

    unsigned char buffer[buffer_size];
    memcpy(buffer, &toWrite, sizeof(toWrite));

    Serial.write(buffer, buffer_size);
    Serial.write("\n");
}

void clearSerial() {
    unsigned char temp = Serial.available();

    for (int i = temp; i >= 0; i--) {
        Serial.read();
    }
}