#include <util/delay.h>
#include <avr/io.h>
#include <Arduino.h>

#include "LedControlMS.h"
#include <math.h>
#include <LiquidCrystal_I2C.h>      //Librería para tipo de programación, habilitación de I/O
#include <avr/interrupt.h>
#include <stdbool.h>

/*int boton = 0; //Variable para guardar la información leída en PORTD
boton = (PIND & (1<<PD1)); /* Lee el pin 1 de PORTD y lo coloca en la variable.*/


String username ="";//Mensaje recibido por serial de nombre
String tipo=""; //Tipo de residuo Residencial  o Industrial
String pesoT =""; //peso de lo desechos

LedControl lc=LedControl(12,11,10,1);//Se define la matriz y sus puertos DIN,CLK,CS
LiquidCrystal_I2C lcd1(0x27,16,2);


byte cara_feliz[] = {
B00111100,
B01000010,
B10100101,
B10000001,
B10100101,
B10011001,
B01000010,
B00111100};


void Representar(byte *Datos,int retardo) //Funcion para la representacion de bytes de datos para una matriz de 8x8 
{
  for (int i = 0; i < 8; i++)  
  {
    lc.setColumn(0,i,Datos[7-i]);
  }
  delay(retardo);
}

//INTERRUPCION BOTON
volatile bool abajo = true;
void interrupt_INT0_Init(){
 cli();
 EICRA = 0b11;
 EIMSK = 1;
 sei();
}
//INTERRUPCIONES
ISR(INT0_vect){
 abajo = !abajo;
}


void setup() {
  Serial.begin(9600);
  DDRD = 0b11111011; //Configura pin 2 de PORTD como entrada y el resto salida
  lc.shutdown(0,false);
  lc.setIntensity(0,12);// La valores estan entre 1 y 15 
  lc.clearDisplay(0);
  ////LCD
  lcd1.init();
  lcd1.backlight();
  lcd1.setCursor(1,0);
  lcd1.println("Bienvenido :) ");
  PORTD |= (1<<PD2); //Activar resistencia de PULLUP en PIN0 del puerto D
  interrupt_INT0_Init();
  
}

void loop (){
  
  for (int col=0; col<=7; col++){
    for (int row=0; row<=7; row++){
      lc.setLed(0,col,row,true); // 
      delay(25);
    }
  }
//////////////

  delay(500);
  lc.writeString(0," BIENVENIDO ");
  delay(500);
  Representar(cara_feliz,5000);
  delay(500);
  Serial.println("Bienvenido :)");
  lcd1.clear();

  while(true){

    Serial.println("Ingresa tu nombre seguido de enter: ");
    lcd1.print("Ingresa tu nombre seguido de enter: ");
    delay(500);
    //for (int posicion = 0; posicion < 20; posicion++){
       // lcd1.scrollDisplayLeft();
      //  delay(250);
      //  }

    while (!(Serial.available())){delay(20);}
    username = Serial.readStringUntil('\n');

    Serial.print("Bienvenido ");
    Serial.println(username);
    lcd1.print("Bienvenido");
    lcd1.print(username);
    delay(2000);
    ///////////////////Validacion de ingreso de tipo de desechos
    //////Datos de los camiones
    float valorTotal = 0.0;
    int nCamionesR = 32;
    int nCamionesI = 32;
    float limR=0.5;
    float limI=1.0;
    float precioR=50.0;
    float precioI=100.0;
    String pesoAux = "";
  //////////////////////////////////////////////////////////////////////
    int aux=0;
    while(aux!=1){ 
        Serial.print("Ingresa el tipo de desecho seguido de enter: (R-residencial, I-industrial)");
        lcd1.print("Ingresa el tipo de desecho seguido de enter: (R-residencial, I-industrial)");
        delay(600);
        //serial_println_strfor (int posicion = 0; posicion < 35; posicion++){
          //  lcd1.scrollDisplayLeft();
           // delay(300);
          //}

        while (!(Serial.available())){_delay_ms(20);}
        String trash= Serial.readStringUntil('\n');
        if ((strcmp(trash,"R")==0) ||(strcmp(trash,"r")==0)) {
          tipo="Residencial" ;
          aux=1;
          break;

        }else if((strcmp(trash,"I")==0) || (strcmp(trash,"i")==0)){
          tipo="Industrial";
          aux=1;
          break;

        }else {Serial.write("Solo un tipo de desecho correcto (R o I)");
          aux=0;
        }  
    }
    Serial.print("Se escogio tipo ");
    lcd1.clear();
    lcd1.print("Se escogio tipo ");
    lcd1.print(tipo);
    Serial.println(tipo);
    delay(600);

    Serial.print("Ingrese el peso (en Kg): " );
    while (!(Serial.available())){delay(20);}
    pesoT = Serial.readStringUntil('\n');
    float pesoAux = (pesoT.toFloat()/1000)
    String pesoAux1 = pesoAux.toString();/////////Peso de  toneladas
    Serial.print("Toneladas: ");
    Serial.println(pesoAux1);

    ///////CALCULO DE PRECIO Y CAMIONES A USAR///////////////////////////////////////////////////////

    int nActual=0;
    ///////// Determinamos el numero de camiones
    if (strcmp(tipo,"Residencial")==0){
      if (pesoAux>limR){
        nActual=ceil(pesoAux/limR);//Ceil redondea a lo maximo, paa determinar el numero minimo de camiones a usar
        valorTotal=precioR*nActual;
        nCamionesR--;//se quita un led residencial
      }
    }else {
      if(pesoAux>limI){
        nActual=ceil(pesoAux/limI);//Ceil redondea a lo maximo, paa determinar el numero minimo de camiones a usar
        valorTotal=precioI*nActual;
        nCamionesI--;//se quita un led industrial
      }
    };
    Serial.print("Numero de camiones:  ");
    //serial_println_str(char(nActual));
    delay(500);
    Serial.print("Valor a pagar:  $");
    //serial_println_str(char(valorTotal));

    /////////////MODIFICADO 5/8/2022
    //Secuencia de apagado de Leds
    //int CResidenciales= [4,5,6,7]; 
    //int CIndustriales= [0,1,2,3]; 
    ///Apagar industriales
    
    ///Apagar Residenciales
    int i=0;
    for (int col=0; col<=3; col++){
      for (int row=7; row>=0; row--){
        if(i<=(32-nCamionesI)){
          lc.setLed(0,col,row,false); // 
          delay(200);
          }
        i++;
        }
      }
    int j=0;
    for (int col=4; col<=7; col++){
      for (int row=7; row>=0; row--){
        if(j<=(32-nCamionesR)){
          lc.setLed(0,col,row,false); // 
          delay(200);
          }
        j++;
        }
      }


    Serial.print("Si desea reiniciar presione el boton ");
    //9/9/22
    
  };
};