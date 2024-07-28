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

#include <stdint.h>
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

/*
 * Represents the first byte of the MQTT Fixed Header.
 *
 * This union allows access to the individual fields of the first byte
 * of the MQTT fixed header, as well as the entire byte itself.
 *
 * +--------+---+---+---+---+---+---+---+---+
 * | Bit    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * +--------+---+---+---+---+---+---+---+---+
 * | Byte 1 |    Type   | D | Q | Q | R |
 * +--------+-----------+---+---+---+---+
 *
 * @field byte  The entire first byte of the fixed header.
 * @field bits  A bitfield struct to access individual fields:
 *              - retain: The MQTT RETAIN flag. If set, the server should hold
 * on to the message.
 *              - qos: Quality of Service level for the message (0, 1, or 2).
 *              - dup: The DUP flag. If set, indicates a duplicate message.
 *              - type: The MQTT Control Packet type (values 1-14).
 */
union mqtt_header {
  unsigned char byte;
  struct {
    unsigned int retain : 1;
    unsigned int qos : 2;
    unsigned int dup : 1;
    unsigned int type : 4;
  } bits;
};

// MQTT v5.0 Property Identifiers
enum mqtt_property_type {
  PROP_PAYLOAD_FORMAT_INDICATOR = 1,
  PROP_MESSAGE_EXPIRY_INTERVAL = 2,
  PROP_CONTENT_TYPE = 3,
  PROP_RESPONSE_TOPIC = 8,
  PROP_CORRELATION_DATA = 9,
  PROP_SUBSCRIPTION_IDENTIFIER = 11,
  PROP_SESSION_EXPIRY_INTERVAL = 17,
  PROP_ASSIGNED_CLIENT_IDENTIFIER = 18,
  PROP_SERVER_KEEP_ALIVE = 19,
  PROP_AUTHENTICATION_METHOD = 21,
  PROP_AUTHENTICATION_DATA = 22,
  PROP_REQUEST_PROBLEM_INFORMATION = 23,
  PROP_WILL_DELAY_INTERVAL = 24,
  PROP_REQUEST_RESPONSE_INFORMATION = 25,
  PROP_RESPONSE_INFORMATION = 26,
  PROP_SERVER_REFERENCE = 28,
  PROP_REASON_STRING = 31,
  PROP_RECEIVE_MAXIMUM = 33,
  PROP_TOPIC_ALIAS_MAXIMUM = 34,
  PROP_TOPIC_ALIAS = 35,
  PROP_MAXIMUM_QOS = 36,
  PROP_RETAIN_AVAILABLE = 37,
  PROP_USER_PROPERTY = 38,
  PROP_MAXIMUM_PACKET_SIZE = 39,
  PROP_WILDCARD_SUBSCRIPTION_AVAILABLE = 40,
  PROP_SUBSCRIPTION_IDENTIFIER_AVAILABLE = 41,
  PROP_SHARED_SUBSCRIPTION_AVAILABLE = 42
};

// Structure to hold a single property
struct mqtt_property {
  enum mqtt_property_type type;
  union {
    uint8_t byte;
    uint16_t word;
    uint32_t dword;
    struct {
      uint16_t len;
      char *data;
    } string;
    struct {
      uint16_t key_len;
      char *key;
      uint16_t value_len;
      char *value;
    } user_property;
  } value;
};

// Structure to hold a list of properties
struct mqtt_properties {
  uint32_t length;            // Total length of all properties
  uint32_t count;             // Number of properties
  struct mqtt_property *list; // Dynamic array of properties
};

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
  struct mqtt_properties properties;
  struct mqtt_properties will_properties;
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
  struct mqtt_properties properties;
};

struct mqtt_publish {
  union mqtt_header header;
  unsigned short pkt_id;
  unsigned short topiclen;
  unsigned char *topic;
  unsigned short payloadlen;
  unsigned char *payload;
  struct mqtt_properties properties;
};

struct mqtt_subscribe {
  union mqtt_header header;
  unsigned short pkt_id;
  unsigned short tuples_len;
  struct {
    unsigned short topic_len;
    unsigned char *topic;
    unsigned qos;
  } *tuples;
  struct mqtt_properties properties;
};

struct mqtt_unsubscribe {
  union mqtt_header header;
  unsigned short pkt_id;
  unsigned short tuples_len;
  struct {
    unsigned short topic_len;
    unsigned char *topic;
  } *tuples;
  struct mqtt_properties properties;
};

struct mqtt_suback {
  union mqtt_header header;
  unsigned short pkt_id;
  unsigned short rcslen;
  unsigned char *rcs;
  struct mqtt_properties properties;
};

struct mqtt_ack {
  union mqtt_header header;
  unsigned short pkt_id;
  struct mqtt_properties properties;
};

typedef struct mqtt_ack mqtt_puback;
typedef struct mqtt_ack mqtt_pubrec;
typedef struct mqtt_ack mqtt_pubrel;
typedef struct mqtt_ack mqtt_pubcomp;
typedef struct mqtt_ack mqtt_unsuback;

struct mqtt_disconnect {
  union mqtt_header header;
  struct mqtt_properties properties;
};

typedef union mqtt_header mqtt_pingreq;
typedef union mqtt_header mqtt_pingresp;

union mqtt_packet {
  struct mqtt_ack ack;
  union mqtt_header header;
  struct mqtt_connect connect;
  struct mqtt_connack connack;
  struct mqtt_suback suback;
  struct mqtt_publish publish;
  struct mqtt_subscribe subscribe;
  struct mqtt_unsubscribe unsubscribe;
  struct mqtt_disconnect disconnect;
};

// Function prototypes
int mqtt_encode_length(unsigned char *, size_t);
int mqtt_decode_length(const unsigned char **, unsigned long *);

int unpack_mqtt_packet(const unsigned char *, union mqtt_packet *);
unsigned char *pack_mqtt_packet(const union mqtt_packet *, unsigned);

union mqtt_header *mqtt_packet_header(unsigned char);
struct mqtt_ack *mqtt_packet_ack(unsigned char, unsigned short);
struct mqtt_connack *mqtt_packet_connack(unsigned char, unsigned char,
                                         unsigned char);
struct mqtt_suback *mqtt_packet_suback(unsigned char, unsigned short,
                                       unsigned char *, unsigned short);
struct mqtt_publish *mqtt_packet_publish(unsigned char, unsigned short, size_t,
                                         unsigned char *, size_t,
                                         unsigned char *);
void mqtt_packet_release(union mqtt_packet *, unsigned);

// New function prototypes for properties
int mqtt_property_add(struct mqtt_properties *props,
                      struct mqtt_property *prop);
struct mqtt_property *mqtt_property_get(struct mqtt_properties *props,
                                        enum mqtt_property_type type);
void mqtt_properties_free(struct mqtt_properties *props);
int mqtt_properties_encode(struct mqtt_properties *props, unsigned char *buf,
                           size_t buflen);
int mqtt_properties_decode(const unsigned char *buf, size_t buflen,
                           struct mqtt_properties *props);

#endif // MQTT_H