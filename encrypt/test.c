#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encrypt.h"

// Allocates memory, REMEMBER TO free() AFTER USING
unsigned char* get_random_bytes(size_t len)
{
  int dev_random_fd = open("/dev/urandom", O_RDONLY);
  if(dev_random_fd < 0) return NULL;

  unsigned char* buf = (unsigned char*) malloc(len);
  if(buf == NULL) return NULL;

  size_t result = read(dev_random_fd, buf, len);
  close(dev_random_fd);
  if(result < 0)
  {
    free(buf);
    return NULL;
  }

  return buf;
}

int main(int argc, char* argv[])
{
  unsigned char* input;
  unsigned char* output;
  unsigned char* output2;
  unsigned char* key;
  unsigned char* iv;
  unsigned char* aead;

  input = get_random_bytes(1024);
  key = get_random_bytes(256/8);
  iv = get_random_bytes(96/8);

  output = (unsigned char*) malloc(1024);
  output2 = (unsigned char*) malloc(1024);
  aead = (unsigned char*) malloc(16);

  printf("Encrypting...\n");
  int rc = encrypt(input, 1024, key, iv, output, aead);
  if(rc < 0) return rc;
  printf("Encrypted bytes: %d\n", rc);
  
  printf("Decrypting...\n");
  rc = decrypt(output, 1024, key, iv, output2, aead);
  if(rc < 0) return rc;

  printf("Decrypted bytes: %d. Veryfing... ", rc);
  rc = memcmp(input, output2, 1024);

  if(rc == 0) printf("Success!\n");
  else printf("Something went wrong ;_;\n");

  free(input);
  free(output);
  free(output2);
  free(key);
  free(iv);
  free(aead);

  return 0;
}

