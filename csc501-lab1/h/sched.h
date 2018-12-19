#define EXPDISTSCHED 1
#define LINUXSCHED 2
#define DEFAULTSCHED 3

void setschedclass (int sched_class);
int getschedclass();
int defaultsched();
int expdistsched();
int linuxsched();
int newepoch();
int getnext();
int isnewepoch();