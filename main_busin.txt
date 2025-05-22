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
// OBJETO: Grupo de entradas del puerto E
PortIn sensorInputs(PE_12); // MQ2 y otros sensores del mismo puerto

//=====[Declaración de salidas digitales con DigitalOut]=====
DigitalOut alarmLed(LED1);
DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

//=====[Comunicación serial con UnbufferedSerial]=====
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Lecturas analógicas con AnalogIn]=====
AnalogIn potentiometer(A0);
AnalogIn lm35(A1);

//=====[Modificación de alarmDeactivationUpdate para usar BusIn]=====

void alarmDeactivationUpdate()
{
    if ( numberOfIncorrectCodes < 5 ) {
        if ( buttonInputs == 0b1111 && !enterButton ) {  // Verifica si todos los botones están presionados
            incorrectCodeLed = OFF;
        }
        if ( enterButton && !incorrectCodeLed && alarmState ) {
            int pressedButtons = buttonInputs.read(); // Lee el estado de todos los botones
            for (int i = 0; i < NUMBER_OF_KEYS; i++) {
                buttonsPressed[i] = (pressedButtons >> i) & 1; // Extrae cada botón como un bit
            }
            if (areEqual()) {
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
