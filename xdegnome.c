/**
 * @file xdegnome.c
 * @author Daniel R. Tabin
 * @brief Unit tests for Degnome
 */

#include "degnome.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>

pthread_mutex_t seedLock = PTHREAD_MUTEX_INITIALIZER;
unsigned long rngseed=0;

int main(int argc, char **argv){
	int verbose = 0;

	if(argc == 2) {
        if(strncmp(argv[1], "-v", 2) != 0) {
            fprintf(stderr, "usage: xdegnome [-v]\n");
            exit(EXIT_FAILURE);
        }
        verbose = 1;
    }
    else if(argc != 1){
        fprintf(stderr, "usage: xdegnome [-v]\n");
        exit(EXIT_FAILURE);
    }

	chrom_size = 10;

	if(verbose){
		printf("Chromosome is length %u\n", chrom_size);
	}

	Degnome* bom_mom = Degnome_new(); // good parent
	Degnome* bad_dad = Degnome_new();	// bad parent
	Degnome* tst_bby = Degnome_new(); // their child

	time_t currtime = time(NULL);                  // time
    unsigned long pid = (unsigned long) getpid();  // process id
    rngseed = currtime ^ pid;                      // random seed
    gsl_rng* rng = gsl_rng_alloc(gsl_rng_taus);    // rand generator

	for(int i = 0; i < chrom_size; i++){
		bom_mom->dna_array[i] = 2;
		bad_dad->dna_array[i] = 1;

		bom_mom->hat_size += bom_mom->dna_array[i];
		bad_dad->hat_size += bad_dad->dna_array[i];
	}

	if(verbose){
		printf("pre-mating values:\n");
		for(int i = 0; i < chrom_size; i++){
       	 	printf("Mom: %lf\t Dad: %f\n", bom_mom->dna_array[i], bad_dad->dna_array[i]);
		}
		printf("Mom hat_size: %lf\t Dad hat_size: %f\n", bom_mom->hat_size, bad_dad->hat_size);
	}

	Degnome_mate(tst_bby, bom_mom, bad_dad, rng);


	if(verbose){
		printf("post-mating values:\n");
		for(int i = 0; i < chrom_size; i++){
       	 	printf("Mom: %lf\t Dad: %f\t Kid: %f\n", bom_mom->dna_array[i], bad_dad->dna_array[i], tst_bby->dna_array[i]);
		}
		printf("Mom hat_size: %lf\t Dad hat_size: %f\t Kid hat_size: %f\n", bom_mom->hat_size, bad_dad->hat_size, tst_bby->hat_size);
	}

	Degnome_free(bom_mom);
	Degnome_free(bad_dad);
	Degnome_free(tst_bby);

	gsl_rng_free(rng);

	printf("All tests for xdegnome completed\n");
}