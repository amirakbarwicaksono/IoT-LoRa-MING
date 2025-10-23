#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <Arduino.h>

String encryptAndSign(const String &plaintext);
String decryptAndVerify(const String &ciphertext);

#endif
