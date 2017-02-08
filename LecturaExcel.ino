#include <SdFat.h>
#include <CSVFile.h>
#include <CSVFileConfig.h>
#include <string.h>
#include <SPI.h>

/**
    Project: Riego
    @author: Andres Sanchez Pascual
    @contact: tito97_sp@hotmail.com
    @date: 08.02.2017
    @version: 1.0
    @license: MIT
*/

/**
    Programa de lectura de archivos de area determinadas ("tomates.csv","pepinos.csv","patatas.csv"...) el cual contiene los datos de los horarios de riego de cada area, asi como la duracion.
    Permite obtener el valor de la linea del horario de la siguiente alarma asi como los valores de mes, hora, minutos y duracion esecificos para programar dicha hora.
    Permite un maximo de 15 Areas.
*/




// =*= CONFIG =*=
// SPI pinout
#define PIN_SPI_CLK 13
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 12
#define PIN_SD_CS 10
// If you have connected other SPI device then
// put here number of pin for disable its.
// Provide -1 if you don't have other devices.
#define PIN_OTHER_DEVICE_CS -1
// Change this value if you have problems with SD card
// Available values: SPI_QUARTER_SPEED //SPI_HALF_SPEED
//It is enum from SdFat
#define SD_CARD_SPEED SPI_FULL_SPEED

#define FILENAME "AREAS.csv"


// =*= END CONFIG =*=

String MATRIZAREAS[15];

int ProximaAlarma[6] = {2017, 0, 0, 0, 0, 0};

SdFat sd;
CSVFile csv;
SdFile m;


// =*= PRINCIPAL =*=

void setup() {
  __malloc_heap_end = (char*)RAMEND;

  // Setup pinout
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_CLK, OUTPUT);
  //Disable SPI devices
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);

#if PIN_OTHER_DEVICE_CS > 0
  pinMode(PIN_OTHER_DEVICE_CS, OUTPUT);
  digitalWrite(PIN_OTHER_DEVICE_CS, HIGH);
#endif //PIN_OTHER_DEVICE_CS > 0


  // Setup serial
  Serial.begin(9600);
  while (!Serial) {
    /* wait for Leonardo */
  }
  // Setup SD card
  if (!sd.begin(PIN_SD_CS, SD_CARD_SPEED))
  {
    Serial.println("SD card begin error");
    return;
  }

  // ----------------------------------------------------- //

  actualizarArchivosAreas();
  proximaFecha(2017, 2, 2, 3, 0);
  for (int i = 0; i < 6; i++) {
    Serial.println(ProximaAlarma[i]);
  }



}

void loop() {
}


/*
    Permite determinar la linea en la que se encuentran los datos de la proxima alarma. Los datos introducidos por orden son el mes, dia, hora y minutos actuales,
    es decir a tiempo real.
    No se introduce la posibilidad de evaluar cuando las filas se han acabado. En el futuro sera una funcionalidad clave.
*/
int LineaMinima(int mesNow, int diaNow, int horaNow, int minutoNow, String ARCHIVO) {

  const byte BUFFER_SIZE = 5;
  char buffer[BUFFER_SIZE + 1];
  buffer[BUFFER_SIZE] = '\0';

  int result = 0;

  bool continuar = true;

  csv.open(String2Char(ARCHIVO), O_RDWR | O_CREAT);
  csv.gotoBeginOfFile();

  while ((!csv.isEndOfLine()) && (continuar == true)) {
    int mesBuffer = 0;
    int diaBuffer = 0;
    int horaBuffer = 0;
    int minutoBuffer = 0;

    csv.gotoField(0);
    csv.readField(mesBuffer, buffer, BUFFER_SIZE);

    if (mesBuffer < mesNow) {
      result++;
    }
    else if (mesBuffer == mesNow) {
      csv.gotoField(1);
      csv.readField(diaBuffer, buffer, BUFFER_SIZE);
      if (diaBuffer < diaNow) {
        result++;
      }
      else if (diaBuffer == diaNow) {
        csv.gotoField(2);
        csv.readField(horaBuffer, buffer, BUFFER_SIZE);
        if (horaBuffer < horaNow) {
          result ++;
        }
        else if (horaBuffer == horaNow) {
          csv.gotoField(3);
          csv.readField(minutoBuffer, buffer, BUFFER_SIZE);
          if (minutoBuffer < minutoNow) {
            result++;
          }
          else if (minutoBuffer == minutoNow) {
            continuar = false;
          }
          else if (minutoBuffer > minutoNow) {
            continuar = false;
          }
        }
        else if (horaBuffer > horaNow) {
          continuar = false;
        }
      }
      else if (diaBuffer > diaNow) {
        continuar = false;
      }
    }
    else if (mesBuffer > mesNow) {
      continuar = false;
    }
    csv.nextLine();
  }
  csv.close();
  return result;
}

/*
   Permite la lectura numerica del valor correspondiente de la celda indicado el numero de fila y columna.
   El valor de retorno es un int.
*/

int leerCasilla(int Linea, int Columna, String ARCHIVO) {
  const byte BUFFER_SIZE = 5;
  char buffer[BUFFER_SIZE + 1];
  buffer[BUFFER_SIZE] = '\0';

  int valor = 0;

  csv.open(String2Char(ARCHIVO), O_RDWR | O_CREAT);
  csv.gotoBeginOfFile();
  csv.gotoLine(Linea);
  csv.gotoField(Columna);
  csv.readField(valor, buffer, BUFFER_SIZE);
  csv.close();
  return valor;
}


// ESPECIFICOS PARA LOS NOMBRE Y ARCHIVOS AREAS //

/*
   Permite la lectura alfabetica del valor correspondiente de la celda indicado el numero de fila y de columna.
   Para su uso de lectura del nombre de las Areas, el valor de la Columna permanecera como 0.
   Devuelve un String con el valor alfabetico de la celda correspondiente y con el valor nulo '\0' como ultimo valor del string.
*/

String leerNombreArea(int Fila, int Columna) {

  const byte BUFFER_SIZE = 15;
  char buffer[BUFFER_SIZE + 1];
  buffer[BUFFER_SIZE] = '\0';

  csv.open(FILENAME, O_RDWR | O_CREAT);
  csv.gotoBeginOfFile();
  csv.gotoField(Columna);
  csv.gotoLine(Fila);
  csv.readField(buffer, BUFFER_SIZE);
  csv.close();
  String m = String(buffer);
  return m;
}

/*
   Permite la modificacion del String obtenido en el metodo leerNombreArea(int Fila, 0), con el objetivo de eliminar el valor nulo '\0' del final del String y agragar la terminacion .csv
   y el valor nulo '\0'.
   Se trata de una clase especifica para la creacion del nombre usado para crear los archivos de las Areas, luego su uso se reduce a ese especifico.
   Rellena la matriz de Strings "MATRIZAREAS", donde se encuentran los nombres de los archivos pertenecientes a cada Area.
   No retorna nada.
   Metodo especifico que busca los archivos de la MATRIZAREAS en la targeta SD y si no existen se encarga de crearlos nuevos con el nombre indicado.
   Metodo especifico.
   Para poder usar este metodo, se ha de invocar primero al metodo actualizarNombreAreas, para evitar que MATRIZAREAS sea nulo.
*/



void actualizarArchivosAreas() {

  String BUFF[15];

  for (int i = 0; i < 15; i++) {
    //OK
    String u = leerNombreArea(i, 0);

    BUFF[i] = u;

    int longitud = BUFF[i].length();

    if (longitud != 0) {
      char m [longitud + 3];

      BUFF[i].toCharArray(m, longitud + 1);
      m[longitud + 3] = '\0';
      m[longitud - 1] = '.';
      m[longitud] = 'c';
      m[longitud + 1] = 's';
      m[longitud + 2] = 'v';

      MATRIZAREAS[i] = String(m);
    }
  }

  for (int i = 0 ; i < 15; i++) {
    m.open(String2Char(MATRIZAREAS[i]), O_CREAT | O_WRITE);
    m.close();
  }
}


// =*= HERRAMIENTAS =*= //

/*
     Metodo para la conversion de String a puntero char (char*).
*/
char* String2Char(String Nombre) {
  if (Nombre.length() != 0) {
    char *p = const_cast<char*>(Nombre.c_str());
    return p;
  }
}


void proximaFecha(int ano, int mes, int dia, int hora, int minuto) {                                   
  
  int Ano = ano;
  int Mes = mes;
  int Dia = dia;
  int Hora = hora;
  int Minuto = minuto;

  for (int i = 0; i < 15 ; i++) {
    //Calculo de la linea minima del primer archivo AREA.
    if (i == 0) {
      if(MATRIZAREAS[i].length() != 3){
      int u = LineaMinima(Mes, Dia, Hora, Minuto, MATRIZAREAS[i]);
      int bufferAlarma[6];
      bufferAlarma[0] = Ano;
      bufferAlarma[1] = leerCasilla(u, 0, MATRIZAREAS[i]);                         //MES
      bufferAlarma[2] = leerCasilla(u, 1, MATRIZAREAS[i]);                         //DIA
      bufferAlarma[3] = leerCasilla(u, 2, MATRIZAREAS[i]);                         //HORA
      bufferAlarma[4] = leerCasilla(u, 3, MATRIZAREAS[i]);                         //MINUTO
      bufferAlarma[5] = leerCasilla(u, 4, MATRIZAREAS[i]);                         //DURACION
      for (int i = 0; i < 6; i++) {
        ProximaAlarma[i] = bufferAlarma[i]; //Sustitucion del vector alarma.
      }
    }}
    //Calculo de la linea minima de los siguientes archivos AREA.

    else {
      int u = LineaMinima(Mes, Dia, Hora, Minuto, MATRIZAREAS[i]);
      int bufferAlarma[6];
      bufferAlarma[0] = Ano;
      bufferAlarma[1] = leerCasilla(u, 0, MATRIZAREAS[i]);                        //MES
      bufferAlarma[2] = leerCasilla(u, 1, MATRIZAREAS[i]);                        //DIA
      bufferAlarma[3] = leerCasilla(u, 2, MATRIZAREAS[i]);                        //HORA
      bufferAlarma[4] = leerCasilla(u, 3, MATRIZAREAS[i]);                        //MINUTO
      bufferAlarma[5] = leerCasilla(u, 4, MATRIZAREAS[i]);                        //DURACION

      if ((bufferAlarma[1] != 0) || (bufferAlarma[2] != 0) || (bufferAlarma[3] != 0) || (bufferAlarma[4] != 0)) {
        if (bufferAlarma[1] < ProximaAlarma[1]) {
          for (int i = 0; i < 6; i++) {
            ProximaAlarma[i] = bufferAlarma[i];
          }
        }
        else if (bufferAlarma[1] == ProximaAlarma[1]) {
          if (bufferAlarma[2] < ProximaAlarma[2]) {
            for (int i = 0; i < 6; i++) {
              ProximaAlarma[i] = bufferAlarma[i];
            }
          }
          else if (bufferAlarma[2] == ProximaAlarma[2]) {
            if (bufferAlarma[3] < ProximaAlarma[3]) {
              for (int i = 0; i < 6; i++) {
                ProximaAlarma[i] = bufferAlarma[i];
              }
            }
            else if (bufferAlarma[3] == ProximaAlarma[3]) {
              if (bufferAlarma[4] < ProximaAlarma[4]) {
                for (int i = 0; i < 6; i++) {
                  ProximaAlarma[i] = bufferAlarma[i];
                }
              }
              else if (bufferAlarma[4] == ProximaAlarma[4]) {
                if (bufferAlarma[5] < ProximaAlarma[5]) {
                  for (int i = 0; i < 6; i++) {
                    ProximaAlarma[i] = bufferAlarma[i];
                  }
                }
              }
            }
          }
        }
      }
      else{break;}
   }
}
}


void waitForKey()
{
  while (Serial.read() >= 0) { }
  Serial.println(F("Type any character to repeat.\n"));
  while (Serial.read() <= 0) { }
}

void initSdFile()
{
  if (sd.exists(FILENAME) && !sd.remove(FILENAME))
  {
    Serial.println("Failed init remove file");
    return;
  }
  if (!csv.open(FILENAME, O_RDWR | O_CREAT)) {
    Serial.println("Failed open file");
  }
}
