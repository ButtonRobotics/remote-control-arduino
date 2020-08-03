#ifndef SHARED_SETTINGS_H
#define SHARED_SETTINGS_H

#include <Arduino.h>
const unsigned int BAUD_RATE = 9600;

// PACKET COMPOSITION
const byte PACKET_START = 145;
const byte PACKET_END = 137;

// TIMING
const unsigned int STILL_ALIVE_CHECK_TIMEOUT_IN_MILLIS = 1000;

#endif