/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

/* Fonction permettant d'écrire le marquage dans les 16 premiers et les 16 derniers octets du bloc pointé par ptr et
d’une longueur de size octets. Elle renvoie l’adresse de la zone utilisable par l’utilisateur,
16 octets après ptr. */

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    /* ecrire votre code ici */
    unsigned long mark=knuth_mmix_one_round( (unsigned long)ptr);
    unsigned long magic = (mark & ~(0b11UL)) | k; 
    
    *(unsigned  long *)ptr = size;
    *(unsigned long*)(ptr +8)=magic;
    *(unsigned long*)(ptr + size -8)=size;
    *(unsigned long*)(ptr + size -16)=magic;

    return (void *)(ptr +16);
}

Alloc
mark_check_and_get_alloc(void *ptr)
{
    /* ecrire votre code ici */
    unsigned long taille1 = *(unsigned  long *)(ptr -16);
    unsigned long magic1 = *(unsigned  long *)(ptr -8);
    unsigned long taille2 = *(unsigned  long *)(ptr + taille1-24);
    unsigned long magic2 = *(unsigned  long *)(ptr + taille1 -32);
    assert(taille1==taille2);
    assert(magic1==magic2);
    
    Alloc a = {(void*)(ptr -16), magic1 & 0b11UL, taille1};
    return a;
}



unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1 << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
