/*
 * Author: K. Potamianos <karolos.potamianos@gmail.com>
 */

#include <cstdint>


/*
 * QVPN packet header
 * - aead: authenticated encryption with associated data tag
 * 
 * For future use:
 * - msg_pos: position in payload
 * - key_pos: position in key stream
 */

struct qvpn_header_t {
  uint8_t aead[16];
  /*
  union {
    uint8_t assoc_data[4];
    struct {
      uint16_t msg_pos;
      uint16_t key_pos;
    };
  }
  */ 
};

/* Todo: fine proper header definition for max IP size */
const unsigned int QVPN_BUF_SIZE = 1<<16;

struct qvpn_state_t {
  uint8_t iv[12];
  uint8_t key[32];
  uint8_t tx_buf[QVPN_BUF_SIZE];
  uint8_t rx_buf[QVPN_BUF_SIZE];

};


int qvpn_rekey(qvpn_state_t *qvpn_state);
int qvpn_init();

ssize_t qvpn_sendpacket(int socket, const void *buf, size_t len);
ssize_t qvpn_recvpacket(int socket, const void *buf, size_t* len);

ssize_t qvpn_send(int socket, const void *buf, size_t len);
ssize_t qvpn_recv(int socket, size_t* len);
