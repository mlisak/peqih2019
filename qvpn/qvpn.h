/*
 * Author: K. Potamianos <karolos.potamianos@gmail.com>
 */

#include <stdint.h>
#include <sys/types.h>

/*
 * QVPN packet header
 * - aead: authenticated encryption with associated data tag
 * 
 * For future use:
 * - msg_pos: position in payload
 * - key_pos: position in key stream
 */

typedef struct {
  uint8_t aead[16];
  union {
    uint8_t assoc_data[2];
    struct {
      uint16_t msg_len;
    };
  };
} qvpn_header_t;

/* Todo: fine proper header definition for max IP size */
#define QVPN_BUF_SIZE (1<<16)


typedef struct {
  union {
    uint8_t buffer[QVPN_BUF_SIZE];
    qvpn_header_t header;
  };
  uint16_t to_get;
  int mode; /* 0 for header, 1 for payload */
} qvpn_buf_t;

typedef struct {
  uint8_t iv[12];
  uint8_t key[32];
  qvpn_buf_t tx_buf;
  qvpn_buf_t rx_buf;
} qvpn_state_t;


int qvpn_rekey(uint8_t key[32]);
int qvpn_init(uint8_t key[32]);

ssize_t qvpn_sendpacket(int socket, const qvpn_buf_t* qvpn_buf, size_t len);
ssize_t qvpn_recvpacket(int socket, qvpn_buf_t* qvpn_buf);

ssize_t qvpn_send(int socket, const uint8_t* qvpn_buf, size_t len);
ssize_t qvpn_recv(int socket, uint8_t* qvpn_buf, size_t* len);
