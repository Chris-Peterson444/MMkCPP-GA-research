#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <errno.h>
//#include <ctime>
#include <time.h>

#include "type.h"
#include "case.h"

extern int errno;

void Statistics(IPTR, Population *p);
void Report(int gen, IPTR pop, Population *p);
void Initialize(int argc, char *argv[], Population *p, Functions *f);

void WritePid(char * pidFile);
void RmPidFile(char *pidFile);

//struct timespec start, finish;
//double elapsed;


int main(int argc, char *argv[])
{
	// Pointer to INDIVIDUAL struct in type.h
	IPTR tmp;
	int foo; /* just a placeholder for a value that is not used */
	Population pop, *p; // Workhorse object. Has all the data I think
	Functions funcs, *f; // Attempt at dynamic function defintion

	// Why are we pointing to variables on the stack here? 
	p = &pop;
	f = &funcs;

	// Initialize genetic algorithm. We are on the first generation
	p->generation = 0;

	// GA setup. Read in files. Etc.
	Initialize(argc, argv, p, f);

	//WritePid(p->pidFile);

	while (p->generation < p->maxgen) {

		//start time recording
		//time_t start = time(clock());
	    //time(start);

		p->generation++;

		// Seriously didn't need to return anything. "Current GA" is whatever GA type used. Defined in Initialize
		foo = f->CurrentGA(p->oldpop, p->newpop, p->generation, p, f);

		// Injection stuff??
		if (p->injectFraction > 0.0) {
			if ((p->generation%p->injectPeriod == 0)
				&& (p->generation <= p->injectStop)) {
				LoadCases(p->newpop, p->generation, p->injectFraction, p, f); // Generate new pop?
				/* printf("Loaded cases %d\n", (int) (loadPerc/100.0 * popsize));*/
			}
		}
		// Get stats on new pop
		Statistics(p->newpop, p);
		// Report on new pop
		Report(p->generation, p->newpop, p);

		//Record data (best individual at each gen) 
		FILE * dataFile;

		dataFile = fopen("myData.txt", "a");

		fprintf(dataFile, "%f\n", p->newpop[p->maxi].objfunc);

		fclose(dataFile);

		//Record best route
		FILE * routeFile;
		routeFile = fopen("myRoutes.txt", "a");
		PhenoPrint(routeFile, p->newpop, p);
		printf(routeFile, "\n");
		for(int i = 0; i < p->newpop->chromLen; i++)
			fprintf(routeFile, "%d, ", p->newpop->chrom[i]);
		fprintf(routeFile, "\n");
		fclose(routeFile);

		tmp = p->oldpop;
		p->oldpop = p->newpop;
		p->newpop = tmp;

		

		//record time
		//time_t end = time(clock());
		//double elapsed = difftime(end, start);
		
		//printf("\nTime Elapsed: %f\n", elapsed);
		//FILE * tFile;
		//tFile = fopen("myGenTimes.txt", "a");
		//fprintf(tFile, "%f\n", elapsed);
		//fclose(tFile);
	}
	if (p->nCurrentCases > 0) {
		p->nCases = FindNCases(p->nCFile);
		StoreNcases(p->nCFile, p->nCases, p->nCurrentCases);
	}
	//RmPidFile(p->pidFile);

	return 0;
}

void WritePid(char *fname)
{
	struct stat buf;
	int er;
	FILE *fp;

	er = stat(fname, &buf);
	if (!(er == -1 || errno == ENOENT)) {
		fprintf(stderr, "Lock file (%s) exists, Process running\n", fname);
		fprintf(stderr, "This process is exiting....\n");
		exit(1);
	}

	if ((fp = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "Error in opening file %s for writing\n", fname);
		exit(2);
	}

	fprintf(fp, "%lu", getpid());
}

void RmPidFile(char *fname)
{
	unlink(fname);
}

