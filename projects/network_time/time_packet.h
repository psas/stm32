/*time_packet.h 
structure for time packets */

struct time_packet {
	uint16_t msg_type;
	uint32_t t1_secs;
	uint32_t t1_ms;
	uint32_t t2_secs;
	uint32_t t2_ms; 
} __attribute__((packed));

typedef struct time_packet time_packet;
