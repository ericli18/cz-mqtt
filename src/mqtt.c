#include "mqtt.h"
#include "mqtt_packet_utils.h"
#include <bits/types/struct_iovec.h>
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

static size_t unpack_mqtt_connect(const unsigned char *buf,
                                  union mqtt_header *hdr,
                                  union mqtt_packet *pkt) {
  struct mqtt_connect connect = {.header = *hdr};
  pkt->connect = connect;

  const unsigned char *init = buf;
  const unsigned char *packet_end;

  /* Decode remaining length */
  size_t remaining_length;
  int status = mqtt_decode_length(&buf, &remaining_length);
  if (status == -1) {
    fprintf(stderr, "Error decoding remaining length\n");
    return 0;
  }

  packet_end = buf + remaining_length;

  /* Skip protocol name and version (8 bytes) */
  buf += 7;

  /* Read variable header byte flags */
  pkt->connect.byte = mqtt_unpack_u8((const uint8_t **)&buf);

  /* Read keepalive */
  pkt->connect.payload.keepalive = mqtt_unpack_u16((const uint8_t **)&buf);

  uint16_t cid_len;
  cid_len = mqtt_unpack_u16(&buf);
  if (cid_len > 0) {
    pkt->connect.payload.client_id = malloc(cid_len + 1);
    mqtt_unpack_bytes((const uint8_t **)&buf, cid_len,
                      pkt->connect.payload.client_id);
    if (pkt->connect.payload.client_id == NULL) {
      return 0;
    }
  } else {
    // TODO: create client id
  }

  if (pkt->connect.bits.will == 1) {
    pkt->connect.payload.will_topic = mqtt_unpack_string(&buf);
    pkt->connect.payload.will_message = mqtt_unpack_string(&buf);
  }
  /* Read the username if username flag is set */
  if (pkt->connect.bits.username == 1)
    pkt->connect.payload.username = mqtt_unpack_string(&buf);
  /* Read the password if password flag is set */
  if (pkt->connect.bits.password == 1)
    pkt->connect.payload.password = mqtt_unpack_string(&buf);

  /* Check if we've read exactly the right number of bytes */
  if (buf != packet_end) {
    fprintf(stderr, "Packet length mismatch\n");
    return 0;
  }

  return buf - init;
}

static size_t unpack_mqtt_publish(const unsigned char *buf,
                                  union mqtt_header *hdr,
                                  union mqtt_packet *pkt) {
  struct mqtt_publish publish = {.header = *hdr};
  pkt->publish = publish;
  /*
   * Second byte of the fixed header, contains the length of remaining bytes
   * of the connect packet
   */
  int status;
  size_t len;
  status = mqtt_decode_length(&buf, &len);
  if (status == -1) {
    // handle error
  }

  /* Read topic length and topic of the soon-to-be-published message */
  pkt->publish.topiclen = mqtt_unpack_u16(&buf);
  pkt->publish.topic = malloc(pkt->publish.topiclen + 1);
  mqtt_unpack_bytes((const uint8_t **)&buf, pkt->publish.topiclen,
                    pkt->publish.topic);

  uint16_t message_len = len;
  /* Read packet id */
  if (publish.header.bits.qos > AT_MOST_ONCE) {
    pkt->publish.pkt_id = mqtt_unpack_u16((const uint8_t **)&buf);
    message_len -= sizeof(uint16_t);
  }
  /*
   * Message len is calculated subtracting the length of the variable header
   * from the Remaining Length field that is in the Fixed Header
   */
  message_len -= (sizeof(uint16_t) + pkt->publish.topiclen);
  pkt->publish.payloadlen = message_len;
  pkt->publish.payload = malloc(message_len + 1);
  mqtt_unpack_bytes((const uint8_t **)&buf, message_len, pkt->publish.payload);
  return len;
}