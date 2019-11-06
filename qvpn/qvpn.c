/*
 * Author: K. Potamianos <karolos.potamianos@gmail.com>
 */

#include "qvpn.h"

#include "encrypt/encrypt.h"

static qvpn_state_t qvpn_state;

int qvpn_rekey(qvpn_state_t *qvpn_state) {
   /* Call Cenk's code here */
   size_t i;

    qvpn_state.iv = 0x0;
    for(i = 0 ; i < sizeof(qvpn_state.key) ; i++)
      qvpn_state.key[i] = i;
}

int qvpn_init() {
    /* Call init crypto here */
    qvpn_rekey(&qvpn_state);
 }


/*
 * This function transfers a payload over the QVPN tunnel.
 * It does not handle the encryption.
 */

ssize_t qvpn_sendpacket(int socket, const void* qvpnbuf, size_t len) {
    /* Todo: handle TCP */
    return sendto(socket, qvpnbuf, len);
}

/*
 * This function receives a payload from the QVPN tunnel.
 * It does not handle the encryption.
 */

ssize_t qvpn_recvpacket(int socket, const void* qvpnbuf, size_t len) {
    /* Todo: handle select here */
    return recv(socket, qvpnbuf, len);
}

/*
 * This function transfers a user payload over the QVPN tunnel.
 * It also handles the encryption.
 */

ssize_t qvpn_send(int socket, const uint8_t* buf, size_t len) {
    qvpn_header_t* header;
    int err;

    if( len + sizeof(qvpn_header_t) >= QVPN_BUF_SIZE )
      die("Cannot send packet larger than 65535, including header.");

    header = tx_buf;

    err = encrypt(buf, len, qvpn_state.key, qvpn_state.iv, NULL, 0, tx_buf+sizeof(qvpn_header_t), header->aead);
    /* Todo: handle error cases */

    err = qvpn_sendpacket(socket, tx_buf, sizeof(qvpn_header_t)+len);
    /* Todo: handle error cases */

    return err;
}

/*
 * This function transfers a user payload over the QVPN tunnel.
 * It also handles the encryption.
 */

ssize_t qvpn_recv(int socket, const uint8_t* buf, size_t* len) {
    qvpn_header_t* header;
    int err;

    err = qvpn_recvpacket(socket, rx_buf, len);
    /* Todo: handle error cases */
    header = (qvpn_header_t*) rx_buf;

    err = decrypt(rx_buf+sizeof(qvpn_header_t), len, qvpn_sate.key, qvpn_state.iv, NULL, 0, buf, header->aead);
    /* Todo: handle error cases */

    return err;
}