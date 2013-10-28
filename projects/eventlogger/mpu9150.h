void    mpu9150_init(I2CDriver* i2cptr);
void    mpu9150_int_event_handler(eventid_t _);
void    mpu9150_ISR(EXTDriver* _, expchannel_t __);
msg_t   Thread_mpu9150_int_dispatch(void* _);
