#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>
#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)
queue_t q;
/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}
/* Enque then Destroy*/
void test_queue_destroy(void){
	fprintf(stderr, "*** TEST queue_destroy ***\n");
	q = queue_create();
	TEST_ASSERT(queue_destroy(q) != 1);
}
/* Increment by 1, delete 5*/

static void inc_item(void* data){
	int* a = (int*)data;
	if(*a == 5){
		queue_delete(q, data);
	} else {
		*a += 1;
	}
}
static void dec_item(void* data){
	int* a = (int*)data;
	if(*a == 2){
		queue_delete(q, data);
	} else {
		*a -= 1;
	}
}
void test_iterator_inc(void){
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	int i;
	fprintf(stderr, "*** TEST queue_iterator_inc ***\n");
	q = queue_create();
	for(i = 0; i < sizeof(data) / sizeof(data[0]); i++){
		queue_enqueue(q, &data[i]);
	}
	queue_iterate(q, inc_item);
	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(queue_length(q) == 9);

}

void test_iterator_dec(void){
	int data[] = {2,2,3};
	int i;
	fprintf(stderr, "*** TEST queue_iterator_dec ***\n");
	q = queue_create();
	for(i = 0; i < sizeof(data) / sizeof(data[0]); i++){
		queue_enqueue(q, &data[i]);
	}
	queue_iterate(q, dec_item);
	printf("%d\n", queue_length(q));
	TEST_ASSERT(queue_length(q) == 1);
	
}




int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_destroy();
	test_iterator_inc();
	test_iterator_dec();
	return 0;
}
