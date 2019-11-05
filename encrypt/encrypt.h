#pragma once

#include <openssl/evp.h>

#define ENC_CTX_CREAT_FAIL 1
#define ENC_INIT_FAIL 2
#define ENC_ENCRYPT_FAIL 3
#define ENC_FINAL_FAIL 4
#define END_AEAD_VERIFY_FAIL 5

#define IV_LEN 12
#define AEAD_TAG_LEN 16

int encrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            unsigned char* out, unsigned char* aead);

int decrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            unsigned char* out, unsigned char* aead); 

