#include <Arduino.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>

// 确保公钥格式正确
const char PUBLIC_KEY[] =
"MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAKaTP4eBWYBh3JDnYa7h2nuYACREgmV1o250/36ebYwaUswQDbUdMoeRvRIWxhCtXEzVkMYtH07ctmpzMo8uTvMCAwEAAQ==";

// 固定的随机密钥
const unsigned char FIXED_RANDOM_KEY[] = "qwertyuiopasdfgh";

// 函数声明
String encryptFixedKeyWithPublicKey(unsigned char *hexArray, size_t &hexLength);
String aesEncrypt(const char *message, const unsigned char *key,unsigned char *hexArray, size_t &hexLength);
void printHex(const unsigned char *data, size_t length);
void base64ToHexArray(const String &base64, unsigned char *hexArray, size_t &hexLength);
String base64Encode(const unsigned char *input, size_t length);
 // 将Base64编码转换为16进制并存储到数组
unsigned char hexArray[256];
size_t hexLength = 0;


String encryptFixedKeyWithPublicKey(unsigned char *hexArray, size_t &hexLength) {
    mbedtls_pk_context pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_encrypt";
    int ret = 1; // 默认返回值为非零，表示错误
    size_t olen = 0;
    unsigned char output[64]; // 512 bits = 64 bytes for the output buffer
    String encoded = "";

    // 初始化结构体
    mbedtls_pk_init(&pk);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // 初始化随机数生成器
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        Serial.print("Failed to initialize RNG, error code: ");
        Serial.println(ret);
        // 清理
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return "";
    }

    // 添加 PEM 标记以解析公钥
    String pem_key = "-----BEGIN PUBLIC KEY-----\n";
    pem_key += PUBLIC_KEY;
    pem_key += "\n-----END PUBLIC KEY-----\n";

    // 解析公钥
    ret = mbedtls_pk_parse_public_key(&pk, (const unsigned char *)pem_key.c_str(), pem_key.length() + 1);
    if (ret != 0) {
        Serial.print("Failed to parse public key, error code: ");
        Serial.println(ret);
        // 清理
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return "";
    }

    // 用公钥加密固定随机密钥
    ret = mbedtls_pk_encrypt(&pk, FIXED_RANDOM_KEY, sizeof(FIXED_RANDOM_KEY)-1, output, &olen, sizeof(output), mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        Serial.print("Failed to encrypt fixed random key, error code: ");
        Serial.println(ret);
        // 清理
        mbedtls_pk_free(&pk);
        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        return "";
    }

    // 打印加密后的16进制表示
    //Serial.println("Encrypted Fixed Random Key (HEX):");
    //printHex(output, olen);

    // Base64 编码
    encoded = base64Encode(output, olen);
    base64ToHexArray(encoded, hexArray, hexLength);
    //printHex(hexArray, hexLength);

    // 清理
    mbedtls_pk_free(&pk);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    return encoded;
}

size_t addPKCS7Padding(const unsigned char *input, size_t inputLength, unsigned char *output, size_t blockSize) {
    size_t paddingLength = blockSize - (inputLength % blockSize);
    size_t paddedLength = inputLength + paddingLength;
    memcpy(output, input, inputLength);
    memset(output + inputLength, paddingLength, paddingLength); // 填充数据
    return paddedLength;
}

String aesEncrypt(const char *message, uint16_t mess_len, unsigned char *output, size_t &outputLength) {
    mbedtls_aes_context aes;
    unsigned char iv[16] = {0}; // Initialization vector (can be random or fixed)
    memcpy(iv, FIXED_RANDOM_KEY, 16); // 示例使用固定 IV，实际应使用随机 IV

    // 初始化 AES 上下文
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char *)FIXED_RANDOM_KEY, 128);

    // 计算填充后的长度
    size_t paddedLength = ((mess_len + 15) / 16) * 16;
    unsigned char *paddedMessage = (unsigned char *)malloc(paddedLength);
    if (paddedMessage == NULL) {
        Serial.println("Memory allocation failed");
        mbedtls_aes_free(&aes);
        return "";
    }

    // 添加 PKCS7 填充
    size_t actualPaddedLength = addPKCS7Padding((const unsigned char *)message, mess_len, paddedMessage, 16);

    // 执行加密
    int ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, actualPaddedLength, iv, paddedMessage, output);
    free(paddedMessage); // 释放内存
    mbedtls_aes_free(&aes);

    if (ret != 0) {
        Serial.print("Failed to AES encrypt, error code: ");
        Serial.println(ret);
        return "";
    }

    outputLength = actualPaddedLength;

    // 打印AES加密后的16进制表示
    //Serial.println("AES Encrypted Message (HEX):");
    //printHex(output, outputLength);

    return "";
}
#if 0
String aesEncrypt(const char *message,uint16_t mess_len, unsigned char *output, size_t &outputLength) {
    mbedtls_aes_context aes;
    unsigned char iv[16] = {0}; // Initialization vector (can be random or fixed)
    memcpy(iv, FIXED_RANDOM_KEY, 16);
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, FIXED_RANDOM_KEY, 128);

    // 计算需要的填充长度
    
    size_t paddedLength = ((mess_len + 15) / 16) * 16; // 计算填充后的长度
    outputLength = paddedLength;

    // 执行加密
    int ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLength, iv, (const unsigned char *)message, output);

    mbedtls_aes_free(&aes);

    if (ret != 0) {
        Serial.print("Failed to AES encrypt, error code: ");
        Serial.println(ret);
        return "";
    }

    // 打印AES加密后的16进制表示
    Serial.println("AES Encrypted Message (HEX):");
     printHex(output, outputLength);

    return "";
}
#endif

String aesDecrypt(const unsigned char *ciphertext, size_t ciphertextLength, unsigned char *hexArray, size_t &hexLength) {
    mbedtls_aes_context aes;
    unsigned char iv[16] = {0};
    memcpy(iv, "qwertyuiopasdfgh", 16);

    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, FIXED_RANDOM_KEY, 128);

    size_t maxOutputLength = ciphertextLength;
    unsigned char *output = (unsigned char*)malloc(maxOutputLength);

    int ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertextLength, iv, ciphertext, output);

    mbedtls_aes_free(&aes);

    if (ret != 0) {
        Serial.print("Failed to AES decrypt, error code: ");
        Serial.println(ret);
        free(output);
        return "";
    }

    size_t paddingLength = output[ciphertextLength - 1];
    hexLength = ciphertextLength - paddingLength;

    memcpy(hexArray, output, hexLength);

    free(output);

    //Serial.println("AES Decrypted Message (HEX):");
    //printHex(hexArray, hexLength);

    return "";
}


void base64ToHexArray(const String &base64, unsigned char *hexArray, size_t &hexLength) 
 {
    hexLength = 0;
    for (size_t i = 0; i < base64.length(); ++i) {
        char c = base64[i];
        unsigned char value = (unsigned char)c;
        // 添加到数组
        hexArray[hexLength++] = value;
    }
}

String base64Encode(const unsigned char *input, size_t length) {
    unsigned char output[512];
    size_t outputLen;

    int ret = mbedtls_base64_encode(output, sizeof(output), &outputLen, input, length);
    if (ret != 0) {
        Serial.print("Failed to encode Base64, error code: ");
        Serial.println(ret);
        return "";
    }

    return String((char *)output);
}


void printHex(const unsigned char *data, size_t length) {
    
    for (size_t i = 0; i < length; ++i) {
        if (data[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(data[i], HEX);
        if ((i + 1) % 16 == 0) {
            Serial.println();
        } else {
            Serial.print(" ");
        }
    }
    Serial.println("");
}
