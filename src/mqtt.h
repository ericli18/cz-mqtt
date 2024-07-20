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
/*
 * Represents an MQTT CONNECT packet structure.
 *
 * This struct encapsulates all the fields necessary for an MQTT CONNECT packet,
 * including the fixed header, connect flags, and variable header payload.
 *
 * @field header       The fixed header for the MQTT packet.
 * @field byte         A union member to access all connect flags as a single byte.
 * @field bits         A bitfield struct to access individual connect flags:
 *                     - reserved: Reserved bit, must be 0.
 *                     - clean_session: If 0, Server must resume communications with Client from current Session
 *                                      If 1, Client and Server must discard previous Session and start new one
 *                     - will: If 1, Will topic and Will message MUST be in payload and used by server
 *                             If 0, Connect flags set to 0, Will MUST NOT be present in the payload
 *                     - will_qos: QoS level for the Will message. Check will flag
 *                     - will_retain: Will Retain flag. Check will flag
 *                     - password: Password flag. If 1, Password must be present in payload
 *                                                Must be 0 if Username is 0
 *                     - username: Username flag. If 1, Username must be present in payload
 * @field payload      The variable header and payload of the CONNECT packet:
 *                     - keepalive: The Keep Alive timer value.
 *                     - client_id: The Client Identifier.
 *                     - username: The Username, if the username flag is set.
 *                     - password: The Password, if the password flag is set.
 *                     - will_topic: The Will Topic, if the will flag is set.
 *                     - will_message: The Will Message, if the will flag is set.
 */
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

// Reference: 3.2
/* 
 * Represents an MQTT CONNACK packet structure.
 *
 * This struct encapsulates the fields of an MQTT CONNACK (Connection Acknowledgement) packet,
 * which is sent by the server in response to a client's CONNECT packet.
 *
 * @field header           The fixed header for the MQTT packet.
 * @field byte             A union member to access all connect acknowledge flags as a single byte.
 * @field bits             A bitfield struct to access individual connect acknowledge flags:
 *                         - session_present: Indicates whether the server already has a session for the client.
 *                         - reserved: The remaining 7 bits are reserved and should be set to 0.
 * @field rc               The Connect Return code, indicating the result of the connection request.
 *                         Possible values are defined in the MQTT specification.
 */
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

//reference: 3.8

/*
 * Represents an MQTT SUBSCRIBE packet structure.
 *
 * This struct encapsulates the fields necessary for an MQTT SUBSCRIBE packet,
 * which is sent from the Client to the Server to create one or more Subscriptions.
 *
 * @field header       The fixed header for the MQTT packet.
 * @field pkt_id       The Packet Identifier for this SUBSCRIBE packet.
 * @field tuples_len   The number of Topic Filter/QoS pairs in the packet.
 * @field tuples       An array of Topic Filter/QoS pairs:
 *                     - topic_len: The length of the Topic Filter.
 *                     - topic: The Topic Filter, a UTF-8 encoded string.
 *                     - qos: The maximum QoS level at which the Server can send
 *                            Application Messages to the Client.
 */
struct mqtt_subscribe {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short tuples_len;
    struct {
        unsigned short topic_len;
        unsigned char *topic;
        unsigned qos;
    } *tuples;
};

//Reference 3.10
struct mqtt_unsubscribe {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short tuples_len;
    struct {
        unsigned short topic_len;
        unsigned char *topic;
    } *tuples;
};

//Reference 3.9
struct mqtt_suback {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short rcslen;
    unsigned char *rcs;
};


//Reference: 3.3
struct mqtt_publish {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short topiclen;
    unsigned char *topic;
    unsigned short payloadlen;
    unsigned char *payload;
};

//Reference: 3.4, can be used for all other Acknowledgement headers
struct mqtt_ack {
    union mqtt_header header;
    unsigned short pkt_id;
};

typedef struct mqtt_ack mqtt_puback;
typedef struct mqtt_ack mqtt_pubrec;
typedef struct mqtt_ack mqtt_pubrel;
typedef struct mqtt_ack mqtt_pubcomp;
typedef struct mqtt_ack mqtt_unsuback;
typedef union mqtt_header mqtt_pingreq;
typedef union mqtt_header mqtt_pingresp;
typedef union mqtt_header mqtt_disconnect;

#endif