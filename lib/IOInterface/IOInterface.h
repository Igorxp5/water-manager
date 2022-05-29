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
        IOInterface(unsigned int pin, IOMode mode);

        virtual unsigned int read() = 0;
        virtual void write(unsigned int w) = 0;
        virtual unsigned int getPin() = 0;

        static IOInterface* get(unsigned int pin);
        static void remove(unsigned int pin);
        static void removeAll();

    protected:
        unsigned int pin;
        IOMode mode;
    
    private:
        static IOInterface** ios;
        static unsigned int* ioPins;
        static unsigned int totalIos;
        static int getIndex(unsigned int pin);
};

class DigitalIO: public IOInterface
{
    public:
        DigitalIO(unsigned int pin, IOMode mode);

        unsigned int read();
        void write(unsigned int w);
        unsigned int getPin();
};

class AnalogicIO: public IOInterface
{
    public:
        AnalogicIO(unsigned int pin, IOMode mode);

        unsigned int read();
        void write(unsigned int w);
        unsigned int getPin();
};

#ifdef TEST
class TestIO: public IOInterface
{
    public:
        TestIO(unsigned int pin);

        unsigned int value = 0; 
        unsigned int read();
        void write(unsigned int w);
        unsigned int getPin();
};
#endif
#endif