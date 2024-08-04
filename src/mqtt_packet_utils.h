#ifndef MQTT_PACKET_UTILS_H
#define MQTT_PACKET_UTILS_H

#include <stdint.h>
#include <stdio.h>

// Unpacking functions
uint8_t mqtt_unpack_u8(const uint8_t **buf);
uint16_t mqtt_unpack_u16(const uint8_t **buf);
uint32_t mqtt_unpack_u32(const uint8_t **buf);
uint8_t *mqtt_unpack_bytes(const uint8_t **buf, size_t length, uint8_t *str);
unsigned char *mqtt_unpack_string(const uint8_t **buf);

// Packing functions
void mqtt_pack_u8(uint8_t **buf, uint8_t val);
void mqtt_pack_u16(uint8_t **buf, uint16_t val);
void mqtt_pack_u32(uint8_t **buf, uint32_t val);
void mqtt_pack_string(uint8_t **buf, const char *str, uint16_t len);

// Utility functions
int mqtt_validate_utf8(const char *str, int len);

#endif // MQTT_PACKET_UTILS_H