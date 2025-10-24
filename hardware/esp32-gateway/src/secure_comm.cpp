// #include "secure_comm.h"
// #include <mbedtls/aes.h>
// #include <mbedtls/md.h>
// #include <ctype.h>

// // AES + HMAC Keys
// static const unsigned char AES_KEY[16] = {
//   'M','y','S','e','c','r','e','t','A','E','S','K','e','y','1','2'
// };
// static const unsigned char AES_IV[16] = {
//   0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE,
//   0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
// };
// static const unsigned char HMAC_KEY[32] = "SuperSecureHmacKeyForAuth!!";

// String bytesToHex(const uint8_t *buf, size_t len) {
//   String out;
//   const char hex[] = "0123456789abcdef";
//   for (size_t i = 0; i < len; i++) {
//     out += hex[buf[i] >> 4];
//     out += hex[buf[i] & 0x0F];
//   }
//   return out;
// }

// int hexToBytes(const String &hexStr, uint8_t *out) {
//   int len = hexStr.length() / 2;
//   for (int i = 0; i < len; i++) {
//     char c1 = hexStr[2 * i];
//     char c2 = hexStr[2 * i + 1];
//     uint8_t val = (isdigit(c1) ? c1 - '0' : tolower(c1) - 'a' + 10) << 4;
//     val |= (isdigit(c2) ? c2 - '0' : tolower(c2) - 'a' + 10);
//     out[i] = val;
//   }
//   return len;
// }

// String aesDecrypt(const uint8_t *cipher, size_t len) {
//   uint8_t output[len + 1];
//   memset(output, 0, sizeof(output));

//   mbedtls_aes_context aes;
//   mbedtls_aes_init(&aes);
//   uint8_t iv[16];
//   memcpy(iv, AES_IV, 16);
//   mbedtls_aes_setkey_dec(&aes, AES_KEY, 128);
//   mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, len, iv, cipher, output);
//   mbedtls_aes_free(&aes);

//   return String((char *)output);
// }

// String computeHMAC(const String &message) {
//   unsigned char result[32];
//   mbedtls_md_context_t ctx;
//   const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

//   mbedtls_md_init(&ctx);
//   mbedtls_md_setup(&ctx, info, 1);
//   mbedtls_md_hmac_starts(&ctx, HMAC_KEY, strlen((const char *)HMAC_KEY));
//   mbedtls_md_hmac_update(&ctx, (const unsigned char *)message.c_str(), message.length());
//   mbedtls_md_hmac_finish(&ctx, result);
//   mbedtls_md_free(&ctx);

//   return bytesToHex(result, 32);
// }

#include "secure_comm.h"
#include <mbedtls/aes.h>
#include <mbedtls/md.h>

static const unsigned char AES_KEY[16] = {
  'M','y','S','e','c','r','e','t','A','E','S','K','e','y','1','2'
};
static const unsigned char HMAC_KEY[32] = "SuperSecureHmacKeyForAuth!!";

// -------------------- Utility: Hex <-> Bytes --------------------
void hexToBytes(const String &hex, unsigned char *out, size_t len) {
  for (size_t i = 0; i < len; i++) {
    out[i] = strtol(hex.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
  }
}

String bytesToHex(const unsigned char *buf, size_t len) {
  String s = "";
  char tmp[3];
  for (size_t i = 0; i < len; i++) {
    sprintf(tmp, "%02x", buf[i]);
    s += tmp;
  }
  return s;
}

// -------------------- Decrypt + Verify --------------------
String decryptAndVerify(const String &packet) {
  int sep = packet.indexOf('|');
  if (sep == -1) {
    Serial.println("[RX] ⚠️ Invalid packet format (no separator).");
    return "";
  }

  String cipherHex = packet.substring(0, sep);
  String hmacHex   = packet.substring(sep + 1);

  int cipherLen = cipherHex.length() / 2;
  unsigned char cipher[cipherLen];
  hexToBytes(cipherHex, cipher, cipherLen);

  unsigned char recvHmac[32];
  hexToBytes(hmacHex, recvHmac, 32);

  // --- Hitung ulang HMAC dari ciphertext ---
  unsigned char calcHmac[32];
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, info, 1);
  mbedtls_md_hmac_starts(&ctx, HMAC_KEY, sizeof(HMAC_KEY));
  mbedtls_md_hmac_update(&ctx, cipher, cipherLen);
  mbedtls_md_hmac_finish(&ctx, calcHmac);
  mbedtls_md_free(&ctx);

  if (memcmp(calcHmac, recvHmac, 32) != 0) {
    Serial.println("[RX] ❌ HMAC verification failed!");
    Serial.print("[RX] Expected: "); Serial.println(bytesToHex(calcHmac, 32));
    Serial.print("[RX] Received: "); Serial.println(hmacHex);
    return "";
  }

  // --- HMAC valid → lanjut dekripsi AES-CBC ---
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  unsigned char iv[16] = {0};
  unsigned char output[cipherLen];

  mbedtls_aes_setkey_dec(&aes, AES_KEY, 128);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, cipherLen, iv, cipher, output);
  mbedtls_aes_free(&aes);

  output[cipherLen - 1] = '\0'; // pastikan null terminated
  String plaintext = String((char *)output);
  return plaintext;
}
