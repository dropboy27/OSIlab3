#include <stdlib.h>
#include "carlo.h"

unsigned long long carlo_alg(unsigned int seed, unsigned long long rounds){
    int cards[52];
    unsigned long long success = 0;
    unsigned int state = seed; 
    for(unsigned long long j =0; j < rounds;j++){
        for(int i =0; i <52; i++){
            cards[i]=i;
        }
        for(int k = 51; k >=0; k--){
            int temp = rand_r(&state) % (k + 1);
            int temp_card = cards[temp];
            cards[temp] = cards[k];
            cards[k] = temp_card;
        }
        if (cards[0]%13 == cards[1]%13){
            success++;
        }
    }
    return success;
}