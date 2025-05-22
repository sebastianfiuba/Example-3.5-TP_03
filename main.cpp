//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

//=====[Defines]===============================================================

#define NUMBER_OF_KEYS                           4
#define BLINKING_TIME_GAS_ALARM               1000
#define BLINKING_TIME_OVER_TEMP_ALARM          500
#define BLINKING_TIME_GAS_AND_OVER_TEMP_ALARM  100
#define NUMBER_OF_AVG_SAMPLES                   100
#define OVER_TEMP_LEVEL                         50
#define TIME_INCREMENT_MS                       10

//=====[Declaration and initialization of public global objects]===============



/*
//=====[Declaración de objetos de clases DigitalIn, DigitalOut y DigitalInOut]=====
// CLASES: DigitalIn, DigitalOut, DigitalInOut -> Son clases de Mbed para manejar pines de entrada y salida
// CONSTRUCTOR: Se usa el constructor de cada clase para crear objetos con parámetros de inicialización (Ej. el pin correspondiente)
// OBJETOS: Cada declaración es una instancia de las respectivas clases


DigitalIn enterButton(BUTTON1);       // Objeto de la clase DigitalIn que representa el botón de entrada
DigitalIn alarmTestButton(D2);        // Otro objeto de DigitalIn para la prueba de alarma
DigitalIn aButton(D4);                // Objeto para el botón A
DigitalIn bButton(D5);                // Objeto para el botón B
DigitalIn cButton(D6);                // Objeto para el botón C
DigitalIn dButton(D7);                // Objeto para el botón D
DigitalIn mq2(PE_12);                 // Sensor MQ2 para detección de gases
*/

//=====[Uso de BusIn para agrupar múltiples entradas digitales]=====
// CLASE: BusIn -> Permite leer múltiples pines como un solo valor binario
// CONSTRUCTOR: Recibe los pines agrupados como parámetros
// OBJETO: Grupo de botones en una sola variable
BusIn buttonInputs(D4, D5, D6, D7); // AButton, BButton, CButton y DButton en un solo objeto
DigitalIn enterButton(BUTTON1);
DigitalIn alarmTestButton(D2);
DigitalIn mq2(PE_12);

//=====[Uso de PortIn para manejar pines de un puerto específico]=====
// CLASE: PortIn -> Maneja entradas de un conjunto de pines basados en el puerto del microcontrolador
// CONSTRUCTOR: No requiere pines específicos, solo define el puerto
// OBJETO: Grupo de entradas del puerto 


//=====[Uso de BusOut para agrupar múltiples salidas digitales]=====
// CLASE: BusOut -> Permite manejar múltiples pines digitales como un solo valor
// CONSTRUCTOR: Recibe los pines agrupados como parámetros
// OBJETO: LEDs que indican estado del sistema
BusOut statusLeds(LED1, LED2, LED3); // Agrupa los LEDs de alarma, código incorrecto y bloqueo del sistema



/*
DigitalOut alarmLed(LED1);            // Objeto de DigitalOut para manejar el LED de alarma
DigitalOut incorrectCodeLed(LED3);    // LED que indica código incorrecto
DigitalOut systemBlockedLed(LED2);    // LED que indica sistema bloqueado


*/

DigitalInOut sirenPin(PE_10); // Entrada/Salida para la sirena




//=====[Declaración de objeto de la clase UnbufferedSerial]===================
// CLASE: UnbufferedSerial -> Maneja comunicación serial sin buffer en Mbed
// INTERFAZ: La clase UnbufferedSerial proporciona una interfaz para comunicación serial con métodos como read() y write()
// OBJETO: Creación de un objeto uartUsb usando el constructor de la clase UnbufferedSerial
UnbufferedSerial uartUsb(USBTX, USBRX, 115200); // Comunicación serie con velocidad de 115200 baudios

// Constructor UnbufferedSerial inicializa un objeto con pones RX TX y baudrate
//mas rapido, no hay buffer y puedo seleccionar el baudrate.

//=====[Declaración de objeto de la clase AnalogIn]==========================
// CLASE: AnalogIn -> Maneja entradas analógicas en Mbed
// CONSTRUCTOR: Recibe el pin analógico como argumento
AnalogIn potentiometer(A0);           // Potenciómetro conectado a A0
AnalogIn lm35(A1);                    // Sensor LM35 para temperatura conectado a A1

// Constructor AnalogIn inicializa objeto con un pin


//=====[Declaration and initialization of public global variables]=============

bool alarmState    = OFF;
bool incorrectCode = false;
bool overTempDetector = OFF;

int numberOfIncorrectCodes = 0;
int buttonBeingCompared    = 0;
int codeSequence[NUMBER_OF_KEYS]   = { 1, 1, 0, 0 };
int buttonsPressed[NUMBER_OF_KEYS] = { 0, 0, 0, 0 };
int accumulatedTimeAlarm = 0;

bool gasDetectorState          = OFF;
bool overTempDetectorState     = OFF;


float potentiometerReading = 0.0;
float lm35ReadingsAverage  = 0.0;
float lm35ReadingsSum      = 0.0;
float lm35ReadingsArray[NUMBER_OF_AVG_SAMPLES];
float lm35TempC            = 0.0;

uint8_t inputs = 0x0;
uint8_t outputs = 0x0;
uint8_t ant_inputs;
uint8_t ant_outputs;

//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
void outputsInit(uint8_t* outputs_a);
void alarmActivationUpdate(uint8_t* outputs_a);
void alarmDeactivationUpdate(uint8_t* pressedButtons, uint8_t* outputs_a);

void uartTask();
void availableCommands();
bool areEqual();
float celsiusToFahrenheit( float tempInCelsiusDegrees );
float analogReadingScaledWithTheLM35Formula( float analogReading );
void print_gpio(bool alarmState_a, uint8_t sensors_byte_a, uint8_t buttons_byte_a);
void printandcheck(uint8_t* ant_buttons_byte, uint8_t* ant_sensors_byte, uint8_t buttons_byte, uint8_t sensors_byte);


//=====[Main function, the program entry point after power on or reset]========

int main()
{
    inputsInit();
    outputsInit(&outputs);
    while (true) {
        alarmActivationUpdate(&outputs);
        alarmDeactivationUpdate(&inputs, &outputs);
        uartTask();
        printandcheck(&ant_inputs, &ant_outputs, inputs, outputs);
        delay(TIME_INCREMENT_MS);
    }
}

//=====[Implementations of public functions]===================================




void inputsInit()
{
    alarmTestButton.mode(PullDown);
  /*  aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);
    */
    buttonInputs.mode(PullDown);
    sirenPin.mode(OpenDrain);
    sirenPin.input();
}



//=====[Modificación de outputsInit para usar BusOut]=====
void outputsInit(uint8_t* outputs_a)
{
    statusLeds = 0b000; // Apaga todos los LEDs al inicio

    *outputs_a = statusLeds.read();
}
/*
void outputsInit()
{
    alarmLed = OFF;
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
}
*/

/*
void alarmActivationUpdate()
{
    static int lm35SampleIndex = 0;
    int i = 0;

    lm35ReadingsArray[lm35SampleIndex] = lm35.read();
    lm35SampleIndex++;
    if ( lm35SampleIndex >= NUMBER_OF_AVG_SAMPLES) {
        lm35SampleIndex = 0;
    }
    
       lm35ReadingsSum = 0.0;
    for (i = 0; i < NUMBER_OF_AVG_SAMPLES; i++) {
        lm35ReadingsSum = lm35ReadingsSum + lm35ReadingsArray[i];
    }
    lm35ReadingsAverage = lm35ReadingsSum / NUMBER_OF_AVG_SAMPLES;
       lm35TempC = analogReadingScaledWithTheLM35Formula ( lm35ReadingsAverage );    
    
    if ( lm35TempC > OVER_TEMP_LEVEL ) {
        overTempDetector = ON;
    } else {
        overTempDetector = OFF;
    }

    if( !mq2) {
        gasDetectorState = ON;
        alarmState = ON;
    }
    if( overTempDetector ) {
        overTempDetectorState = ON;
        alarmState = ON;
    }
    if( alarmTestButton ) {             
        overTempDetectorState = ON;
        gasDetectorState = ON;
        alarmState = ON;
    }    
    if( alarmState ) { 
        accumulatedTimeAlarm = accumulatedTimeAlarm + TIME_INCREMENT_MS;
        sirenPin.output();                                     
        sirenPin = LOW;                                        
    
        if( gasDetectorState && overTempDetectorState ) {
            if( accumulatedTimeAlarm >= BLINKING_TIME_GAS_AND_OVER_TEMP_ALARM ) {
                accumulatedTimeAlarm = 0;
                alarmLed = !alarmLed;
            }
        } else if( gasDetectorState ) {
            if( accumulatedTimeAlarm >= BLINKING_TIME_GAS_ALARM ) {
                accumulatedTimeAlarm = 0;
                alarmLed = !alarmLed;
            }
        } else if ( overTempDetectorState ) {
            if( accumulatedTimeAlarm >= BLINKING_TIME_OVER_TEMP_ALARM  ) {
                accumulatedTimeAlarm = 0;
                alarmLed = !alarmLed;
            }
        }
    } else{
        alarmLed = OFF;
        gasDetectorState = OFF;
        overTempDetectorState = OFF;
        sirenPin.input();                                  
    }
}

*/

//=====[Modificación de alarmActivationUpdate para usar BusOut y PortOut]=====

void alarmActivationUpdate(uint8_t* outputs_a)
{
    static int lm35SampleIndex = 0;
    int i = 0;
    lm35ReadingsArray[lm35SampleIndex] = lm35.read();
    lm35SampleIndex++;
    
    if (lm35SampleIndex >= NUMBER_OF_AVG_SAMPLES) {
        lm35SampleIndex = 0;
    }

    lm35ReadingsSum = 0.0;
    for (i = 0; i < NUMBER_OF_AVG_SAMPLES; i++) {
        lm35ReadingsSum += lm35ReadingsArray[i];
    }

    lm35ReadingsAverage = lm35ReadingsSum / NUMBER_OF_AVG_SAMPLES;
    lm35TempC = analogReadingScaledWithTheLM35Formula(lm35ReadingsAverage);    

    if (lm35TempC > OVER_TEMP_LEVEL) {
        overTempDetector = ON;
    } else {
        overTempDetector = OFF;
    }

    if (!mq2) {  
        gasDetectorState = ON;
        alarmState = ON;
    }

    if (overTempDetector) {  
        overTempDetectorState = ON;
        alarmState = ON;
    }

    if (alarmTestButton) {  
        overTempDetectorState = ON;
        gasDetectorState = ON;
        alarmState = ON;
    }    

    if (alarmState) { 
        accumulatedTimeAlarm += TIME_INCREMENT_MS;
         
        if (gasDetectorState && overTempDetectorState) {
            if (accumulatedTimeAlarm >= BLINKING_TIME_GAS_AND_OVER_TEMP_ALARM) {
                accumulatedTimeAlarm = 0;
                statusLeds = statusLeds ^ 0b001; // Alterna el LED de alarma
            }
        } else if (gasDetectorState) {
            if (accumulatedTimeAlarm >= BLINKING_TIME_GAS_ALARM) {
                accumulatedTimeAlarm = 0;
                statusLeds = statusLeds ^ 0b001;
            }
        } else if (overTempDetectorState) {
            if (accumulatedTimeAlarm >= BLINKING_TIME_OVER_TEMP_ALARM) {
                accumulatedTimeAlarm = 0;
                statusLeds = statusLeds ^ 0b001;
            }
        }
    } else {
        statusLeds = 0b000; // Apaga los LEDs de estado
        gasDetectorState = OFF;
        overTempDetectorState = OFF;
           }


    *outputs_a = statusLeds.read();

    return;
}



void alarmDeactivationUpdate(uint8_t* pressedButtons, uint8_t* outputs_a)
{

    *pressedButtons = buttonInputs.read() & buttonInputs.mask();

    if ( numberOfIncorrectCodes < 5 ) {
        if ( *pressedButtons == 0b1111 && !enterButton ) {  // Verifica si todos los botones están presionados
            statusLeds = statusLeds & 0b011;
        }
        if ( enterButton && !(statusLeds.read() & 0b100) && alarmState ) {
             // Lee el estado de todos los botones
            for (int i = 0; i < NUMBER_OF_KEYS; i++) {
                buttonsPressed[i] = (*pressedButtons >> i) & 1; // Extrae cada botón como un bit
            }
            if (areEqual()) {
                alarmState = OFF;
                numberOfIncorrectCodes = 0;
            } else {
                statusLeds = statusLeds  |  0b100;
                numberOfIncorrectCodes++;
            }
        }
    } else {
        statusLeds = statusLeds  |  0b010;
    }


    *outputs_a = statusLeds.read();

    return;
}

/*


void alarmDeactivationUpdate()
{
    if ( numberOfIncorrectCodes < 5 ) {
        if ( aButton && bButton && cButton && dButton && !enterButton ) {
            incorrectCodeLed = OFF;
        }
        if ( enterButton && !incorrectCodeLed && alarmState ) {
            buttonsPressed[0] = aButton;
            buttonsPressed[1] = bButton;
            buttonsPressed[2] = cButton;
            buttonsPressed[3] = dButton;
            if ( areEqual() ) {
                alarmState = OFF;
                numberOfIncorrectCodes = 0;
            } else {
                incorrectCodeLed = ON;
                numberOfIncorrectCodes++;
            }
        }
    } else {
        systemBlockedLed = ON;
    }
}
*/


//=====[Definición de una función con un mensaje entre objetos]=============
// MENSAJE: La comunicación entre objetos ocurre cuando se llama a métodos de un objeto. Ejemplo: uartUsb.read()
// INTERFAZ: La función uartTask() utiliza la interfaz de UnbufferedSerial para leer y escribir datos seriales

void uartTask()
{
    char receivedChar = '\0';
    char str[100];
    int stringLength;
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
        switch (receivedChar) {
        case '1':
            if ( alarmState ) {
                uartUsb.write( "The alarm is activated\r\n", 24);
            } else {
                uartUsb.write( "The alarm is not activated\r\n", 28);
            }
            break;

        case '2':
            if ( !mq2 ) {
                uartUsb.write( "Gas is being detected\r\n", 22);
            } else {
                uartUsb.write( "Gas is not being detected\r\n", 27);
            }
            break;

        case '3':
            if ( overTempDetector ) {
                uartUsb.write( "Temperature is above the maximum level\r\n", 40);
            } else {
                uartUsb.write( "Temperature is below the maximum level\r\n", 40);
            }
            break;
            
        case '4':
            uartUsb.write( "Please enter the code sequence.\r\n", 33 );
            uartUsb.write( "First enter 'A', then 'B', then 'C', and ", 41 ); 
            uartUsb.write( "finally 'D' button\r\n", 20 );
            uartUsb.write( "In each case type 1 for pressed or 0 for ", 41 );
            uartUsb.write( "not pressed\r\n", 13 );
            uartUsb.write( "For example, for 'A' = pressed, ", 32 );
            uartUsb.write( "'B' = pressed, 'C' = not pressed, ", 34);
            uartUsb.write( "'D' = not pressed, enter '1', then '1', ", 40 );
            uartUsb.write( "then '0', and finally '0'\r\n\r\n", 29 );

            incorrectCode = false;

            for ( buttonBeingCompared = 0;
                  buttonBeingCompared < NUMBER_OF_KEYS;
                  buttonBeingCompared++) {

                uartUsb.read( &receivedChar, 1 );
                uartUsb.write( "*", 1 );

                if ( receivedChar == '1' ) {
                    if ( codeSequence[buttonBeingCompared] != 1 ) {
                        incorrectCode = true;
                    }
                } else if ( receivedChar == '0' ) {
                    if ( codeSequence[buttonBeingCompared] != 0 ) {
                        incorrectCode = true;
                    }
                } else {
                    incorrectCode = true;
                }
            }

            if ( incorrectCode == false ) {
                uartUsb.write( "\r\nThe code is correct\r\n\r\n", 25 );
                alarmState = OFF;
                statusLeds = statusLeds  &  0b011;
                numberOfIncorrectCodes = 0;
            } else {
                uartUsb.write( "\r\nThe code is incorrect\r\n\r\n", 27 );
                statusLeds = statusLeds  |  0b100;
                numberOfIncorrectCodes++;
            }                
            break;

        case '5':
            uartUsb.write( "Please enter new code sequence\r\n", 32 );
            uartUsb.write( "First enter 'A', then 'B', then 'C', and ", 41 );
            uartUsb.write( "finally 'D' button\r\n", 20 );
            uartUsb.write( "In each case type 1 for pressed or 0 for not ", 45 );
            uartUsb.write( "pressed\r\n", 9 );
            uartUsb.write( "For example, for 'A' = pressed, 'B' = pressed,", 46 );
            uartUsb.write( " 'C' = not pressed,", 19 );
            uartUsb.write( "'D' = not pressed, enter '1', then '1', ", 40 );
            uartUsb.write( "then '0', and finally '0'\r\n\r\n", 29 );

            for ( buttonBeingCompared = 0; 
                  buttonBeingCompared < NUMBER_OF_KEYS; 
                  buttonBeingCompared++) {

                uartUsb.read( &receivedChar, 1 );
                uartUsb.write( "*", 1 );

                if ( receivedChar == '1' ) {
                    codeSequence[buttonBeingCompared] = 1;
                } else if ( receivedChar == '0' ) {
                    codeSequence[buttonBeingCompared] = 0;
                }
            }

            uartUsb.write( "\r\nNew code generated\r\n\r\n", 24 );
            break;
 
        case 'p':
        case 'P':
            potentiometerReading = potentiometer.read();
            sprintf ( str, "Potentiometer: %.2f\r\n", potentiometerReading );
            stringLength = strlen(str);
            uartUsb.write( str, stringLength );
            break;

        case 'c':
        case 'C':
            sprintf ( str, "Temperature: %.2f \xB0 C\r\n", lm35TempC );
            stringLength = strlen(str); 
            uartUsb.write( str, stringLength );
            break;

        case 'f':
        case 'F':
            sprintf ( str, "Temperature: %.2f \xB0 F\r\n", 
                celsiusToFahrenheit( lm35TempC ) );
            stringLength = strlen(str);
            uartUsb.write( str, stringLength );
            break;

        default:
            availableCommands();
            break;

        }
    }
}


void availableCommands()
{
    uartUsb.write( "Available commands:\r\n", 21 );
    uartUsb.write( "Press '1' to get the alarm state\r\n", 34 );
    uartUsb.write( "Press '2' to get the gas detector state\r\n", 41 );
    uartUsb.write( "Press '3' to get the over temperature detector state\r\n", 54 );
    uartUsb.write( "Press '4' to enter the code sequence\r\n", 38 );
    uartUsb.write( "Press '5' to enter a new code\r\n", 31 );
    uartUsb.write( "Press 'P' or 'p' to get potentiometer reading\r\n", 47 );
    uartUsb.write( "Press 'f' or 'F' to get lm35 reading in Fahrenheit\r\n", 52 );
    uartUsb.write( "Press 'c' or 'C' to get lm35 reading in Celsius\r\n\r\n", 51 );
}

bool areEqual()
{
    int i;

    for (i = 0; i < NUMBER_OF_KEYS; i++) {
        if (codeSequence[i] != buttonsPressed[i]) {
            return false;
        }
    }

    return true;
}

float analogReadingScaledWithTheLM35Formula( float analogReading )
{
    return ( analogReading * 3.3 / 0.01 );
}



//=====[Ejemplo de sobrecarga de método]=========================
// CLASE: Se define sobrecarga en la función celsiusToFahrenheit(), ya que recibe diferentes tipos de datos

float celsiusToFahrenheit( float tempInCelsiusDegrees )
{
    return ( tempInCelsiusDegrees * 9.0 / 5.0 + 32.0 );
}



void printandcheck(uint8_t* ant_buttons_byte, uint8_t* ant_sensors_byte, uint8_t buttons_byte, uint8_t sensors_byte){
        if((sensors_byte != *ant_sensors_byte) || (buttons_byte != *ant_buttons_byte)){

            print_gpio(alarmState, sensors_byte, buttons_byte);
            *ant_buttons_byte = buttons_byte;
            *ant_sensors_byte = sensors_byte;

        }


}

void print_gpio(bool alarmState_a, uint8_t sensors_byte_a, uint8_t buttons_byte_a){


    if(alarmState_a)
        printf("El led esta prendido\n");
    else
        printf("El led esta apagado\n");

    if (sensors_byte_a & 0x1 )
        printf("Se detecta gas");
    else
        printf("No se detecta gas");
    printf("\n");
    
    if (sensors_byte_a & 0x2 )
        printf("Se paso la temperatura");
    else
        printf("No se paso la temperatura");
    printf("\n");

    if (buttons_byte_a & 0x1)
        printf("Se apreto el boton \"a\"");
    else
        printf("No se apreto el boton \"a\"");
    printf("\n");
    
    if (buttons_byte_a & 0x2)
        printf("Se apreto el boton \"b\"");
    else
        printf("No se apreto el boton \"b\"");
    printf("\n");
    if (buttons_byte_a & 0x4 )
        printf("Se apreto el boton \"c\"");
    else
        printf("No se apreto el boton \"c\"");
    printf("\n");

    if (buttons_byte_a & 0x8 )
        printf("Se apreto el boton \"d\"");
    else
        printf("No se apreto el boton \"d\"");
    printf("\n");

}