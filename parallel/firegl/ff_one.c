#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TREE 2
#define FIRE 1
#define FIRE_STARTED 3
#define SPARK 4
#define MISSING 0

typedef struct Node {
	int i;
	int j;
} Tree;

Tree** forest;
Tree** aforest;

int fsize;

void wait(unsigned int ms) {
	struct timeval t;
	gettimeofday(&t,NULL);
	double tf = t.tv_sec + (t.tv_usec + ms*1000.0) / 1000000.0 ;
	gettimeofday(&t,NULL);
	while(t.tv_sec + (t.tv_usec) / 1000000.0 < tf) gettimeofday(&t,NULL);
}
void flushforest(int width) {
	register int i;
	int finish = width*width;
	for(i=0;i<finish;i++) {
		// printf("%d %d\n",i, (unsigned int)(forest[i]));
		// if(forest[i] != NULL) free(forest[i]);
		forest[i] = NULL;
	}
}

void printff(int* ffm,int width) {

	register int i,j;

	for(i=0;i<width;i++) {
		printf("\x1B[F");
	}

	for(i=0;i<width;i++) {
		for(j=0;j<width;j++) {
			switch(*(ffm + width * i + j)) {
				case TREE:
					printf("X");
					break;
				case SPARK:
				case FIRE:
				case FIRE_STARTED:
					printf("\x1B[33m&\x1B[0m");
					break;
				default:
					printf("-");
					break;
			}
		}
		printf("\n");
	}

}

void burn(int* ptr, int i, int j) {
	switch(*ptr) {
		case TREE:
			*ptr = SPARK;
			break;
		case FIRE:
			*ptr = MISSING;
	}
}

int tick(int* ffm, int width) {
	register int i,j,k,count;
	for(k=0;k<begindex;k++) {
		i = forest[k]->i;
		j = forest[k]->j;
		if(*(ffm + i * width + j) == FIRE) {
			*(ffm + i * width + j) = MISSING;
			if(i > 0) {
				int* ptr = ffm + (i-1) * width + j;
				burn(ptr, i-1, j);
			}
			if(i < width - 1) {
				int* ptr = ffm + (i+1) * width + j;
				burn(ptr, i+1, j);
			}
			if(j > 0) { 
				int* ptr = ffm + i * width + j - 1;
				burn(ptr, i, j-1);
			}
			if(j < width - 1) {
				int* ptr = ffm + i * width + j + 1;
				burn(ptr, i, j+1);
			}
		}
		free(forest[k]);
	}
	
	return count;
}

void flushall(int width) {
	flushforest(width);
}

int regenerate(int* ffm, int width, double prob) {
	register int i,j;
	int count = 0;
	/* for(i=0;i<width*width;i++) {
		*(ffm + i) = MISSING;
	} */
	for(i=0;i<width;i++) {
		for(j=0;j<width;j++) {
			if((double)rand() / (double)RAND_MAX < prob) {
				*(ffm + width * i + j) = j==0 ? FIRE : TREE;
				if(j==0) {
					Tree* tmp = (Tree*)malloc(sizeof(Tree));
					tmp->j = 0;
					tmp->i = i;
					forest[begindex++] = tmp;
					count++;
					fsize++;
				}
			}
			else {
				*(ffm + width * i + j) = MISSING;
			}
		}
	}

	// Set fire to the trees
	/*
	int count = 0;
	for(i=0;i<width;i++) {
		if(*(ffm + width * i) == TREE) {
			*(ffm + width * i) = FIRE;
			Tree* tmp = (Tree*)malloc(sizeof(Tree));
			tmp->j = 0;
			tmp->i = i;
			forest[begindex++] = tmp;
			count++;
		}
	}
	return count;
	*/
}

int main(int argc, char* argv[]) {
	register int bigl;

	if(argc<3) {
		printf("usage: ff width prob\n");
		return 1;
	}

	int width = atoi(argv[1]); //1000;
	double prob = atof(argv[2]); //500;

	struct timeval t;
	gettimeofday(&t,NULL);
	srand(t.tv_sec + t.tv_usec);

	// printf("%7.6f %f\n",0.0,0.0); // 0

	Tree** cf = malloc(sizeof(Tree*) * width * width);
	forest = cf;

	int* ffm;
	ffm = malloc(sizeof(int) * width * width);


	unsigned register int total = 0;
	
	flushall(width);

	register int count = regenerate(ffm,width,prob);


	printff(ffm,width);
	// printf("%d %f\n",count,prob);
	// wait(1000);

	unsigned register int time = 0;

	while(count) {
		count = tick(ffm,width);
		// printf("\t%d\n",count);
		printff(ffm,width);
		wait(50);
		// printf("%d %d\n",loop,++time);
		// wait(2000);
		time++;
	}
	total += time; // / (double)width;
	// printf("\tTotal: %d\n",total);

	// printf("freeing\n");

	printf("%7.6f %f\n",prob,(double)(total)/(double)(width));
	
	// printf("%7.6f %f\n",1.0,1.0);

	free(cf);
	free(ffm);

	return 0;
}
