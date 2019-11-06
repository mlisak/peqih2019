#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encrypt.h"

int main(int argc, char* argv[])
{
  unsigned char* input;
  unsigned char* output;
  unsigned char* output2;
  unsigned char* key;
  unsigned char* iv;
  unsigned char* aead;

  unsigned char* ad;

  input = get_random_bytes(1024);
  key = get_random_bytes(256/8);
  iv = get_random_bytes(96/8);

  ad = get_random_bytes(69);

  output = (unsigned char*) malloc(1024);
  output2 = (unsigned char*) malloc(1024);
  aead = (unsigned char*) malloc(16);

  printf("Encrypting...\n");
  int rc = encrypt(input, 1024, key, iv, ad, 69, output, aead);
  if(rc < 0) return rc;
  printf("Encrypted bytes: %d\n", rc);

  printf("Decrypting...\n");
  rc = decrypt(output, 1024, key, iv, ad, 69, output2, aead);
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
  free(ad);

  return 0;
}

