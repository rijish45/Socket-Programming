#define MAX_HOPS 512

typedef struct potato {

	int total_hop_num;
	int hop_count;
	char message_type;
  	unsigned long hop_trace[MAX_HOPS];


}