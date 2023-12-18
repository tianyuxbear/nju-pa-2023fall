#include <stdio.h>
#include <assert.h>

int main()
{
	struct timeval tv;
	uint64_t ms = 500;
	gettimeofday(&tv, NULL);
	while(1){
		while((tv.tv_sec * 1000 + tv.tv_usec / 1000) < ms){
			gettimeofday(&tv, NULL);
		}
		printf("ms = %d\n", (int)ms);
		ms += 500;
	}
}