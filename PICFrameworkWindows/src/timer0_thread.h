typedef struct __timer0_thread_struct {
	// "persistent" data for this "lthread" would go here
	int	data;
} timer0_thread_struct;

#define SONAR 1
#define CAMERA 2
#define ENCODERS 3

void init_timer0_lthread(timer0_thread_struct *);

int timer0_lthread(timer0_thread_struct *,int,int,unsigned char*);
