#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define TREE 2
#define FIRE 1
#define FIRE_STARTED 3
#define MISSING 0

typedef struct Node {
	int i;
	int j;
} Tree;

Tree** forest;
Tree** aforest;
int begindex;
int abegindex;

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
	begindex = 0;
}

void flushaforest(int width) {
	register int i;
	int finish = width*width;
	for(i=0;i<finish;i++) {
		aforest[i] = NULL;
	}
	abegindex = 0;
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
				case FIRE:
					printf("\x1B[33m*\x1B[0m");
					break;
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
	if(*ptr == TREE) {
		// *ptr = FIRE_STARTED;
		Tree* tmp = (Tree*)malloc(sizeof(Tree));
		tmp->i = i;
		tmp->j = j;
		aforest[abegindex++] = tmp;
	}
}

int tick(int* ffm, int width) {
	// flushaforest(width);

	abegindex = 0;

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
	
	// flushforest(width);

	begindex = 0;

	count = 0;
	for(k=0;k<abegindex;k++) {
		i = aforest[k]->i;
		j = aforest[k]->j;
		*(ffm + width * i + j) = FIRE;
		// Tree* tmp = (Tree*)malloc(sizeof(Tree));
		// tmp->i = i;
		// tmp->j = j;
		// forest[begindex++] = tmp;
		forest[begindex++] = aforest[k];
		count++;
		// free(aforest[k]);
	}

	return count;
}

void flushall(int width) {
	flushaforest(width);
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

	int ierr, pid, num_procs;

	MPI_Status status;
	int tag = 0;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	char pname[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(pname, &name_len);

	register int bigl;

	if(argc<4) {
		printf("usage: ff width iterations resolution\n");
		return 1;
	}

	int width = atoi(argv[1]); //1000;
	int iterations = atoi(argv[2]); //500;
	int resolution = atoi(argv[3]); //1000;

	double increment = 1.0/resolution;

	double prob;

	if(pid == 0) {
		printf("Manager at %s %f\n",pname,increment);
		int i;
		double prob = .55;
		for(i=1;i<num_procs;i++) {
			MPI_Send( &prob, 1, MPI_DOUBLE, i, tag, MPI_COMM_WORLD );
			prob += increment;
		}
		while(prob < .65) {
			double ans;
			MPI_Recv( &ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status );
			int node = status.MPI_SOURCE;
			// printf("%12s (%03d): %7.6f %f\n",pname,node,prob,ans);
			prob += increment;
			// printf("\tSending %f to %d\n",prob,i);
			MPI_Send( &prob, 1, MPI_DOUBLE, node, tag, MPI_COMM_WORLD );
			// printf("\t\t\tSent\n");
		}
		printf("Calling finalize\n");
		ierr = MPI_Finalize();
		printf("Called finalize\n");
	} else {
		while(1) {
			// printf("%7.6f %f\n",0.0,0.0); // 0

			Tree** cf = malloc(sizeof(Tree*) * width * width);
			Tree** cfa = malloc(sizeof(Tree*) * width * width);

			forest = cf;
			aforest = cfa;

			int* ffm;
			ffm = malloc(sizeof(int) * width * width);

			unsigned register int total = 0;

			// printf("Receiving on %s (%03d)\n",pname,pid);

			MPI_Recv( &prob, 1, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, &status );
			
			// printf("Received %f on %s (%03d)\n",prob,pname,pid);

			register int loop;
			for(loop=0;loop<iterations;loop++) {

				flushall(width);

				register int count = regenerate(ffm,width,prob);


				// printff(ffm,width);
				// printf("%d %f\n",count,prob);
				// wait(1000);

				unsigned register int time = 0;

				while(count) {
					count = tick(ffm,width);
					// printf("\t%d\n",count);
					// printff(ffm,width);
					// wait(1000);
					// printf("%d %d\n",loop,++time);
					// wait(2000);
					time++;
				}
				total += time; // / (double)width;
				// printf("\tTotal: %d\n",total);

				// printf("freeing\n");


			}

			double ans;
			ans = (double)(total)/(double)(loop*width);

			printf("%12s (%03d): %7.6f %f\n",pname,pid+1,prob,ans);

			MPI_Send( &ans, 1, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD );
			
			// printf("%7.6f %f\n",1.0,1.0);

			free(cfa);
			free(cf);
			free(ffm);
		}
		MPI_Finalize();
	}
	return 0;
}
