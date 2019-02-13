#define MAX_HOPS 512

typedef struct potato {

	int total_hop_num;
  	unsigned long hop_trace[MAX_HOPS*4];

}POTATO;