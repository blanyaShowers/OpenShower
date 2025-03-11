
#ifndef Sounds_h
#define Sounds_h

#include "Arduino.h"

class Sounds
{
public:
  Sounds(int port);
  void playMelody(const int melody[], const int durations[], int numNotes, int baseDuration = 500);
  void TemperatureReached();
  void Start();
  void Success();
  void Error();
  void Shutdown();
  void BluetoothNewPreference();
  void NewMQTTPreference();
  void OneClick();
private:
  int _port; 
};

#endif