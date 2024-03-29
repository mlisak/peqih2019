#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "encrypt.h"

inline static int is_error(int rc, EVP_CIPHER_CTX* ctx)
{
  if(rc == 0) EVP_CIPHER_CTX_free(ctx);
  return rc == 0 ? 1 : 0;
}

size_t fill_with_random(unsigned char* buf, size_t len)
{
  int dev_random_fd = open("/dev/urandom", O_RDONLY);
  if(dev_random_fd < 0) return -1;

  size_t result = read(dev_random_fd, buf, len);
  close(dev_random_fd);
  
  return result;
}

unsigned char* get_random_bytes(size_t len)
{
  unsigned char* buf = (unsigned char*) malloc(len);
  if(buf == NULL) return NULL;

  size_t result = fill_with_random(buf, len);
  if(result < 0)
  {
    free(buf);
    return NULL;
  }

  return buf;
}

int encrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            const unsigned char* ad, const int ad_len,
            unsigned char* out, unsigned char* aead_tag)
{
  int out_len = 0;
  int ret = 0;

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if(ctx == NULL) return -ENC_CTX_CREAT_FAIL;

  int rc = EVP_EncryptInit(ctx, EVP_aes_256_gcm(), NULL, NULL);
  if(is_error(rc, ctx)) return -ENC_INIT_FAIL;

  rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL);
  if(is_error(rc, ctx)) return -ENC_INIT_FAIL;

  rc = EVP_EncryptInit(ctx, NULL, key, iv);
  if(is_error(rc, ctx)) return -ENC_INIT_FAIL;

  if(ad != NULL && ad_len != 0)
  {
    rc = EVP_EncryptUpdate(ctx, NULL, &out_len, ad, ad_len);
    if(is_error(rc, ctx)) return -ENC_ENCRYPT_FAIL;
    ret += out_len;
  }

  rc = EVP_EncryptUpdate(ctx, out, &out_len, in, in_len);
  if(is_error(rc, ctx)) return -ENC_ENCRYPT_FAIL;
  ret += out_len;

  rc = EVP_EncryptFinal(ctx, out, &out_len);
  if(is_error(rc, ctx)) return -ENC_FINAL_FAIL;
  ret+= out_len;

  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, AEAD_TAG_LEN, aead_tag);
  EVP_CIPHER_CTX_free(ctx);
  return ret;
}

int decrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            const unsigned char* ad, const int ad_len,
            unsigned char* out, unsigned char* aead_tag)
{
  int out_len = 0;
  int ret = 0;

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if(ctx == NULL) return -ENC_CTX_CREAT_FAIL;

  int rc = EVP_DecryptInit(ctx, EVP_aes_256_gcm(), NULL, NULL);
  if(is_error(rc, ctx)) return -ENC_INIT_FAIL;

  rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL);
  if(is_error(rc, ctx)) return -ENC_INIT_FAIL;

  rc = EVP_DecryptInit(ctx, NULL, key, iv);
  if(is_error(rc, ctx)) return -ENC_INIT_FAIL;

  if(ad != NULL && ad_len != 0)
  {
    rc = EVP_DecryptUpdate(ctx, NULL, &out_len, ad, ad_len);
    if(is_error(rc, ctx)) return -ENC_ENCRYPT_FAIL;
    ret += out_len;
  }

  rc = EVP_DecryptUpdate(ctx, out, &out_len, in, in_len);
  if(is_error(rc, ctx)) return -ENC_ENCRYPT_FAIL;
  ret += out_len;

  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, AEAD_TAG_LEN, aead_tag);
  rc = EVP_DecryptFinal(ctx, out, &out_len);
  if(is_error(rc, ctx)) return -ENC_FINAL_FAIL;
  ret += out_len;

  EVP_CIPHER_CTX_free(ctx);
  return ret;
}

