#define MAX 512
//Struct for the Potato

typedef struct potato {

	int hop_num; //Total hops remaining
    int hop_trace[MAX]; //Keep track of the trace
    int current_hop; //Current hp number

}POTATO;
