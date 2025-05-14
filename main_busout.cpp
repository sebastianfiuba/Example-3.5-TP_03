//=====[Uso de BusOut para agrupar múltiples salidas digitales]=====
// CLASE: BusOut -> Permite manejar múltiples pines digitales como un solo valor
// CONSTRUCTOR: Recibe los pines agrupados como parámetros
// OBJETO: LEDs que indican estado del sistema
BusOut statusLeds(LED1, LED2, LED3); // Agrupa los LEDs de alarma, código incorrecto y bloqueo del sistema

DigitalInOut sirenPin(PE_10); // Entrada/Salida para la sirena


//=====[Modificación de outputsInit para usar BusOut]=====
void outputsInit()
{
    statusLeds = 0b000; // Apaga todos los LEDs al inicio
}

//=====[Modificación de alarmActivationUpdate para usar BusOut y PortOut]=====

void alarmActivationUpdate()
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
        externalDevices = LOW; // Activa los dispositivos en el puerto E
        
        if (gasDetectorState && overTempDetectorState) {
            if (accumulatedTimeAlarm >= BLINKING_TIME_GAS_AND_OVER_TEMP_ALARM) {
                accumulatedTimeAlarm = 0;
                statusLeds ^= 0b001; // Alterna el LED de alarma
            }
        } else if (gasDetectorState) {
            if (accumulatedTimeAlarm >= BLINKING_TIME_GAS_ALARM) {
                accumulatedTimeAlarm = 0;
                statusLeds ^= 0b001;
            }
        } else if (overTempDetectorState) {
            if (accumulatedTimeAlarm >= BLINKING_TIME_OVER_TEMP_ALARM) {
                accumulatedTimeAlarm = 0;
                statusLeds ^= 0b001;
            }
        }
    } else {
        statusLeds = 0b000; // Apaga los LEDs de estado
        gasDetectorState = OFF;
        overTempDetectorState = OFF;
        externalDevices = HIGH; // Desactiva los dispositivos en el puerto E      
    }
}
