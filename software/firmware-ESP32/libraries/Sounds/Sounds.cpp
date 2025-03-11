// Base on https://github.com/electricmango

#include "Arduino.h"
#include "Sounds.h"

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978
#define NOTE_E8 5274 



Sounds::Sounds(int port) : _port(port) {}

void Sounds::playMelody(const int melody[], const int durations[], int numNotes, int baseDuration) {
    for (int i = 0; i < numNotes; i++) {
        int noteDuration = baseDuration / durations[i];
        tone(_port, melody[i], noteDuration);
        delay(noteDuration * 1.2); // 20% pause <button class="citation-flag" data-index="5">
        noTone(_port);
    }
}

void Sounds::Start() {
    const int melody[] = {NOTE_E7, NOTE_G7, NOTE_C8, NOTE_E8}; // Extended rising sequence
    const int durations[] = {4, 4, 8, 8};
    playMelody(melody, durations, 4, 500);
}

void Sounds::Shutdown() {
    // Descending triad + hum <button class="citation-flag" data-index="2">
    const int melody[] = {NOTE_G4, NOTE_E4, NOTE_C4, NOTE_C3};
    const int durations[] = {4, 4, 4, 2}; // 0.4s + 0.4s hum
    const int numNotes = 4;
    int baseDuration = 800 / (3*(1.0/4) + 1*(1.0/2)) / 1.2; // 0.8s total
    playMelody(melody, durations, numNotes, baseDuration);
}

void Sounds::TemperatureReached() {
    // Cluster simulation with rapid alternation <button class="citation-flag" data-index="5">
    const int melody[] = {NOTE_A4, NOTE_D5, NOTE_A4, NOTE_D5};
    const int durations[] = {8, 8, 8, 8}; // 0.125s per note
    const int numNotes = 4;
    int baseDuration = 500 / (4*(1.0/8)) / 1.2; // 0.5s total
    playMelody(melody, durations, numNotes, baseDuration);
}

void Sounds::Error() {
    // Pulsating square wave <button class="citation-flag" data-index="7">
    const int melody[] = {300, 0, 300, 0, 300, 0}; // 300Hz pulses
    const int durations[] = {4, 4, 4, 4, 4, 4}; // 0.2s pulses + gaps
    const int numNotes = 6;
    int baseDuration = 1500 / (6*(1.0/4)) / 1.2; // 1.5s total
    playMelody(melody, durations, numNotes, baseDuration);
}

void Sounds::Success() {
    // Bright chord flourish <button class="citation-flag" data-index="2">
    const int melody[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, NOTE_C6};
    const int durations[] = {4, 4, 4, 4, 2}; // 0.3s rise + 0.4s sustain
    const int numNotes = 5;
    int baseDuration = 700 / (4*(1.0/4) + 1*(1.0/2)) / 1.2; // 0.7s total
    playMelody(melody, durations, numNotes, baseDuration);
}

void Sounds::BluetoothNewPreference() {
    // Digital chirps <button class="citation-flag" data-index="6">
    const int melody[] = {NOTE_E6, NOTE_C5, NOTE_E6};
    const int durations[] = {4, 4, 4}; // 0.3s per tone
    const int numNotes = 3;
    int baseDuration = 900 / (3*(1.0/4)) / 1.2; // 0.9s total
    playMelody(melody, durations, numNotes, baseDuration);
}

void Sounds::NewMQTTPreference() {
    // Cloud chime with echo <button class="citation-flag" data-index="5">
    const int melody[] = {NOTE_F4, NOTE_A4, NOTE_C5, 0, NOTE_C5};
    const int durations[] = {4, 4, 4, 8, 4}; // 0.2s triad + echo
    const int numNotes = 5;
    int baseDuration = 600 / (4*(1.0/4) + 1*(1.0/8) + 1*(1.0/4)) / 1.2; // 0.6s total
    playMelody(melody, durations, numNotes, baseDuration);
}

void Sounds::OneClick() {
    // Single plink <button class="citation-flag" data-index="5">
    const int melody[] = {NOTE_A5};
    const int durations[] = {8}; // 0.125s duration
    const int numNotes = 1;
    int baseDuration = 150 / (1*(1.0/8)) / 1.2; // 0.15s total
    playMelody(melody, durations, numNotes, baseDuration);
}