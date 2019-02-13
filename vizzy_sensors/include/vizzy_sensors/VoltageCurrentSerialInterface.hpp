#include <string>
#include <iostream>
#include <cstdio>

// OS Specific sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial/serial.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;

class VoltageCurrentSerialInterface
{
    double voltage;
    double current;
    serial::Serial *my_serial;
    static const unsigned long baud = 115200;
    public:
    bool initComm(string port){
        my_serial = new serial::Serial(port, baud, serial::Timeout::simpleTimeout(1000));
        if(my_serial->isOpen()){
            cout << port << " open at " << baud << endl;
            return true;
        }
        else
        {
            return false;
        }
        
    }
    void closeComm(){
      my_serial->close();
    }
    int getFirmwareVersion(std::string &firmware_version){
        string test_string(1,' ');
        my_serial->flushInput();
        size_t bytes_wrote = my_serial->write(test_string);
        string result = my_serial->read(29);
        firmware_version.resize(25);
        if (result[0]==test_string[0]){
            firmware_version = result.substr(1,25);
            my_serial->flushOutput();
            return 1;
        }
        else{
             my_serial->flushOutput();
            return -1;
        }
    }
    int getSystemPowerSupply(double &current,double &voltage){
        string test_string(1,'0');
        //cout << "Writing " << test_string << endl;
        size_t bytes_wrote = my_serial->write(test_string);
        usleep(300*1000);
        std::vector< uint8_t > buffer;
        int size_read = my_serial->read(buffer,8);
        usleep(300*1000);
        if (size_read == 8 && buffer[0]==test_string[0]){
            //cout << "Going to compute" << endl;
            int bytesSum=0;
            for (int my_c=0;my_c<6;my_c++){
                bytesSum+=buffer[my_c];
            }
            int checksum = buffer[6]*0x0100+ buffer[7]*0x0001;
            if (checksum == bytesSum){
                voltage = (buffer[1]*0x0100+ buffer[2]*0x0001)/100.0;
                current = (buffer[3]*0x0100+ buffer[4]*0x0001)/100.0;
                return 0;
            }
            else{
             return -1;
            }
        }
        else{
            return -1;
        }
    }
};
