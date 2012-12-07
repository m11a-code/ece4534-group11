typedef struct __timer0_thread_struct {
	// "persistent" data for this "lthread" would go here
	int	data;
} timer0_thread_struct;

#define SONAR_SEND 1
#define SONAR_RECV 2
#define CAMERA 3
#define ENCODERS 4

void init_timer0_lthread(timer0_thread_struct *);

int timer0_lthread(timer0_thread_struct *,int,int,unsigned char*);
