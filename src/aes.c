
#include "aes.h"

#define get_left_4bits(num) ({ \
    (num & 0xf0u) >> 4u; \
})

#define get_right_4bits(num) ({ \
    (num & 0x0fu); \
})

static uint8_t mul(uint8_t a, uint8_t b) {

    uint8_t p = 0;

    for (uint8_t i = 0; i < 8; i++) {
        if (b & 1u) {
            p ^= a;
        }

        uint8_t hbs = a & 0x80u;
        a <<= 1u;
        if (hbs) a ^= 0x1bu;
        b >>= 1u;
    }

    return p;
}

static void word_add(const uint8_t *a, const uint8_t *b, uint8_t *d) {

    d[0] = a[0] ^ b[0];
    d[1] = a[1] ^ b[1];
    d[2] = a[2] ^ b[2];
    d[3] = a[3] ^ b[3];
}

static void word_mul(uint8_t *a, uint8_t *b, uint8_t *d) {

    d[0] = mul(a[0], b[0]) ^ mul(a[3], b[1]) ^ mul(a[2], b[2]) ^ mul(a[1], b[3]);
    d[1] = mul(a[1], b[0]) ^ mul(a[0], b[1]) ^ mul(a[3], b[2]) ^ mul(a[2], b[3]);
    d[2] = mul(a[2], b[0]) ^ mul(a[1], b[1]) ^ mul(a[0], b[2]) ^ mul(a[3], b[3]);
    d[3] = mul(a[3], b[0]) ^ mul(a[2], b[1]) ^ mul(a[1], b[2]) ^ mul(a[0], b[3]);
}


static uint8_t s_box[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};


static uint8_t inv_s_box[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

static uint8_t R[] = {0x02, 0x00, 0x00, 0x00};

static uint8_t *Rcon(uint8_t i) {

    if (i == 1) {
        R[0] = 0x01;
    } else if (i > 1) {
        R[0] = 0x02;
        i--;
        while (i - 1 > 0) {
            R[0] = mul(R[0], 0x02);
            i--;
        }
    }

    return R;
}

static void add_round_key(uint8_t *state, const uint8_t *w, uint32_t packet_word, uint8_t r) {

    for (int i = 0; i < packet_word; i++) {
        state[packet_word * 0 + i] = state[packet_word * 0 + i] ^ w[4 * packet_word * r + 4 * i + 0];
        state[packet_word * 1 + i] = state[packet_word * 1 + i] ^ w[4 * packet_word * r + 4 * i + 1];
        state[packet_word * 2 + i] = state[packet_word * 2 + i] ^ w[4 * packet_word * r + 4 * i + 2];
        state[packet_word * 3 + i] = state[packet_word * 3 + i] ^ w[4 * packet_word * r + 4 * i + 3];
    }
}

static void _mix(uint8_t *state, uint8_t *a, int packet_word) {
    uint8_t col[4], res[4];
    for (int j = 0; j < packet_word; j++) {
        for (int i = 0; i < 4; i++) {
            col[i] = state[packet_word * i + j];
        }

        word_mul(a, col, res);

        for (int i = 0; i < 4; i++) {
            state[packet_word * i + j] = res[i];
        }
    }
}

static void mix_columns(uint8_t *state, int packet_word) {

    uint8_t a[] = {0x02, 0x01, 0x01, 0x03};
    _mix(state, a, packet_word);
}

static void inv_mix_columns(uint8_t *state, int packet_word) {

    uint8_t a[] = {0x0e, 0x09, 0x0d, 0x0b};
    _mix(state, a, packet_word);
}

static void shift_rows(uint8_t *state, int packet_word) {
    for (int i = 1; i < 4; i++) {
        int s = 0;
        while (s < i) {
            uint8_t tmp = state[packet_word * i + 0];

            for (int k = 1; k < packet_word; k++) {
                state[packet_word * i + k - 1] = state[packet_word * i + k];
            }

            state[packet_word * i + packet_word - 1] = tmp;
            s++;
        }
    }
}

static void inv_shift_rows(uint8_t *state, int packet_word) {
    for (int i = 1; i < 4; i++) {
        int s = 0;
        while (s < i) {
            uint8_t tmp = state[packet_word * i + packet_word - 1];

            for (int k = packet_word - 1; k > 0; k--) {
                state[packet_word * i + k] = state[packet_word * i + k - 1];
            }

            state[packet_word * i + 0] = tmp;
            s++;
        }
    }
}

static void sub_bytes(uint8_t *state, int packet_word) {

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < packet_word; j++) {
            uint8_t t = state[packet_word * i + j];
            state[packet_word * i + j] = s_box[16 * get_left_4bits(t) + get_right_4bits(t)];
        }
    }
}

static void inv_sub_bytes(uint8_t *state, int packet_word) {

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < packet_word; j++) {
            uint8_t t = state[packet_word * i + j];
            state[packet_word * i + j] = inv_s_box[16 * get_left_4bits(t) + get_right_4bits(t)];
        }
    }
}

static void sub_word(uint8_t *w) {

    for (int i = 0; i < 4; i++) {
        w[i] = s_box[16 * get_left_4bits(w[i]) + get_right_4bits(w[i])];
    }
}

static void rot_word(uint8_t *w) {
    uint8_t tmp = w[0];

    for (int i = 0; i < 3; i++) {
        w[i] = w[i + 1];
    }

    w[3] = tmp;
}

static void key_extend(aes_ctx *obj) {

    uint8_t tmp[4];
    uint8_t len = obj->packet_word * (obj->round + 1);

    for (int i = 0; i < obj->key_word; i++) {
        obj->w[4 * i + 0] = obj->key[4 * i + 0];
        obj->w[4 * i + 1] = obj->key[4 * i + 1];
        obj->w[4 * i + 2] = obj->key[4 * i + 2];
        obj->w[4 * i + 3] = obj->key[4 * i + 3];
    }

    for (int i = obj->key_word; i < len; i++) {
        tmp[0] = obj->w[4 * (i - 1) + 0];
        tmp[1] = obj->w[4 * (i - 1) + 1];
        tmp[2] = obj->w[4 * (i - 1) + 2];
        tmp[3] = obj->w[4 * (i - 1) + 3];

        if (i % obj->key_word == 0) {
            rot_word(tmp);
            sub_word(tmp);
            word_add(tmp, Rcon(i / obj->key_word), tmp);
        } else if (obj->key_word > 6 && i % obj->key_word == 4) {
            sub_word(tmp);
        }

        obj->w[4 * i + 0] = obj->w[4 * (i - obj->key_word) + 0] ^ tmp[0];
        obj->w[4 * i + 1] = obj->w[4 * (i - obj->key_word) + 1] ^ tmp[1];
        obj->w[4 * i + 2] = obj->w[4 * (i - obj->key_word) + 2] ^ tmp[2];
        obj->w[4 * i + 3] = obj->w[4 * (i - obj->key_word) + 3] ^ tmp[3];
    }
}

static void padding(aes_ctx *obj) {
    if (obj->padding == None)
        return;
    uint32_t need = obj->packet_word * 4 - (obj->text_size % (obj->packet_word * 4));
    uint32_t size = obj->text_size + need;

    uint32_t temp[obj->text_size];
    memcpy(temp, obj->in, obj->text_size);

    obj->in = realloc(obj->in, size);
    memset(obj->in, 0, size);
    memcpy(obj->in, temp, obj->text_size);

    switch (obj->padding) {
        default:
        case Zero:
            break;
        case Pkcs5:
        case Pkcs7:
            memset(obj->in + obj->text_size, need & 0xffu, need);
            break;
        case ISO10126:
        case ANSIX923:
            obj->in[size - 1] = need & 0xffu;
            break;
    }

    obj->text_size = size;
}

static void inv_padding(aes_ctx *obj) {
    uint32_t tail_len = 0;
    uint8_t tail_char = obj->out[obj->text_size - 1];

    if (tail_char == '\x00') {
        for (int i = obj->text_size - 1; i >= 0; --i) {
            if (obj->out[i] != '\x00') {
                tail_len = obj->text_size - 1 - i;
                break;
            }
        }
    } else {
        tail_len = (uint32_t) (tail_char & 0xffu);
    }

    if (tail_len >= obj->packet_word * 4)
        tail_len = obj->packet_word * 4;

    obj->text_size -= tail_len;
}

static void aes_ctx_init(aes_ctx *obj) {

    if (obj->key_size <= 16) {
        obj->key_word = 4;
        obj->round = 10;
    } else if (obj->key_size <= 24) {
        obj->key_word = 6;
        obj->round = 12;
    } else {
        obj->key_word = 8;
        obj->round = 14;
    }

    obj->w = malloc(obj->packet_word * (obj->round + 1) * 4);
    memset(obj->w, 0, obj->packet_word * (obj->round + 1) * 4);
    key_extend(obj);
    padding(obj);
}

static void _encode(const uint8_t *in, uint8_t *out, uint8_t *w, int packet_word, int round) {

    uint8_t state[4 * packet_word];
    uint8_t r, i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < packet_word; j++) {
            state[packet_word * i + j] = in[i + 4 * j];
        }
    }

    add_round_key(state, w, packet_word, 0);

    for (r = 1; r < round; r++) {
        sub_bytes(state, packet_word);
        shift_rows(state, packet_word);
        mix_columns(state, packet_word);
        add_round_key(state, w, packet_word, r);
    }

    sub_bytes(state, packet_word);
    shift_rows(state, packet_word);
    add_round_key(state, w, packet_word, round);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < packet_word; j++) {
            out[i + 4 * j] = state[packet_word * i + j];
        }
    }
}

static void _decode(const uint8_t *in, uint8_t *out, uint8_t *w, int packet_word, int round) {

    uint8_t state[4 * packet_word];
    uint8_t r, i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < packet_word; j++) {
            state[packet_word * i + j] = in[i + 4 * j];
        }
    }

    add_round_key(state, w, packet_word, round);

    for (r = round - 1; r >= 1; r--) {
        inv_shift_rows(state, packet_word);
        inv_sub_bytes(state, packet_word);
        add_round_key(state, w, packet_word, r);
        inv_mix_columns(state, packet_word);
    }

    inv_shift_rows(state, packet_word);
    inv_sub_bytes(state, packet_word);
    add_round_key(state, w, packet_word, 0);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < packet_word; j++) {
            out[i + 4 * j] = state[packet_word * i + j];
        }
    }
}

//todo: implement with multi-thread
static uint8_t *_ecb_encode(const uint8_t *in, uint32_t in_szie, uint8_t *w, int packet_word, int round) {

    uint8_t *out = malloc(in_szie);

    int packets = in_szie / (packet_word * 4);
    for (int i = 0; i < packets; ++i) {
        _encode(in + i * packet_word * 4, out + i * packet_word * 4, w, packet_word, round);
    }

    return out;
}

static uint8_t *_ecb_decode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round) {

    uint8_t *out = malloc(in_size);

    int packets = in_size / (packet_word * 4);
    for (int i = 0; i < packets; ++i) {
        _decode(in + i * packet_word * 4, out + i * packet_word * 4, w, packet_word, round);
    }

    return out;
}

static void xor(uint8_t *a, const uint8_t *b, uint32_t size) {
    for (int i = 0; i < size; ++i) {
        a[i] ^= b[i];
    }
}

static uint8_t *_cbc_encode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round, uint8_t *iv) {
    uint8_t *out = malloc(in_size);

    uint8_t buffer[packet_word * 4];
    if (iv)
        memcpy(buffer, iv, packet_word * 4);
    else
        memset(buffer, 0, packet_word * 4);

    int packets = in_size / (packet_word * 4);

    for (int i = 0; i < packets; ++i) {
        xor(buffer, in + i * packet_word * 4, packet_word * 4);
        _encode(buffer, out + i * packet_word * 4, w, packet_word, round);
        memcpy(buffer, out + i * packet_word * 4, packet_word * 4);
    }

    return out;
}

static uint8_t *_cbc_decode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round, uint8_t *iv) {
    uint8_t *out = malloc(in_size);

    uint8_t buffer[packet_word * 4];

    int packets = in_size / (packet_word * 4);

    for (int i = packets - 1; i > 0; --i) {
        _decode(in + i * packet_word * 4, buffer, w, packet_word, round);
        xor(buffer, in + (i - 1) * packet_word * 4, packet_word * 4);
        memcpy(out + i * packet_word * 4, buffer, packet_word * 4);
    }
    _decode(in, buffer, w, packet_word, round);
    if (iv)
        xor(buffer, iv, packet_word * 4);
    memcpy(out, buffer, packet_word * 4);

    return out;
}
//
//static uint8_t *_cfb_encode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round, uint8_t *iv) {
//
//}
//
//static uint8_t *_cfb_decode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round, uint8_t *iv) {
//
//}
//
//static uint8_t *_ofb_encode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round, uint8_t *iv) {
//
//}
//
//static uint8_t *_ofb_dncode(const uint8_t *in, uint32_t in_size, uint8_t *w, int packet_word, int round, uint8_t *iv) {
//
//}


uint8_t *aes_encode(uint8_t *in, uint32_t in_size,
                    uint8_t *key, uint32_t key_size,
                    uint32_t *out_size, uint8_t *iv,
                    padding_method padding, packet_method packet) {
    aes_ctx ctx = {
            .text_size = in_size,
            .key = key,
            .key_size = key_size,
            .padding = padding,
            .packet = packet,
            .packet_word = 4,
    };
    ctx.in = malloc(in_size);
    memcpy(ctx.in, in, in_size);

    aes_ctx_init(&ctx);

    //todo: cfb and ofb mode
    switch (packet) {
        default:
        case ECB:
            ctx.out = _ecb_encode(ctx.in, ctx.text_size, ctx.w, ctx.packet_word, ctx.round);
            break;
        case CBC:
            ctx.out = _cbc_encode(ctx.in, ctx.text_size, ctx.w, ctx.packet_word, ctx.round, iv);
            break;
        case CFB:
            break;
        case OFB:
            break;
    }

    if (out_size)
        *out_size = ctx.text_size;

    free(ctx.in);
    free(ctx.w);

    return ctx.out;
}


uint8_t *aes_decode(uint8_t *in, uint32_t in_size,
                    uint8_t *key, uint32_t key_size,
                    uint32_t *out_size, uint8_t *iv,
                    packet_method packet) {
    aes_ctx ctx = {
            .text_size = in_size,
            .key = key,
            .key_size = key_size,
            .packet = packet,
            .packet_word = 4,
            .padding = None,
    };
    ctx.in = malloc(in_size);
    memcpy(ctx.in, in, in_size);

    aes_ctx_init(&ctx);

    //todo: cfb and ofb mode
    switch (packet) {
        default:
        case ECB:
            ctx.out = _ecb_decode(ctx.in, ctx.text_size, ctx.w, ctx.packet_word, ctx.round);
            break;
        case CBC:
            ctx.out = _cbc_decode(ctx.in, ctx.text_size, ctx.w, ctx.packet_word, ctx.round, iv);
            break;
        case CFB:
            break;
        case OFB:
            break;
    }

    inv_padding(&ctx);

    if (out_size)
        *out_size = ctx.text_size;

    free(ctx.in);
    free(ctx.w);

    return ctx.out;
}