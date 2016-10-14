#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TREE 2
#define FIRE 1
#define FIRE_STARTED 3
#define MISSING 0

void wait(unsigned int ms) {
	struct timeval t;
	gettimeofday(&t,NULL);
	double tf = t.tv_sec + (t.tv_usec + ms*1000.0) / 1000000.0 ;
	gettimeofday(&t,NULL);
	while(t.tv_sec + (t.tv_usec) / 1000000.0 < tf) gettimeofday(&t,NULL);
}

void printff(int* ffm,int width,int length,int clear) {

	char p[1000000] = { '\0' };
	register int i,j,offset,ei;

	if(clear) {
		for(i=0;i<width;i++) {
			printf("\x1B[F");
		}
	}

	offset = 0;
	ei = 0;
	for(i=0;i<width;i++) {
		for(j=0;j<length;j++) {
			switch(*(ffm + length * i + j)) {
				case TREE:
					p[offset++] = '\x1B';
					p[offset++] = '[';
					p[offset++] = '3';
					p[offset++] = '2';
					p[offset++] = 'm';
					p[offset++] = 'X';
					p[offset++] = '\x1B';
					p[offset++] = '[';
					p[offset++] = '0';
					p[offset++] = 'm';
					break;
				case FIRE:
					p[offset++] = '\x1B';
					p[offset++] = '[';
					p[offset++] = '3';
					p[offset++] = '3';
					p[offset++] = 'm';
					p[offset++] = '*';
					p[offset++] = '\x1B';
					p[offset++] = '[';
					p[offset++] = '0';
					p[offset++] = 'm';
					break;
				case FIRE_STARTED:
					p[offset++] = '\x1B';
					p[offset++] = '[';
					p[offset++] = '3';
					p[offset++] = '3';
					p[offset++] = 'm';
					p[offset++] = '&';
					p[offset++] = '\x1B';
					p[offset++] = '[';
					p[offset++] = '0';
					p[offset++] = 'm';
					break;
				default:
					p[offset++] = '-';
					break;
			}
		}
		p[offset++] = '\n';
		ei++;
	}
	printf("%s",p);

}

void burn(int* ptr) {
	if(*ptr == TREE) {
		*ptr = FIRE_STARTED;
	}
}

int tick(int* ffm, int width, int length) {
	register int i,j,count;
	for(i=0;i<width;i++) {
		for(j=0;j<length;j++) {
			if(*(ffm + i * length + j) == FIRE) {
				*(ffm + i * length + j) = MISSING;
				if(i > 0) {
					int* ptr = ffm + (i-1) * length + j;
					burn(ptr);
				}
				if(i < length - 1) {
					int* ptr = ffm + (i+1) * length + j;
					burn(ptr);
				}
				if(j > 0) { 
					int* ptr = ffm + i * length + j - 1;
					burn(ptr);
				}
				if(j < length - 1) {
					int* ptr = ffm + i * length + j + 1;
					burn(ptr);
				}
			}
		}
	}

	count = 0;
	for(i=0;i<width;i++) {
		for(j=0;j<length;j++) {
			if(*(ffm + length * i + j) == FIRE_STARTED) {
				*(ffm + length * i + j) = FIRE;
				count++;
			}
		}
	}
	return count;
}

int main(int argc, char* argv[]) {

	double prob;
	int width = 80;
	int length = 300;
	int* ffm;

	srand(077777U);

	if(argc > 1) {
		prob = atof(argv[1]);
	} else {
		// printf("Supply a number from 0 to 1.\n");
		return 1;
	}

	ffm = malloc(sizeof(int) * width * length);
	register int i,j;
	for(i=0;i<width*length;i++) {
		*(ffm + i) = MISSING;
	}
	for(i=0;i<width;i++) {
		for(j=0;j<length;j++) {
			if((double)rand() / (double)RAND_MAX < prob) {
				*(ffm + length * i + j) = TREE;
			}
		}
	}

	// Set fire to the trees
	int count = 0;
	for(i=0;i<width;i++) {
		if(*(ffm + length * i) == TREE) {
			*(ffm + length * i) = FIRE;
			count++;
		}
	}

	printff(ffm,width,length,0);
	wait(25);

	unsigned register int time = 0;

	while(count) {
		count = tick(ffm,width,length);
		printff(ffm,width,length,1);
		wait(25);
		++time;
	}
	
	printf("%d\n",time);

	return 0;
}