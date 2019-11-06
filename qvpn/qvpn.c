/*
 * Author: K. Potamianos <karolos.potamianos@gmail.com>
 */

#include "qvpn.h"

#include <err.h>
#include <string.h>
#include <sys/socket.h>

//#define TESTME
#ifdef TESTME
int encrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            const unsigned char* ad, const int ad_len,
            unsigned char* out, unsigned char* aead_tag);

int decrypt(const unsigned char* in, const int in_len,
            const unsigned char* key, const unsigned char* iv,
            const unsigned char* ad, const int ad_len,
            unsigned char* out, unsigned char* aead_tag);
#else
#include "encrypt/encrypt.h"
#endif

static qvpn_state_t qvpn_state;

int qvpn_rekey(uint8_t key[32]) {
    memcpy(qvpn_state.key, key, 32);
    /* Using IV of 0 as we get plenty of keys */
    memset(qvpn_state.iv, 0, 12);
    /* get_random_bytes(qvpn_state.iv, 12); */
}

int qvpn_init(uint8_t key[32]) {
    /* Call init crypto here */
    qvpn_rekey(key);
 }


/*
 * This function transfers a payload over the QVPN tunnel.
 * It does not handle the encryption.
 */

ssize_t qvpn_sendpacket(int socket, const qvpn_buf_t* qvpnbuf, size_t len) {
    /* Todo: handle TCP */
    return sendto(socket, (void*) qvpnbuf->buffer, len, 0, NULL, 0);
}

/*
 * This function receives a payload from the QVPN tunnel.
 * It does not handle the encryption.
 */

ssize_t qvpn_recvpacket(int socket, qvpn_buf_t* qvpn_buf) { /* Todo: use qvpn_buf for multithread */
    int n;
    uint8_t* rxPtr;

    if( qvpn_state.rx_buf.mode == 0 ) { 
        /* Header reading mode */
        rxPtr = (uint8_t*) &qvpn_state.rx_buf.header + sizeof(qvpn_header_t) - qvpn_state.rx_buf.to_get;
    } else {
        rxPtr = (uint8_t*) &qvpn_state.rx_buf.buffer + sizeof(qvpn_header_t) + (qvpn_state.rx_buf.header.msg_len - qvpn_state.rx_buf.to_get);
    }

    n = recv(socket, rxPtr, qvpn_state.rx_buf.to_get, 0);

    if( n == 0 ) return 0;
    else if( n == qvpn_state.rx_buf.to_get && qvpn_state.rx_buf.mode == 0) {
        qvpn_state.rx_buf.to_get = qvpn_state.rx_buf.header.msg_len;
        qvpn_state.rx_buf.mode = 1;
        return qvpn_recvpacket(socket, qvpn_buf);
    } else if(n < qvpn_state.rx_buf.to_get) {
        qvpn_state.rx_buf.to_get =- n;
        return 0;
    } else if( n == qvpn_state.rx_buf.to_get && qvpn_state.rx_buf.mode == 1) {
        qvpn_state.rx_buf.mode = 0;
        return qvpn_state.rx_buf.header.msg_len;
    }

    return 0;
}

/*
 * This function transfers a user payload over the QVPN tunnel.
 * It also handles the encryption.
 */

ssize_t qvpn_send(int socket, const uint8_t *buf, size_t len) {
    qvpn_header_t* header;
    int err;

    /*
    if( len + sizeof(qvpn_header_t) >= QVPN_BUF_SIZE )
      die("Cannot send packet larger than 65535, including header.");
    */

    header = &qvpn_state.tx_buf.header;
    header->msg_len = len;

    err = encrypt(buf, len, 
                qvpn_state.key, qvpn_state.iv, 
                header->assoc_data, sizeof(header->assoc_data), 
                qvpn_state.tx_buf.buffer+sizeof(qvpn_header_t), header->aead);
    /* Todo: handle error cases */

    err = qvpn_sendpacket(socket, &qvpn_state.tx_buf, sizeof(qvpn_header_t)+len);
    /* Todo: handle error cases */

    return err;
}

/*
 * This function transfers a user payload over the QVPN tunnel.
 * It also handles the encryption.
 */

ssize_t qvpn_recv(int socket, uint8_t* buf, size_t* len) {
    qvpn_header_t* header;
    int err;

    err = qvpn_recvpacket(socket, &qvpn_state.rx_buf);
    if( err <= 0 ) {
        return err;
    }

    *len = err;

    /* Todo: handle error cases */
    header = (qvpn_header_t*) &qvpn_state.rx_buf.header;

    err = decrypt(qvpn_state.rx_buf.buffer+sizeof(qvpn_header_t), *len, 
                qvpn_state.key, qvpn_state.iv, 
                header->assoc_data, sizeof(header->assoc_data),
                buf, header->aead);

    /* Todo: handle error cases */

    return *len;
}