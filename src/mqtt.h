/*
 * MQTT Fixed Header Structure
 *
 * +--------+---+---+---+---+---+---+---+---+
 * | Bit    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * +--------+---+---+---+---+---+---+---+---+
 * | Byte 1 |    MQTT type  |    Flags     |
 * +--------+---+---+---+---+---+---+---+---+
 * | Byte 2 |                               |
 * |   .    |      Remaining Length         |
 * |   .    |                               |
 * | Byte 5 |                               |
 * +--------+-------------------------------+
 *
 * MQTT type: Defines the MQTT control packet type
 * Flags: Specific flags for each MQTT packet type
 *        These are not mandatory, just the bit 7-4 control block
 * Remaining Length: Variable length field (1-4 bytes)
 *                   Indicates the number of bytes remaining
 *                   in the packet, excluding the fixed header
 */

#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>

/*
 * MQTT Control Packet Types (First byte of fixed header), useful for generic
 * replies
 *
 * Each value represents the first byte of the fixed header for different
 * MQTT control packets. The upper 4 bits denote the packet type, while
 * the lower 4 bits are used for flags specific to each packet type.
 *
 * +--------+-----------------------------------+-------------+
 * | Hex    | Control Packet Type               | Bit Pattern |
 * +--------+-----------------------------------+-------------+
 * | 0x20   | CONNACK (Connection Acknowledgment)| 0010 0000   |
 * | 0x30   | PUBLISH                           | 0011 0000   |
 * | 0x40   | PUBACK (Publish Acknowledgment)   | 0100 0000   |
 * | 0x50   | PUBREC (Publish Received)         | 0101 0000   |
 * | 0x60   | PUBREL (Publish Release)          | 0110 0000   |
 * | 0x70   | PUBCOMP (Publish Complete)        | 0111 0000   |
 * | 0x90   | SUBACK (Subscribe Acknowledgment) | 1001 0000   |
 * | 0xB0   | UNSUBACK (Unsubscribe Acknowledgment) | 1011 0000   |
 * | 0xD0   | PINGRESP (Ping Response)          | 1101 0000   |
 * +--------+-----------------------------------+-------------+
 */

#define CONNACK_BYTE 0x20
#define PUBLISH_BYTE 0x30
#define PUBACK_BYTE 0x40
#define PUBREC_BYTE 0x50
#define PUBREL_BYTE 0x60
#define PUBCOMP_BYTE 0x70
#define SUBACK_BYTE 0x90
#define UNSUBACK_BYTE 0xB0
#define PINGRESP_BYTE 0xD0

// Reference: 2.2.1 MQTT Control Packet type
enum packet_type {
  CONNECT = 1,
  CONNACK = 2,
  PUBLISH = 3,
  PUBACK = 4,
  PUBREC = 5,
  PUBREL = 6,
  PUBCOMP = 7,
  SUBSCRIBE = 8,
  SUBACK = 9,
  UNSUBSCRIBE = 10,
  UNSUBACK = 11,
  PINGREQ = 12,
  PINGRESP = 13,
  DISCONNECT = 14
};

// Reference: 3.3.1.2 QoS
// TODO: Set bits myself? will see in the future
enum qos_level { AT_MOST_ONCE, AT_LEAST_ONCE, EXACTLY_ONCE };

// ordered in little-endian
union mqtt_header {
  unsigned char byte;
  struct {
    unsigned int retain : 1;
    unsigned int qos : 2;
    unsigned int dup : 1;
    unsigned int type : 4;
  } bits;
};

// Reference: 3.1.2 Variable header, beginning for connect
struct mqtt_connect {
  union mqtt_header header;
  union {
    unsigned char byte;
    struct {
      unsigned int reserved : 1;
      unsigned int clean_session : 1;
      unsigned int will : 1;
      unsigned int will_qos : 2;
      unsigned int will_retain : 1;
      unsigned int password : 1;
      unsigned int username : 1;
    } bits;
  };
  struct {
    unsigned short keepalive;
    unsigned char *client_id;
    unsigned char *username;
    unsigned char *password;
    unsigned char *will_topic;
    unsigned char *will_message;
  } payload;
};

struct mqtt_connack {
  union mqtt_header header;
  union {
    unsigned char byte;
    struct {
      unsigned int session_present : 1;
      unsigned int reserved : 7;
    } bits;
  };
  unsigned char rc;
};
