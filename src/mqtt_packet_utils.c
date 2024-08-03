#include "mqtt_packet_utils.h"
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t mqtt_unpack_u8(const uint8_t **buf) {
  uint8_t val = **buf;
  (*buf)++;
  return val;
}

//this is for unpacking strings, strings are preceded by the most significant bit
uint16_t mqtt_unpack_u16(const uint8_t **buf) {
  uint16_t val;
  memcpy(&val, *buf, sizeof(uint16_t));
  *buf += sizeof(uint16_t);
  return ntohs(val);
}

uint32_t mqtt_unpack_u32(const uint8_t **buf) {
  uint32_t val;
  memcpy(&val, *buf, sizeof(uint32_t));
  *buf += sizeof(uint32_t);
  return ntohl(val);
}


unsigned char *mqtt_unpack_string(const uint8_t **buf, uint32_t *length) {
  *length = mqtt_unpack_u16(buf);
  unsigned char *str = malloc(*length + 1);
  if (str == NULL) {
    return NULL; // Memory allocation failed
  }
  memcpy(str, *buf, *length);
  str[*length] = '\0';
  *buf += *length;
  return str;
}

void mqtt_pack_u8(uint8_t **buf, uint8_t val) {
  **buf = val;
  (*buf)++;
}

void mqtt_pack_u16(uint8_t **buf, uint16_t val) {
  uint16_t netval = htons(val);
  memcpy(*buf, &netval, sizeof(uint16_t));
  *buf += sizeof(uint16_t);
}

void mqtt_pack_u32(uint8_t **buf, uint32_t val) {
  uint32_t netval = htonl(val);
  memcpy(*buf, &netval, sizeof(uint32_t));
  *buf += sizeof(uint32_t);
}

void mqtt_pack_string(uint8_t **buf, const char *str, uint16_t length) {
  mqtt_pack_u16(buf, length);
  memcpy(*buf, str, length);
  *buf += length;
}

int mqtt_validate_utf8(const char *str, int len) {
  int i = 0;
  while (i < len) {
    if ((str[i] & 0x80) == 0) {
      // ASCII character
      i++;
    } else if ((str[i] & 0xE0) == 0xC0) {
      // 2-byte UTF-8 character
      if (i + 1 >= len || (str[i + 1] & 0xC0) != 0x80)
        return 0;
      i += 2;
    } else if ((str[i] & 0xF0) == 0xE0) {
      // 3-byte UTF-8 character
      if (i + 2 >= len || (str[i + 1] & 0xC0) != 0x80 ||
          (str[i + 2] & 0xC0) != 0x80)
        return 0;
      i += 3;
    } else if ((str[i] & 0xF8) == 0xF0) {
      // 4-byte UTF-8 character
      if (i + 3 >= len || (str[i + 1] & 0xC0) != 0x80 ||
          (str[i + 2] & 0xC0) != 0x80 || (str[i + 3] & 0xC0) != 0x80)
        return 0;
      i += 4;
    } else {
      return 0; // Invalid UTF-8 encoding
    }
  }
  return 1; // Valid UTF-8 string
}
