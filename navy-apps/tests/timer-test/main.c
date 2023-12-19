#include <stdio.h>
#include <assert.h>

extern uint32_t NDL_GetTicks();

int main()
{
	struct timeval tv;
	uint32_t ms = 500;
	uint32_t ticks = NDL_GetTicks();
	while(1){
		while(ticks < ms){
			ticks = NDL_GetTicks();
		}
		printf("ms = %d\n", (int)ms);
		ms += 500;
	}
}