
#ifndef Sounds_h
#define Sounds_h

#include "Arduino.h"

class Sounds
{
public:
  Sounds(int port);
  void Temperature();
  void Start();
  void Sucess();
  void Error();
  void Shutdown();
  void BluetoothNewPreference();
  void NewMQTTPreference();
private:
  int _port; 
};

#endif