/***
* Sensor e controle de vazao de água. 
* Codigo para medir o vazao (em L/min) e o volume atraves do numero de pulsos
* dados pelo sensor. Um rele é mantido acionado enquanto o volume estiver abaixo
* do desejado e desligado quando alcancar um determinado volume.
* 
* Codigo para medir a vazao(L/min), sendo calculado o volume total.
* Permite a interrupcao do fluxo por meio de um limite de vazao e/ou volume maximos.
* O custo ate o momento eh calculado com base em uma tarifa estabelecida pelo usuario.
* Conta com tres telas na interface lcd: Vazao e seu limite; Volume e seu limite; Tarifa e custo ate o momento.
* 
* Autores:
* André Nagano 11261913
* Luiz Cardoso 11804567
* Pedro Vanzan 11805015
* Rafael Ferro 11800393
* 
***/
//==============================================================================================
#include <Wire.h> // Biblioteca utilizada para fazer a comunicação com o I2C
#include <LiquidCrystal_I2C.h> // Biblioteca utilizada para fazer a comunicação com o display 20x4

#define col 16 // Serve para definir o numero de colunas do display utilizado
#define lin  2 // Serve para definir o numero de linhas do display utilizado
#define ende  0x27 // Serve para definir o endereço do display.

LiquidCrystal_I2C lcd(ende,col,lin); // Chamada da funcação LiquidCrystal para ser usada com o I2C

//==============================================================================================
//definicoes do usuario
const float volume_sem_limite = true;
const float tarifa = 4000;
const float volume_limite = 1.5;
const float vazao_limite = 50;
//==============================================================================================
//definicao do pino do sensor e de interrupcao
const int INTERRUPCAO_SENSOR = 0; //interrupt = 0 equivale ao pino digital 2
const int PINO_SENSOR = 2;

//definicao dos pinos conectados ao rele e ao botao
const int PINO_RELE = 7;
const int PINO_BOTAO = 8;

//definicao da variavel de contagem de voltas
unsigned long contador = 0;

//definicao do fator de calibracao para conversao do valor lido
const float fator_calibracao = 6.7;

//definicao das variaveis de vazao e volume
float vazao = 0;
float volume = 0;
float volume_total = 0;
float custo = 0;
char flag_limiteVazao = LOW;
int mode = 0;
char botao;

//definicao da variavel de intervalo de tempo
unsigned long tempo_antes = 0;
unsigned long tempo_botao = 0;
//==============================================================================================
void setup(){

  //inicializacao do monitor serial
  Serial.begin(9600);

  //mensagem de inicializacao
  Serial.println("Medidor de vazao e Volume de Liquidos\n");

  //configuracao do pino do sensor como entrada em nivel logico alto
  pinMode(PINO_SENSOR, INPUT_PULLUP);

  //configuracao do pino do rele como saida em nivel logico baixo
  pinMode(PINO_RELE, OUTPUT);
  digitalWrite(PINO_RELE, LOW);

  //configuracao do pino do botao como entrada
  pinMode(PINO_BOTAO, INPUT);


  lcd.init(); // Serve para iniciar a comunicação com o display já conectado
  lcd.backlight(); // Serve para ligar a luz do display
  lcd.clear(); // Serve para limpar a tela do display;

  tempo_antes = millis();
}
//==============================================================================================
void loop() {
  botao = digitalRead(PINO_BOTAO);
  
  //executa a contagem de pulsos uma vez por segundo
  if((millis() - tempo_antes) > 1000){

    //desabilita a interrupcao para realizar a conversao do valor de pulsos
    detachInterrupt(INTERRUPCAO_SENSOR);

    //conversao do valor de pulsos para L/min
    vazao = ((1000.0 / (millis() - tempo_antes)) * contador) / fator_calibracao;


    if (mode == 0){
      lcd.setCursor(0,0); // Coloca o cursor do display na coluna 1 e linha 1
      lcd.print("Vazao:");
      lcd.print(vazao);
      lcd.print("L/min ");
      lcd.setCursor(0, 1);
      lcd.print("lim:");
      lcd.print(vazao_limite);
      lcd.print("L/min    ");
    }
    else if(mode == 1){
      lcd.setCursor(0,0); // Coloca o cursor do display na coluna 1 e linha 1
      lcd.print("Volume:");
      lcd.print(volume_total);
      lcd.print("L     ");
      lcd.setCursor(0, 1);
      lcd.print("lim:");
      lcd.print(volume_limite);
      lcd.print("L       ");
    }
    else{
      lcd.setCursor(0,0); // Coloca o cursor do display na coluna 1 e linha 1
      lcd.print("Custo: R$ ");
      lcd.print(custo);
      lcd.setCursor(0, 1);
      lcd.print("Tf:R$");
      lcd.print(tarifa);
      lcd.print("/m3");
      }
    //exibicao do valor de vazao
    Serial.print("Mode");
    Serial.print(mode);
    Serial.println();

    //calculo do volume em L passado pelo sensor
    volume = vazao / 60;

    //armazenamento do volume
    volume_total += volume;

    //calculo de custo
    custo = volume_total * tarifa/1000;

    //exibicao do valor de volume
    Serial.print("Volume: ");
    Serial.print(volume_total);
    Serial.println(" L");
    Serial.println();

    //logica para o acionamento do rele
    if(vazao > vazao_limite){
      digitalWrite(PINO_RELE, LOW);
      flag_limiteVazao = HIGH;
    }
    else if((volume_total > volume_limite) && (!volume_sem_limite)){
      digitalWrite(PINO_RELE, LOW);
    } 
    else if(!flag_limiteVazao){
      digitalWrite(PINO_RELE, HIGH);
      delay(300);
    }
   
    //reinicializacao do contador de pulsos
    contador = 0;
    
    //atualizacao da variavel tempo_antes
    tempo_antes = millis();

    //contagem de pulsos do sensor
    attachInterrupt(INTERRUPCAO_SENSOR, contador_pulso, FALLING);
    
  }

  //logica para zerar a variavel volume_total
   if((botao == HIGH) && (millis()-tempo_botao>300)){
      flag_limiteVazao = LOW;
      tempo_botao = millis();
      if (++mode > 2) {
        mode = 0;
      }
  }
  
}
//==============================================================================================
//funcao chamada pela interrupcao para contagem de pulsos
void contador_pulso() {
  
  contador++;
  
}
