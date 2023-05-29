//potenciometro
const int pinPotenciometro = A0;
int valorPotencia = 0;

//intervalos
const int listaIntervalosSessao[4] = {1, 2, 3, 4};
int indiceIntervaloSessao = 0;
unsigned long intervaloPausa = 0;
unsigned long intervaloSessao = 0;
unsigned long tempoAtual = 0;

//ciclos
const int numCiclo[4] = {1, 2, 3, 4};
int cicloAtual = 0;

//botoes
const int botaoIniciar = 2;
const int botaoInterromper = 3;

//leds
const int ledPinVerde = 12;
const int ledPinVermelho = 13;
const int pinCiclos[4] = {7, 8, 9, 10};
const int pinInterrompido = 11;
bool ledVerdeLigado = false;
bool ledVermelhoLigado = false;

//Definição de estados
enum State {
  INICIAL,
  FOCO,
  PAUSA,
  INTERROMPIDO
};

//Variável de estado
State estadoAtual = INICIAL;
State estadoAnterior = INICIAL;

long tempoAnterior = 0;

//flags periodo pausa/foco
bool sessaoIniciada = false;
bool pausaIniciada = false;

unsigned long inicioFoco = 0;  // Variável para registar o início do período de foco
unsigned long inicioPausa = 0;  // Variável para registar o início do período de pausa
unsigned long tempoPassado = 0;  // Variável para armazenar o tempo decorrido no período de foco

//setup I/O
void setup() {
  // Configuração dos pins
  pinMode(pinPotenciometro, INPUT);
  pinMode(botaoIniciar, INPUT_PULLUP);
  pinMode(botaoInterromper, INPUT_PULLUP);
  pinMode(ledPinVerde, OUTPUT);
  pinMode(ledPinVermelho, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(pinCiclos[i], OUTPUT);
  }
  pinMode(pinInterrompido, OUTPUT);
  Serial.begin(9600);
}

//atualiza ledbicolor
void atualizarLEDs(bool ledVerdeLigado, bool ledVermelhoLigado) {
  digitalWrite(ledPinVerde, ledVerdeLigado ? HIGH : LOW);
  digitalWrite(ledPinVermelho, ledVermelhoLigado ? HIGH : LOW);
}

void atualizarLEDsCiclos(int cicloAtual) {
  switch (cicloAtual) {
    case 0:
      for (int i = 0; i < 4; i++) {
        digitalWrite(pinCiclos[i], LOW);
      }
      break;
    case 1:
      digitalWrite(pinCiclos[0], HIGH);
      digitalWrite(pinCiclos[1], LOW);
      digitalWrite(pinCiclos[2], LOW);
      digitalWrite(pinCiclos[3], LOW);
      break;
    case 2:
      digitalWrite(pinCiclos[0], HIGH);
      digitalWrite(pinCiclos[1], HIGH);
      digitalWrite(pinCiclos[2], LOW);
      digitalWrite(pinCiclos[3], LOW);
      break;
    case 3:
      digitalWrite(pinCiclos[0], HIGH);
      digitalWrite(pinCiclos[1], HIGH);
      digitalWrite(pinCiclos[2], HIGH);
      digitalWrite(pinCiclos[3], LOW);
      break;
    case 4:
      digitalWrite(pinCiclos[0], HIGH);
      digitalWrite(pinCiclos[1], HIGH);
      digitalWrite(pinCiclos[2], HIGH);
      digitalWrite(pinCiclos[3], HIGH);
      break;
  }
}

bool botaoIniciarPressionado = false;  // flag para verificar o botão de iniciar
bool botaoInterromperPressionado = false;
long tempoInterrompido = 0;

//Função para atualizar a máquina de estados
void atualizaEstado(long intervaloSessao, long intervaloPausa) {
  // Verifica o tempo decorrido em todos os estados
  if (pausaIniciada) {
    tempoPassado = millis() - inicioPausa;
    Serial.print("Pausa decorrida: ");
    Serial.print(tempoPassado);
    Serial.println(" ms");
    Serial.println(intervaloPausa);
    if (tempoPassado < intervaloPausa) {
      // Período de pausa ainda não terminou
      return;
    }
  }
  switch (estadoAtual) {
    case INICIAL:
      Serial.println("INICIAL");
      cicloAtual = 0;
      // AÇÕES PARA ESTADO INICIAL
      atualizarLEDs(true, false);  // led verde ligado
      atualizarLEDsCiclos(cicloAtual);

      botaoIniciarPressionado = digitalRead(botaoIniciar) == HIGH;  // Atualiza a variável para indicar se o botão está pressionado

      if (botaoIniciarPressionado) {
        cicloAtual++;
        estadoAtual = FOCO;
        estadoAnterior = INICIAL;
        sessaoIniciada = true;
        if (!pausaIniciada) {
          inicioFoco = millis();  // Regista o início do período de foco
        }
      }
      break;

    case FOCO:
      // AÇÕES PARA ESTADO FOCO
      atualizarLEDs(false, true);

      if(cicloAtual > 4){
        for (int i = 0; i < 4; i++) {
        digitalWrite(pinCiclos[i], HIGH);
        delay(100);
      }
      for (int i = 4; i > 0; i--) {
        digitalWrite(pinCiclos[i], LOW);
        delay(100);
      }
        estadoAnterior = FOCO;
        estadoAtual = INICIAL;
      }
      atualizarLEDsCiclos(cicloAtual);

      botaoInterromperPressionado = digitalRead(botaoInterromper) == HIGH;
      if (botaoInterromperPressionado) {
        estadoAtual = INTERROMPIDO;
        estadoAnterior = FOCO;
        tempoAnterior = millis() - inicioFoco;
      }

      if (sessaoIniciada) {
        if (estadoAnterior == INTERROMPIDO) {
          intervaloSessao -= tempoAnterior;
        }
        tempoPassado = millis() - inicioFoco;
        Serial.print("Tempo decorrido: ");
        Serial.print(tempoPassado);
        Serial.println(" ms");
        Serial.println(intervaloSessao);
        if (tempoPassado < intervaloSessao) {
          // Período de foco ainda não terminou
          break;
        } else {
          estadoAtual = PAUSA;
          estadoAnterior = FOCO;
          pausaIniciada = true;
          inicioPausa = millis();  // Regista o início do período de pausa
          atualizarLEDs(true, false);
        }
      }
      break;

    case PAUSA:
      // AÇÕES PARA ESTADO DE PAUSA
      if (pausaIniciada) {
        tempoPassado = millis() - inicioPausa;
        Serial.print("Pausa decorrida: ");
        Serial.print(tempoPassado);
        Serial.println(" ms");
        Serial.println(intervaloPausa);
        if (tempoPassado < intervaloPausa) {
          // Período de pausa ainda não terminou
          break;
        } else {
          cicloAtual++;
          estadoAtual = FOCO;
          estadoAnterior = PAUSA;
          inicioFoco = millis();  // Regista o início do próximo período de foco
          atualizarLEDs(false, true);
        }
      }
      break;

    case INTERROMPIDO:
      Serial.println("INTERROMPIDO");
      digitalWrite(pinInterrompido, HIGH);

      if (digitalRead(botaoIniciar) == HIGH) {
        digitalWrite(pinInterrompido, LOW);
        estadoAtual = estadoAnterior;

        if (estadoAtual == FOCO) {
          inicioFoco = millis() - tempoAnterior;  // Atualiza o tempo de início do período de foco com o valor anterior
        } else if (estadoAtual == PAUSA) {
          inicioPausa = millis() - tempoAnterior;  // Atualiza o tempo de início do período de pausa com o valor anterior
        }
        pausaIniciada = (estadoAtual == PAUSA);  // Atualiza a flag de pausa iniciada
      }
      break;
  }
}

void loop() {
  // Mapeia o valor do potenciómetro para um intervalo entre 0 e 3 (índices do array listaIntervalosSessao)
  valorPotencia = analogRead(pinPotenciometro);
  indiceIntervaloSessao = map(valorPotencia, 0, 687, 0, 3);
  //intervaloSessao = listaIntervalosSessao[indiceIntervaloSessao] * 60000;  // passa o intervalo para milisegundos
  intervaloSessao = 5000;
  //intervaloPausa = 1 * 60000;  // 1 min em milisegundos
  intervaloPausa = 5000;
  //Serial.println(intervaloSessao);
  atualizaEstado(intervaloSessao, intervaloPausa);
}
