//
// Created by void0red on 5/10/19.
//

#ifndef AES_GUI_AES_H
#define AES_GUI_AES_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum padding_method {
    None, Zero, Pkcs5, Pkcs7, ISO10126, ANSIX923
} padding_method;

typedef enum packet_method {
    ECB, CBC, CFB, OFB
} packet_method;

typedef struct _aes_ctx {
    uint8_t *in;
    uint8_t *out;
    uint32_t text_size;

    uint8_t *key;
    uint32_t key_size;

    padding_method padding;

    packet_method packet;

    uint8_t *w;

    int packet_word;
    int key_word;
    int round;
} aes_ctx;

uint8_t *aes_encode(uint8_t *in, uint32_t in_size,
                    uint8_t *key, uint32_t key_size,
                    uint32_t *out_size, uint8_t *iv,
                    padding_method padding, packet_method packet);

uint8_t *aes_decode(uint8_t *in, uint32_t in_size,
                    uint8_t *key, uint32_t key_size,
                    uint32_t *out_size, uint8_t *iv,
                    packet_method packet);

#endif //AES_GUI_AES_H
