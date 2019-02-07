
#include <vizzy_sensors/VoltageCurrentSerialInterface.hpp>

int main(int argc, char **argv) {
  VoltageCurrentSerialInterface voltage_reader;
  if (voltage_reader.initComm("/dev/ttyUSB1")){
    string firmware_version;
    double voltage;
    double current;
    while (voltage_reader.getFirmwareVersion(firmware_version)){
      cout << firmware_version << endl;
      int res = voltage_reader.getSystemPowerSupply(current,voltage);
      cout << "Voltage: " << voltage << " Current: " << current << endl;
    }
    /*else{
      cout << "Error reading firmware version" << endl;
    }*/
  }
  else
    cout << "Not able to open the port" << endl;
  voltage_reader.closeComm();
  return 0;
}

