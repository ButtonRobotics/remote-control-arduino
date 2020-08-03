// FLAGS
const byte FLAG_HANDSHAKE_MASTER = 1;
const byte FLAG_HANDSHAKE_SLAVE = 2;
const byte FLAG_HANDSHAKE_SLAVE_CONFIRMATION = 3;
const byte FLAG_STILL_ALIVE_CHECK_QUESTION = 4;
const byte FLAG_STILL_ALIVE_CHECK_ANSWER = 5;
const byte FLAG_RESTART = 6;
const byte FLAG_ACTION_MOVE = 7;
const byte FLAG_ACTION_STOP = 8;
const byte FLAG_HANDBREAK = 9;

// TIMING
const unsigned int STILL_ALIVE_CHECK_TIMEOUT_IN_MILLIS = 1000;

// OTHER CONSTANTS
const unsigned int BAUD_RATE = 9600;

// PACKET COMPOSITION
const byte PACKET_START = 145;
const byte PACKET_END = 137;

// SESSION VARIABLES
bool serial_connected;
byte session_key;
unsigned long time_of_still_alive_check;
unsigned long current_time;

bool blink_w, blink_g, blink_r, blink_y;


// PIN SETTINGS
byte led_w = 4;
byte led_g = 5;
byte led_r = 6;
byte led_y = 7;
void setup() {
  current_time = millis();
  Serial.begin(BAUD_RATE);
  reset();
  ledSetup();
}

void loop() {
  // Serial.flush();
  current_time = millis();
  checkIncomingCommunication();
  if (serial_connected && !stillAliveCheck()) {
    reset();
  }

  if (!serial_connected) { // TODO#1: send every x millis
    sendHandshakeMasterPacket();
  }

  digitalWrite(led_r, serial_connected);
  // delay(5); // instead of delay, do TODO#1
}

void reset() {
  printDebug("RESET");
  serial_connected = false;
  randomSeed(analogRead(0));
  session_key = (byte)random(255);
  time_of_still_alive_check = ULONG_MAX;

  // led
  blink_w = false;
  blink_g = false;
  blink_r = false;
  blink_y = false;
}

bool stillAliveCheck() {
  return !(current_time - time_of_still_alive_check > STILL_ALIVE_CHECK_TIMEOUT_IN_MILLIS);
}

void checkIncomingCommunication() {
  byte packetParts[5];
  unsigned int num_of_incoming_bytes = Serial.available();
  for (byte i = 0; i < num_of_incoming_bytes && Serial.available() >= 5; i++) {
    if (Serial.peek() != PACKET_START) {
      Serial.read();
      continue;
    }
    for (byte j = 0; j < 5 && ((j == 4 && Serial.peek() == PACKET_END) || Serial.peek() != PACKET_END); j++) {
      packetParts[j] = Serial.read();
      if (j == 4) {
        parseSerialPacket(packetParts[0], packetParts[1],
                          packetParts[2], packetParts[3], packetParts[4]);
      }
    }
  }
}

void ledSetup() {
  pinMode(led_w, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_y, OUTPUT);
  digitalWrite(led_w, LOW);
  digitalWrite(led_g, LOW);
  digitalWrite(led_r, LOW);
  digitalWrite(led_y, LOW);

  blink_w = true;
  blink_g = true;
  blink_r = true;
  blink_y = true;
}

void blink() {
}

void parseSerialPacket(byte start, byte flag, byte val1, byte val2, byte end_pckt) {
    if (flag == FLAG_STILL_ALIVE_CHECK_QUESTION && serial_connected) {
      printDebug("FLAG_STILL_ALIVE_CHECK_QUESTION");
      sendStillAliveCheckAnswerPacket(val1, val2);
      time_of_still_alive_check = millis();
    } else if (flag == FLAG_HANDSHAKE_SLAVE && val1 == session_key && serial_connected) {
      printDebug("FLAG_HANDSHAKE_SLAVE");
      sendHandshakeSlaveConfirmationPacket();
      serial_connected = true;
      time_of_still_alive_check = millis();
    }
}

void sendStillAliveCheckAnswerPacket(byte val1, byte val2) {
  sendPacket(FLAG_STILL_ALIVE_CHECK_ANSWER, val1, val2);
}

void sendHandshakeSlaveConfirmationPacket() {
  sendPacket(FLAG_HANDSHAKE_SLAVE_CONFIRMATION, session_key, 0);
}

void sendHandshakeMasterPacket() {
  sendPacket(FLAG_HANDSHAKE_MASTER, session_key, 0);
}

void sendPacket(byte flag, byte value1, byte value2) {
  digitalWrite(led_w, LOW);
  Serial.write(PACKET_START);
  Serial.write(flag);
  Serial.write(value1);
  Serial.write(value2);
  Serial.write(PACKET_END);
  Serial.println();
}

// remove printDebug for release
void printDebug(string text) {
  Serial.println(text);
}
