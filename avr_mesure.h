#if !defined(AVR_MESURE_H)
#define AVR_MESURE_H 1

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_16MHz       0x00
#define CPU_8MHz        0x01
#define CPU_4MHz        0x02
#define CPU_2MHz        0x03
#define CPU_1MHz        0x04
#define CPU_500kHz      0x05
#define CPU_250kHz      0x06
#define CPU_125kHz      0x07
#define CPU_62kHz       0x08

#define HEX(n) (((n) < 10) ? ((n) + '0') : ((n) + 'A' - 10))

uint16_t readAdc(uint8_t ch);
void writeHex(uint16_t val);

#endif /* !defined(AVR_MESURE_H) */
