#ifndef SERIAL_H_
#define SERIAL_H_

void serial_init();
char serial_readbyte();
void serial_write(const char *s);

#endif
