## elementos de c++ en AnalogIn
- Clase: AnalogIn es una clase que encapsula la funcionalidad de lectura de voltajes analógicos
- Objeto: Se pueden instanciar objetos de la clase AnalogIn, como AnalogIn ain(A0);, donde ain es un objeto
- Método: La clase AnalogIn tiene métodos como read(), read_u16(), read_voltage(), entre otros
- Constructor: AnalogIn tiene constructores como AnalogIn(PinName pin, float vref=MBED_CONF_TARGET_DEFAULT_ADC_VREF), que inicializa un objeto con un pin específico
- Herencia: No parece haber herencia explícita en AnalogIn, pero podría estar basada en abstracciones internas de Mbed
- Interfaz: La clase AnalogIn proporciona una interfaz para interactuar con el hardware de conversión analógica-digital
- Sobrecarga: Se observa sobrecarga en el operador operator float(), que permite usar un objeto AnalogIn como si fuera un número flotante
- Mensaje: En este contexto, los mensajes pueden referirse a la comunicación entre objetos, como cuando se llama a read() para obtener un valor analógico
## elementos de c++ en USB Serial
- Clase: BufferedSerial y Serial son clases que encapsulan la funcionalidad de comunicación UART
- Objeto: Se pueden instanciar objetos de estas clases, como BufferedSerial serial(USBTX, USBRX);, donde serial es un objeto
- Método: Estas clases tienen métodos como write(), read(), baud(), attach(), entre otros
- Constructor: BufferedSerial tiene constructores como BufferedSerial(PinName tx, PinName rx, int baud), que inicializa un objeto con pines TX y RX
- Herencia: Algunas clases de USB en Mbed heredan de USBDevice, proporcionando funcionalidad específica para dispositivos USB
- Interfaz: BufferedSerial y Serial proporcionan una interfaz para interactuar con el hardware de comunicación serie
- Sobrecarga: Se observa sobrecarga en métodos como attach(), que permite diferentes formas de asignar funciones de interrupción
- Mensaje: En este contexto, los mensajes pueden referirse a la transmisión de datos entre dispositivos mediante UART o USB

  
=======
##TP3 Practica 
PDF:
https://drive.google.com/file/d/1Z-0NNw1D7d_yGwuoITIFb9wjcPgtKVv-/view