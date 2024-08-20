#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <Arduino.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>

// 函数声明
String encryptFixedKeyWithPublicKey(unsigned char *hexArray, size_t &hexLength);
String aesEncrypt(const char *message,uint16_t mess_len, unsigned char *output, size_t &outputLength);
void printHex(const unsigned char *data, size_t length);
void base64ToHexArray(const String &base64, unsigned char *hexArray, size_t &hexLength);
String base64Encode(const unsigned char *input, size_t length);
String aesDecrypt(const unsigned char *ciphertext, size_t ciphertextLength, unsigned char *hexArray, size_t &hexLength) ;
#endif // ENCRYPTION_H
