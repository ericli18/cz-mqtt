#include "mqtt.h"
#include <msgpack.h>
#include <msgpack/unpack.h>
#include <stdint.h>
#include <stdio.h>

/*
 * MQTT v3.1.1 standard, Remaining length field on the fixed header can be at
 * most 4 bytes.
 */
static const int MAX_LEN_BYTES = 4;

// Reference: 3.1.1
/*
 * Encode the remaining length field. This is the Variable Header
 * and the paylod, without the fixed header and the bytes for storing
 * length
 *
 */
int mqtt_encode_length(unsigned char *buf, size_t len) {
  int bytes = 0;
  do {
    if (bytes + 1 > MAX_LEN_BYTES)
      return bytes;
    short d = len % 128;
    len /= 128;
    /* if there are more data to encode, set the top bit of this byte to let
     * decoding algorithm know */
    if (len > 0)
      d |= 128;
    buf[bytes++] = d;
  } while (len > 0);
  return bytes;
}

// Reference: 3.1.1
/*
 * Decode the remaining length field. This is the Variable Header
 * and the paylod, without the fixed header and the bytes for storing
 * length
 *
 */
int mqtt_decode_length(const unsigned char **buf, unsigned long *length) {
  unsigned int multiplier = 1;
  unsigned int value = 0;
  unsigned char encodedByte;

  do {
    // want to check if we are over the max 4 byte limit for encoded bytes
    if (multiplier > 128 * 128 * 128) {
      return -1;
    }

    encodedByte = **buf;
    // Mask off most significant bit
    value += (encodedByte & 127) * multiplier;
    multiplier *= 128;
    (*buf)++;

  } while ((encodedByte & 128) != 0);

  *length = value;
  return 0;
}
