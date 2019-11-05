#pragma once

#include <openssl/evp.h>

#define ENC_CTX_CREAT_FAIL 1
#define ENC_INIT_FAIL 2
#define ENC_ENCRYPT_FAIL 3
#define ENC_FINAL_FAIL 4
#define END_AEAD_VERIFY_FAIL 5

#define IV_LEN 12
#define AEAD_TAG_LEN 16

/* Wrappers for openssl's AES-256-GCM
 *
 * For both functions:
 *
 * in - input buffer
 * in_len - length of input buffer
 * key - buffer containing key bytes (32 bytes)
 * iv - buffer containing iv bytes (12 bytes)
 * out - output buffer
 * ad - associated data buffer
 * ad_len - associated data length
 * aead_tag - output buffer for AEAD tag (16 bytes)
 *
 * If not using any associated data, set ad to NULL and
 * ad_len to 0.
 *
 * All the buffers should be managed (allocated and freed)
 * by the caller.
 *
 * The functions return number of succesfully processed bytes,
 * or negative number indicating an error (see defines above).
 */

int encrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            const unsigned char* ad, const int ad_len,
            unsigned char* out, unsigned char* aead_tag);

int decrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            const unsigned char* ad, const int ad_len,
            unsigned char* out, unsigned char* aead_tag);

