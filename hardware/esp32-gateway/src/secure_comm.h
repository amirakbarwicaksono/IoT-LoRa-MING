#ifndef SECURE_COMM_H
#define SECURE_COMM_H

#include <Arduino.h>

String aesDecrypt(const uint8_t *cipher, size_t len);
String computeHMAC(const String &message);
int hexToBytes(const String &hexStr, uint8_t *out);
String bytesToHex(const uint8_t *buf, size_t len);

#endif
