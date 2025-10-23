#include "crypto_utils.h"
#include <mbedtls/aes.h>
#include <mbedtls/md.h>

static const unsigned char AES_KEY[16] = {
  'M','y','S','e','c','r','e','t','A','E','S','K','e','y','1','2'
};
static const unsigned char HMAC_KEY[32] = "SuperSecureHmacKeyForAuth!!";

String bytesToHex(const unsigned char *buf, size_t len) {
  String s = "";
  char tmp[3];
  for (size_t i = 0; i < len; i++) {
    sprintf(tmp, "%02x", buf[i]);
    s += tmp;
  }
  return s;
}

void hexToBytes(const String &hex, unsigned char *out, size_t len) {
  for (size_t i = 0; i < len; i++) {
    out[i] = strtol(hex.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
  }
}

String encryptAndSign(const String &plaintext) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  unsigned char iv[16] = {0};

  int len = plaintext.length();
  int padded = (len + 15) / 16 * 16;
  unsigned char input[padded];
  unsigned char output[padded];
  memset(input, 0, padded);
  memcpy(input, plaintext.c_str(), len);

  mbedtls_aes_setkey_enc(&aes, AES_KEY, 128);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded, iv, input, output);
  mbedtls_aes_free(&aes);

  unsigned char hmac[32];
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, info, 1);
  mbedtls_md_hmac_starts(&ctx, HMAC_KEY, sizeof(HMAC_KEY));
  mbedtls_md_hmac_update(&ctx, output, padded);
  mbedtls_md_hmac_finish(&ctx, hmac);
  mbedtls_md_free(&ctx);

  return bytesToHex(output, padded) + "|" + bytesToHex(hmac, 32);
}

String decryptAndVerify(const String &ciphertext) {
  int sep = ciphertext.indexOf('|');
  if (sep == -1) return "";

  String cipherHex = ciphertext.substring(0, sep);
  String hmacHex   = ciphertext.substring(sep + 1);

  int len = cipherHex.length() / 2;
  unsigned char cipher[len];
  hexToBytes(cipherHex, cipher, len);

  unsigned char hmacRecv[32];
  hexToBytes(hmacHex, hmacRecv, 32);

  unsigned char hmacCalc[32];
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, info, 1);
  mbedtls_md_hmac_starts(&ctx, HMAC_KEY, sizeof(HMAC_KEY));
  mbedtls_md_hmac_update(&ctx, cipher, len);
  mbedtls_md_hmac_finish(&ctx, hmacCalc);
  mbedtls_md_free(&ctx);

  if (memcmp(hmacCalc, hmacRecv, 32) != 0) {
    Serial.println("[SECURITY] ⚠️ HMAC mismatch!");
    return "";
  }

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  unsigned char iv[16] = {0};
  unsigned char output[len];
  mbedtls_aes_setkey_dec(&aes, AES_KEY, 128);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, len, iv, cipher, output);
  mbedtls_aes_free(&aes);

  return String((char *)output);
}
