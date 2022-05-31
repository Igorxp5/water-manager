#ifndef INPUT_SOURCE_H
#define INPUT_SOURCE_H

enum IOMode {
    READ_ONLY, WRITE_ONLY, READ_WRITE
};

enum IOType {
    ANY, DIGITAL, ANALOGIC
};

class IOInterface
{
    public:
        IOInterface(unsigned int pin, IOMode mode, IOType type);

        unsigned int read();
        void write(unsigned int w);
        unsigned int getPin();

        static IOInterface* get(unsigned int pin);
        static void remove(unsigned int pin);
        static void removeAll();

    protected:
        unsigned int pin;
        IOMode mode;
        IOType type;
        #ifdef TEST
        unsigned int value = 0;
        #endif
    
    private:
        static IOInterface** ios;
        static unsigned int* ioPins;
        static unsigned int totalIos;
        static int getIndex(unsigned int pin);
};

#endif
