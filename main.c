#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char b64table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "0123456789+/";

static char b64revtb[256] = {
    -3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*0-15*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*16-31*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /*32-47*/
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, /*48-63*/
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, /*64-79*/
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /*80-95*/
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /*96-111*/
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /*112-127*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*128-143*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*144-159*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*160-175*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*176-191*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*192-207*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*208-223*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*224-239*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 /*240-255*/
    };

// prototypes
unsigned char *spc_base64_encode(const unsigned char *input, size_t len, int wrap);
static unsigned int raw_base64_decode(unsigned char *input, unsigned char *output,
                                        int strict, int *err);
unsigned char *spc_base64_decode (unsigned char *buf, size_t *len, int strict, int *err);

int main(void) {

   char* input = "Hello world";
    printf(spc_base64_encode(input, strlen(input), 0));

    // TODO: set up decoder
    return 0;
}


// accept binary buffer and return a base64 encoded, null terminated string
unsigned char *spc_base64_encode(const unsigned char *input, const size_t len, const int wrap) {

    unsigned char *output, *p;
    size_t i = 0, mod = len % 3, toalloc;

    toalloc = (len / 3) * 4 + (3 - mod) % 3 + 1;
    if (wrap) {
        toalloc += len / 57;
        if (len % 57) toalloc++;
    }

    p = output = (unsigned char *) malloc(((len / 3) + (mod ? 1 : 0)) * 4 + 1);
    if (!p) return 0;

    while (i < len - mod) {
        *p++ = b64table[input[i++] >> 2];
        *p++ = b64table[((input[i - 1] << 4) | (input[i] >> 4)) & 0x3F];
        *p++ = b64table[((input[i] << 2) | (input[i + 1] >> 6)) & 0x3F];
        *p++ = b64table[input[i] & 0x3F];
        i += 2;
        if (wrap && !(i % 57)) *p++ = '\n';
    }
    if (!mod) {
        if (wrap && i % 57) *p++ = '\n';
        *p = 0;
        return output;
    } else {
        *p++ = b64table[input[i++] >> 2];
        *p++ = b64table[((input[i - 1] << 4) | (input[i] >> 4)) & 0x3F];
        if (mod == 1) {
            *p++ = '=';
            *p++ = '=';
            if (wrap) *p++ = '\n';
            *p = 0;
            return output;
        } else {
            *p++ = b64table[(input[i] << 2) & 0x3C];
            *p++ = '=';
            if (wrap) *p++ = '\n';
            *p = 0;
            return output;
        }
    }
}

// if string isn't properly padded or terminates prematurely return an error
static unsigned int raw_base64_decode(unsigned char *input, unsigned char *output,
                                        int strict, int *err) {
    unsigned int result = 0, x;
    unsigned char buf[3], *p = input, pad = 0;
    *err = 0;

    while (!pad) {
        switch ((x = b64revtb[*p++])) {
            case -3: // null terminator
                if (((p - 1) - input) % 4 < 2) {
                    *err = 1;
                    return result;
                }
            case -2: // padding character, invalid
                if (((p - 1) - input) % 4 < 2) {
                    *err = 1;
                    return result;
                } else if (((p - 1) - input) % 4 == 2) {
                    // check appropriate padding
                    if (*p != '=') {
                        *err = 1;
                        return result;
                    }
                    buf[2] = 0;
                    pad = 2;
                    result++;
                    break;
                } else {
                    pad = 1;
                    result += 2;
                    break;
                }
                return result;
            case -1:
                if (strict) {
                    *err = 2;
                    return result;
                }
                break;
            default:
                switch (((p - 1) - input) % 4) {
                    case 0:
                        buf[0] = x << 2;
                        break;
                    case 1:
                        buf[0] |= (x >> 4);
                        buf[1] = x << 4;
                        break;
                    case 2:
                        buf[1] |= (x >> 2);
                        buf[2] |= (x << 6);
                        break;
                    case 3:
                        buf[2] |= x;
                        result += 3;
                        for (x = 0; x < 3 - pad; x++) *output++ = buf[x];
                        break;
                }
                break;
        }
    }
    for (x = 0; x < 3 - pad; x++) *output++ = buf[x];
    return result;
}

unsigned char *spc_base64_decode(unsigned char *buf, size_t *len, int strict, int *err) {

    unsigned char *outbuf = (unsigned char *) malloc(3 * (strlen(buf) / 4 + 1));
    if (!outbuf) {
        *err = -3;
        *len = 0;
        return 0;
    }
    *len = raw_base64_decode(buf, outbuf, strict, err);
    if (*err) {
        free(outbuf);
        *len = 0;
        outbuf = 0;
    }
    return outbuf;
}