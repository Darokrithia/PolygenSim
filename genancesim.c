#include "jobqueue.h"
#include "degnome.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>

void usage(void);
double get_fitness(double hat_size);

const char *usageMsg =
    "Usage:\n"
    "genancesim [-c CL] [-p PS] [-g G] [-o CR] [-s]|[-u] [-v]\n"
    "\n"
    "\"CL\" is chromosome length.  \"PS\" is the population size, \"G\"\n"
    "is thenumber of generations that the simulation will run,\n"
    "and \"CR\" is crossover rate.  If \"-s\" is present, there will\n"
    "be selection, while if \"-u\" is present all degnomes will\n"
    "contribute to two offspring.  Note: \"-s\" and \"-u\" cannot be\n"
    "simultaneously active. If \"-v\" is present there will be\n"
    "output at every single generation.\n"
    "They can be in any order, and not all are needed. Default CL\n"
    "is 10, default PS is 10, the default G is 1000, and defualt\n"
    "CR is 2.\n";

pthread_mutex_t seedLock = PTHREAD_MUTEX_INITIALIZER;
unsigned long rngseed=0;

//	there is no need for the line 'int chrom_size' as it is declared as a global variable in degnome.h
int pop_size;
int num_gens;
int crossover_rate;
int selective;
int uniform;
int verbose;

void usage(void) {
	fputs(usageMsg, stderr);
	exit(EXIT_FAILURE);
}


double get_fitness(double hat_size){
	return hat_size;
}

int main(int argc, char **argv){

	chrom_size = 10;
	pop_size = 10;
	num_gens = 1000;
	crossover_rate = 2;
	selective = 0;
	uniform = 0;
	verbose = 0;

	if(argc > 11){
		printf("\n");
		usage();
	}
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if (strcmp(argv[i], "-c" ) == 0 && argc > (i+1)){
				sscanf(argv[i+1], "%u", &chrom_size);
				i++;
			}
			else if (strcmp(argv[i], "-p") == 0 && argc > (i+1)){
				sscanf(argv[i+1], "%u", &pop_size);
				i++;
			}
			else if (strcmp(argv[i], "-g") == 0 && argc > (i+1)){
				sscanf(argv[i+1], "%u", &num_gens);
				i++;
			}
			else if (strcmp(argv[i], "-o") == 0 && argc > (i+1)){
				sscanf(argv[i+1], "%u", &crossover_rate);
				i++;
			}
			else if (strcmp(argv[i], "-s") == 0){
				if(uniform){
					usage();
				}
				selective = 1;
			}
			else if (strcmp(argv[i], "-u") == 0){
				if(selective){
					usage();
				}
				uniform = 1;
			}
			else if (strcmp(argv[i], "-v") == 0){
				verbose = 1;
			}
			else{
				printf("\n");
				usage();
			}
		}
		else{
			usage();
		}
	}

    time_t currtime = time(NULL);                  // time
    unsigned long pid = (unsigned long) getpid();  // process id
    rngseed = currtime ^ pid;                      // random seed
    gsl_rng* rng = gsl_rng_alloc(gsl_rng_taus);    // rand generator

	Degnome* parents;
	Degnome* children;
	Degnome* temp;

	printf("%u, %u, %u\n", chrom_size, pop_size, num_gens);

	parents = malloc(pop_size*sizeof(Degnome));
	children = malloc(pop_size*sizeof(Degnome));

	for (int i = 0; i < pop_size; i++){
		parents[i].dna_array = malloc(chrom_size*sizeof(double));
		children[i].dna_array = malloc(chrom_size*sizeof(double));
		parents[i].hat_size = 0;

		for(int j = 0; j < chrom_size; j++){
			parents[i].dna_array[j] = (i);	//children isn't initialized
			parents[i].hat_size += (i);		//all genes are a degnome are identical so they are easy to source
		}
	}

	printf("\nGeneration 0:\n\n");
	for(int i = 0; i < pop_size; i++){
		printf("Degnome %u\n", i);
		for(int j = 0; j < chrom_size; j++){
			printf("%lf\t", parents[i].dna_array[j]);
		}
		printf("\n");
	}
	printf("\n\n");

	for(int i = 0; i < num_gens; i++){
		if(!uniform){
			double fit;
			if(selective){
				fit = get_fitness(parents[0].hat_size);
			}
			else{
				fit = 100;			//in runs withoutslection, everybody is equally fit
			}

			double total_hat_size = fit;
			double cum_hat_size[pop_size];
			cum_hat_size[0] = fit;

			for(int j = 1; j < pop_size; j++){
				if(selective){
					fit = get_fitness(parents[j].hat_size);
				}
				else{
					fit = 100;
				}

				total_hat_size += fit;
				cum_hat_size[j] = (cum_hat_size[j-1] + fit);
			}

			for(int j = 0; j < pop_size; j++){

			    pthread_mutex_lock(&seedLock);
				gsl_rng_set(rng, rngseed);
			    rngseed = (rngseed == ULONG_MAX ? 0 : rngseed + 1);
	    		pthread_mutex_unlock(&seedLock);

				int m, d;

				double win_m = gsl_rng_uniform(rng);
				win_m *= total_hat_size;
				double win_d = gsl_rng_uniform(rng);
				win_d *= total_hat_size;

				// printf("win_m:%lf, wind:%lf, max: %lf\n", win_m,win_d,total_hat_size);

				for (m = 0; cum_hat_size[m] < win_m; m++){
					continue;
				}

				for (d = 0; cum_hat_size[d] < win_d; d++){
					continue;
				}

				// printf("m:%u, d:%u\n", m,d);

				Degnome_mate(children + j, parents + m, parents + d, rng, 0, 0, crossover_rate);
			}
		}
		else{
			// printf("uniform!!!\n");

			int moms[pop_size];
			int dads[pop_size];
			int mom_max = pop_size;
			int dad_max = pop_size;

			int m, d;

			for(int j = 0; j < pop_size; j++){
				moms[j] = j;
				dads[j] = j;
			}

			for(int j = 0; j < pop_size; j++){
				pthread_mutex_lock(&seedLock);
				gsl_rng_set(rng, rngseed);
			    rngseed = (rngseed == ULONG_MAX ? 0 : rngseed + 1);

	    		pthread_mutex_unlock(&seedLock);

				int index_m = (int) gsl_rng_uniform_int (rng, mom_max);
				int index_d = (int) gsl_rng_uniform_int (rng, dad_max);

				m = moms[index_m];
				d = dads[index_d];

				// printf("m:\t%u\nd:\t%u\n", m, d);
				
				//reduce the pool of available degnomes
				//in order to make sure everybody get's two chances to mate
				//one as a dad and one as a mom
			    
				int temp_m = moms[index_m];
				int temp_d = dads[index_d];
				moms[index_m] = moms[mom_max-1];
				dads[index_d] = dads[dad_max-1];
				moms[mom_max-1] = temp_m;
				dads[dad_max-1] = temp_d;

				mom_max--;
				dad_max--;

				Degnome_mate(children + j, parents + m, parents + d, rng, 0, 0, crossover_rate);
			}
		}
		temp = children;
		children = parents;
		parents = temp;
	}

	printf("Generation %u:\n", num_gens);
	for(int i = 0; i < pop_size; i++){
		printf("Degnome %u\n", i);
		for(int j = 0; j < chrom_size; j++){
			printf("%lf\t", parents[i].dna_array[j]);
		}
		printf("\nTOTAL HAT SIZE: %lg\n\n", parents[i].hat_size);
	}

	//free everything

	for (int i = 0; i < pop_size; i++){
		free(parents[i].dna_array);
		free(children[i].dna_array);
		parents[i].hat_size = 0;
	}

	free(parents);
	free(children);

	gsl_rng_free (rng);
}