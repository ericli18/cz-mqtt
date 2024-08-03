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

  /* Read Properties (MQTT v5.0) */
  size_t properties_length;
  status = mqtt_decode_length(&buf, &properties_length);
  // TODO: Handle error

  // if (properties_length > 0) {
  //   const unsigned char *properties_end = buf + properties_length;
  //   while (buf < properties_end) {
  //     struct mqtt_property prop;
  //     prop.type =
  //         (enum mqtt_property_type)mqtt_unpack_u8((const uint8_t **)&buf);

  //     switch (prop.type) {
  //     case PROP_SESSION_EXPIRY_INTERVAL:
  //     case PROP_MESSAGE_EXPIRY_INTERVAL:
  //     case PROP_WILL_DELAY_INTERVAL:
  //     case PROP_MAXIMUM_PACKET_SIZE:
  //       prop.value.dword = mqtt_unpack_u32((const uint8_t **)&buf);
  //       break;
  //     case PROP_AUTHENTICATION_METHOD:
  //     case PROP_AUTHENTICATION_DATA:
  //     case PROP_CONTENT_TYPE:
  //     case PROP_RESPONSE_TOPIC:
  //     case PROP_ASSIGNED_CLIENT_IDENTIFIER:
  //     case PROP_SERVER_REFERENCE:
  //     case PROP_REASON_STRING:
  //     case PROP_RESPONSE_INFORMATION:
  //       // String properties
  //       mqtt_unpack_variable_int(&buf, (uint32_t *)&prop.value.string.len);
  //       prop.value.string.data = (char *)malloc(prop.value.string.len + 1);
  //       memcpy(prop.value.string.data, buf, prop.value.string.len);
  //       prop.value.string.data[prop.value.string.len] = '\0';
  //       buf += prop.value.string.len;
  //       break;
  //     case PROP_SERVER_KEEP_ALIVE:
  //     case PROP_RECEIVE_MAXIMUM:
  //     case PROP_TOPIC_ALIAS_MAXIMUM:
  //     case PROP_TOPIC_ALIAS:
  //       prop.value.word = mqtt_unpack_u16((const uint8_t **)&buf);
  //       break;
  //     case PROP_PAYLOAD_FORMAT_INDICATOR:
  //     case PROP_REQUEST_PROBLEM_INFORMATION:
  //     case PROP_REQUEST_RESPONSE_INFORMATION:
  //     case PROP_MAXIMUM_QOS:
  //     case PROP_RETAIN_AVAILABLE:
  //     case PROP_WILDCARD_SUBSCRIPTION_AVAILABLE:
  //     case PROP_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
  //     case PROP_SHARED_SUBSCRIPTION_AVAILABLE:
  //       prop.value.byte = mqtt_unpack_u8((const uint8_t **)&buf);
  //       break;
  //     case PROP_USER_PROPERTY:
  //       mqtt_unpack_variable_int(&buf,
  //                                (uint32_t
  //                                *)&prop.value.user_property.key_len);
  //       prop.value.user_property.key =
  //           (char *)malloc(prop.value.user_property.key_len + 1);
  //       memcpy(prop.value.user_property.key, buf,
  //              prop.value.user_property.key_len);
  //       prop.value.user_property.key[prop.value.user_property.key_len] =
  //       '\0'; buf += prop.value.user_property.key_len;

  //       mqtt_unpack_variable_int(
  //           &buf, (uint32_t *)&prop.value.user_property.value_len);
  //       prop.value.user_property.value =
  //           (char *)malloc(prop.value.user_property.value_len + 1);
  //       memcpy(prop.value.user_property.value, buf,
  //              prop.value.user_property.value_len);
  //       prop.value.user_property.value[prop.value.user_property.value_len] =
  //           '\0';
  //       buf += prop.value.user_property.value_len;
  //       break;
  //     case PROP_SUBSCRIPTION_IDENTIFIER:
  //     case PROP_CORRELATION_DATA:
  //       // These properties have variable-length fields
  //       mqtt_unpack_variable_int(&buf, (uint32_t *)&prop.value.string.len);
  //       prop.value.string.data = (char *)malloc(prop.value.string.len);
  //       memcpy(prop.value.string.data, buf, prop.value.string.len);
  //       buf += prop.value.string.len;
  //       break;
  //     default:
  //       fprintf(stderr, "Unknown property ID: %d\n", prop.type);
  //       return 0;
  //     }

  //     if (mqtt_property_add(&pkt->connect.properties, &prop) != 0) {
  //       fprintf(stderr, "Failed to add property\n");
  //       return 0;
  //     }
  //   }

  //   if (buf != properties_end) {
  //     fprintf(stderr, "Property length mismatch\n");
  //     return 0;
  //   }
  // }

  buf += properties_length;
  /* Read Client ID */
  uint16_t cid_len;
  cid_len = mqtt_unpack_u16(&buf);
  if (cid_len > 0) {
    pkt->connect.payload.client_id =
        mqtt_unpack_string((const uint8_t **)&buf, &cid_len);
    if (pkt->connect.payload.client_id == NULL) {
      return 0;
    }
  } else {
    // TODO: create client id
  }

  if (pkt->connect.bits.will == 1) {
    size_t will_properties_len;
    mqtt_decode_length(&buf, &will_properties_len);
    buf+= will_properties_len;

    // TODO: Handle properties later too.... and the error

    uint16_t will_topic_len;
    will_topic_len = mqtt_unpack_u16(&buf);
    
    pkt->connect.payload.will_topic = mqtt_unpack_string(&buf, &will_topic_len);
    //TODO: WTF is the will payload

  }

  /* Read Username if username flag is set */
  if (pkt->connect.bits.username == 1) {
    uint32_t username_len;
    mqtt_unpack_variable_int(&buf, &username_len);
    pkt->connect.payload.username =
        mqtt_unpack_string((const uint8_t **)&buf, &username_len);
    if (pkt->connect.payload.username == NULL) {
      return 0;
    }
  }

  /* Read Password if password flag is set */
  if (pkt->connect.bits.password == 1) {
    uint32_t password_len;
    mqtt_unpack_variable_int(&buf, &password_len);
    pkt->connect.payload.password =
        mqtt_unpack_string((const uint8_t **)&buf, &password_len);
    if (pkt->connect.payload.password == NULL) {
      return 0;
    }
  }

  /* Check if we've read exactly the right number of bytes */
  if (buf != packet_end) {
    fprintf(stderr, "Packet length mismatch\n");
    return 0;
  }

  return buf - init;
}