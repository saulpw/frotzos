#ifndef DEV_TIME_H_
#define DEV_TIME_H_

// timer
#define TIMER_HZ 100

extern void setup_timer();
extern void isr_timer();

double seconds(); // since startup

#endif
