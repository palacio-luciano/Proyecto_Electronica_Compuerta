//Se incluyen librerias para controlar el servo y general#include <Servo.h>
#include <stdlib.h>

//Se crea el servo compuerta
Servo compuerta;

//Se colocan los pines del ultrasonido y servo
const int pinServo=9;
const int pinTrig=7;
const int pinEcho=6;

//Se inician las variables
const float alturaSensor=30;
float anormal=5;
float acritica=18;

float altura=0;

//Se inicializa la parte del comando
String Comando="";
bool comandoListo=false;
bool automatico=false;

//Se inician los angulos de la compuerta
int apertura=0;
int aperturaAnt=0;

//Se inicia el serial,se reserva espacio para el comando y se asignan los pines
void setup() {
  Serial.begin(9600);
  Comando.reserve(20);

  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);

  compuerta.attach(pinServo);
  compuerta.write(apertura);
}

void loop() {

//Hace la lectura del sensor
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  long duracion=pulseIn(pinEcho, HIGH);

//Mide la distancia vacía en cm
  float distanciaVacia = duracion * 0.01715;

//Se le da el valor a la altura
  altura=alturaSensor-distanciaVacia;

//Para evitar errores de lectura se obliga a que si la altura es negativa tome el valor de 0
  if (altura<0) {
    altura=0;
  }

//Las próximas líneas sirven para asignar los valores de apertura, si es menor a la altura normal entonces queda cerrado, si es mayor a la crítica entonces se abre completamente, si está en el medio se va a abrir utilizando interpolación
  int aperturaNuev=0;
  if(altura<=anormal){
    aperturaNuev=0;
  }
  else if(altura>=acritica){
    aperturaNuev=90;
  }
  else{
    aperturaNuev=map(altura*10,anormal*10,acritica*10,0,90);
  }

//Para evitar errores (Por oleaje por ejemplo) se crea una zona muerta de 10 grados, evitando que el servo esté en constante movimiento.
  if (abs(aperturaNuev-aperturaAnt)>=10){
    apertura=aperturaNuev;
    compuerta.write(apertura);
    aperturaAnt=apertura;
  }

//Analiza si se ha enviado algún comando, si se verifica entonces llama a la función "interpreta".
  if (comandoListo){
    interpreta();
  }

//Se utiliza para cambiar entre lectura automática o manual.
  if(automatico==true){
  Serial.println(altura);
  }
  delay(300);
}

//Verifica si se envió un comando por el serial.
void serialEvent(){
  while(Serial.available()){
    char inChar=(char)Serial.read();
    Comando+=inChar;

    if (inChar=='\r'){
      comandoListo=true;
    }
  }
}

//Función para controlar remotamente.
void interpreta() {

//Para em
  if (Comando[0] == ':' && Comando[1] == '1') {
    char accion = Comando[2];
    int valorConsigna;
    int valoranormal;
    int valoracritica;

    switch (accion) {
      case 'P':
        Serial.print(":1P ");
        Serial.println(altura);
        Serial.print("\r");
        break;
       
      case 'C':
        valorConsigna=atoi(&Comando[3]);
       
        if (valorConsigna >= 0 && valorConsigna <= 90) {
          apertura = valorConsigna;
          compuerta.write(apertura);
         
          Serial.print(":1C ");
          Serial.println(apertura);
          Serial.print("\r");
        }
        else{
          Serial.println("Dato invalido\r");
        }
        break;
      case 'X':
        valoranormal=atoi(&Comando[3]);

        if (valoranormal >= 0 && valoranormal <= acritica && valoranormal <= alturaSensor) {  

          Serial.print(":1X ");
          anormal=valoranormal;
          Serial.println(anormal);
          Serial.print("\r");
        }
        else{
          Serial.println("Dato invalido\r");
        }
        break;
      case 'Z':
        valoracritica=atoi(&Comando[3]);

        if (valoracritica >= anormal && valoracritica <= alturaSensor) {  

          Serial.print(":1Z ");
          acritica=valoracritica;
          Serial.println(acritica);
          Serial.print("\r");
        }
        break;
      case 'A':
        if (automatico==true){  
          automatico=false;
        }
        else{
          automatico=true;
        }
        Serial.print(":1A ");
        Serial.print("\r");
        break;
    }
  }
  Comando = "";
  comandoListo = false;
}
