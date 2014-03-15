#ifndef _BQ3060_H_
#define _BQ3060_H_

#define BQ3060_ADDR (0x16 >> 1)

#define LOWDATA_BYTE(data) ((data) & 0xFF)
#define HIGHDATA_BYTE(data) (((data) & 0xFF00) >> 8)
#define DATA_FROM_BYTES(low, high) (((low) & 0xFF) | ((high) &0xFF) << 8)

struct BQ3060Config{
    I2CDriver *I2CD;
};

extern struct BQ3060Config rnh3060conf;

void BQ3060_init(struct BQ3060Config * conf);
int BQ3060_Set(uint8_t register_id, uint16_t data);
int BQ3060_Get(uint8_t register_id, uint16_t* data);

#endif /* _BQ3060_H_ */
