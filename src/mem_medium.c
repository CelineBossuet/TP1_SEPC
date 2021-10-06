/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"
#include <stdbool.h>


unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}


void * division_block_recursif(unsigned int taille_block)
{
    if (taille_block >= arena.medium_next_exponant + FIRST_ALLOC_MEDIUM_EXPOSANT){
        mem_realloc_medium();
        
    }
    void * ptr = arena.TZL[taille_block];
    if (ptr != NULL){ // On a trouvé un bloc à découper
        arena.TZL[taille_block] = *(void **)ptr; // On déréférence de la liste le bloc ainsi trouvé.
        return ptr; // On retourne le pointeur du bloc à découper 
    }else{
        void * block_a_decouper = division_block_recursif( taille_block + 1); // On a dans tous les cas un bloc à découper
        void * buddy = (void *)((uintptr_t)block_a_decouper ^ (1 << taille_block)); // On crée l'appariement
        arena.TZL[taille_block] = buddy;
        *(void **)buddy = 0; //On initialise la nouvelle liste.
        return block_a_decouper;
    }


}

void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    size +=32; //taille entière
    unsigned int taille_optimale = puiss2(size);
    void * ptr = division_block_recursif(taille_optimale);
    arena.TZL[taille_optimale] = *(void **)ptr;
    return mark_memarea_and_get_user_ptr(ptr, taille_optimale, MEDIUM_KIND);
}



/*
Par le même principe, on va récursivement remonter la pile des buddy jusqu'à ne plus en trouvé
*/

void remonter_buddy(void * ptr, unsigned int taille_block){
    void * buddy = (void *)((uintptr_t) ptr ^ (1 << taille_block));
    // On réaliste le parcours de la liste 
    void * traceur = arena.TZL[taille_block];
    void * sent = &traceur;
    while(traceur != NULL){
        if(*(void **)traceur == buddy){
            *(void **)traceur = *(void**)buddy;
            if(ptr > buddy){
                ptr = buddy;
            }
            return remonter_buddy(ptr, taille_block + 1);
        }
        traceur = *(void **) traceur;
    }
    traceur = sent;
    *(void **)ptr = arena.TZL[taille_block];
    arena.TZL[taille_block]=ptr;
}


void efree_medium(Alloc a) {
    unsigned int size_log = puiss2((unsigned long)a.size);
    remonter_buddy(a.ptr, size_log);
}


