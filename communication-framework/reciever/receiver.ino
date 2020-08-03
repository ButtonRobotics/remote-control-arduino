#include ".\src\com_flags.h"
#include ".\src\shared_settings.h"

#define ULONG_MAX 0xffffffffUL

// TIMING
const unsigned int STILL_ALIVE_CHECK_PERIOD_IN_MILLIS = 400;
const unsigned int BLINK_SPEED = 40;

// SESSION VARIABLES
bool serial_connected;
bool sent_still_alive_check_packet;
byte session_key;
byte still_alive_check_key;
unsigned long time_of_still_alive_check;
unsigned long current_time;

bool isOddCycle;
unsigned long blink_cycle;

// PIN SETTINGS
byte led_w = 4;
byte led_g = 6;
byte led_r = 8;
byte led_y = 10;
void setup()
{
  current_time = millis();
  Serial.begin(BAUD_RATE);
  reset();
  ledSetup();
}

void loop()
{
  //Serial.flush();
  current_time = millis();
  checkIncomingCommunication();

  if (serial_connected && !stillAliveCheck())
  {
    reset();
  }

  if (serial_connected)
  {
    digitalWrite(led_r, HIGH);
  }
  else
  {
    digitalWrite(led_r, LOW);
  }
  delay(5);
}

void ledSetup()
{
  pinMode(led_w, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_y, OUTPUT);
  digitalWrite(led_w, LOW);
  digitalWrite(led_g, LOW);
  digitalWrite(led_r, LOW);
  digitalWrite(led_y, LOW);
}

void reset()
{
  printDebug("RESET");
  serial_connected = false;
  session_key = 0;
  time_of_still_alive_check = ULONG_MAX;
  sent_still_alive_check_packet = false;
}

bool stillAliveCheck()
{
  bool isAlive = true;
  if (current_time > time_of_still_alive_check &&
      current_time - time_of_still_alive_check > STILL_ALIVE_CHECK_TIMEOUT_IN_MILLIS)
  {
    isAlive = false;
  }
  else if (!sent_still_alive_check_packet && current_time - time_of_still_alive_check > STILL_ALIVE_CHECK_PERIOD_IN_MILLIS)
  {
    still_alive_check_key = random(255);
    sendStillAliveCheckPacket();
    sent_still_alive_check_packet = true;
  }
  return isAlive;
}

void checkIncomingCommunication()
{
  unsigned int i = 0;
  byte packetParts[5];
  for (byte i = 0; i < 100 && Serial.available() >= 5; i++)
  {
    while (Serial.available() > 0 && Serial.peek() != PACKET_START)
    {
      Serial.read();
    }
    for (byte j = 0; j < 5 && ((j == 4 && Serial.peek() == PACKET_END) || Serial.peek() != PACKET_END); j++)
    {
      packetParts[j] = Serial.read();
      if (j == 4)
      {
        parseSerialPacket(packetParts[4], packetParts[3],
                          packetParts[2], packetParts[1], packetParts[0]);
      }
    }
  }
}

void blink()
{
}

void parseSerialPacket(byte end_pckt, byte val2, byte val1, byte flag, byte start)
{
  if (start != PACKET_START || end_pckt != PACKET_END)
  {
    return;
  }
  digitalWrite(led_g, HIGH);
  delay(50);
  digitalWrite(led_g, LOW);
  if (serial_connected == false)
  {
    if (flag == FLAG_HANDSHAKE_MASTER)
    {
      printDebug("FLAG_HANDSHAKE_MASTER");
      sendHandshakeSlavePacket(val1);
    }
    else if (flag == FLAG_HANDSHAKE_SLAVE_CONFIRMATION)
    {
      printDebug("FLAG_HANDSHAKE_SLAVE_CONFIRMATION");
      session_key = val1;
      serial_connected = true;
      time_of_still_alive_check = millis();
    }
  }
  else
  {
    if (flag == FLAG_STILL_ALIVE_CHECK_ANSWER)
    {
      printDebug("FLAG_STILL_ALIVE_CHECK_ANSWER");
      if (val1 == session_key && val2 == still_alive_check_key &&
          current_time - time_of_still_alive_check < STILL_ALIVE_CHECK_TIMEOUT_IN_MILLIS)
      {
        time_of_still_alive_check = millis();
        still_alive_check_key = 255;
        sent_still_alive_check_packet = false;
      }
    }
  }
}

void sendStillAliveCheckPacket()
{
  sendPacket(FLAG_STILL_ALIVE_CHECK_QUESTION, session_key, still_alive_check_key);
}

void sendHandshakeSlavePacket(byte temp_session_key)
{
  sendPacket(FLAG_HANDSHAKE_SLAVE, temp_session_key, 0);
}

void sendPacket(byte flag, byte value1, byte value2)
{
  digitalWrite(led_w, HIGH);
  delay(50);
  digitalWrite(led_w, LOW);
  Serial.write(PACKET_START);
  Serial.write(flag);
  Serial.write(value1);
  Serial.write(value2);
  Serial.write(PACKET_END);
  Serial.println();
}

// remove printDebug for release
void printDebug(string text)
{
  Serial.println(text);
}
