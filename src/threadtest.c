#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


int fib(int i){
	if ( i <= 0 ){
		return 1;
	}
	return fib(i-1)+fib(i-2);
}

void *job(void *arg){
	int s;
	s = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	printf("%d for setcanceltype()\n",s);
	printf("thread started\n");
	while(1){
		int i =0;
		if(i <0 ){
			i=i+100;
		}
		if(i >10000){
			i = -100;
		}
		if(i==10001){
			printf("thread got 10001\n");
			break;
		}
	}
	//for(int i=40; i<10000; i++){
	//	printf("fib(%d) = %d\n", i, fib(i));
	//}
}



int main(){
	int s;
	pthread_t thread;
	s = pthread_create(&thread, NULL, &job, NULL);
	printf("pthread returned %d\n",s);
	getc(stdin);
	printf("Cacnelling thread\n");
	s = pthread_cancel(thread);
	printf("thread cancel returned %d", s);
	printf("joining thread");
	void *res;
	s = pthread_join(thread, &res);
	printf("%d", res == PTHREAD_CANCELED);
}
