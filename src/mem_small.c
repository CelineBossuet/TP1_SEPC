/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"


/*
Le principe de cette fonction est tout d'abord de bien vérifier qu'il y a quelquechose à allouer. D'où la 
première condition. Sinon, on fait pointer deux pointeurs sur la zone mémoire puis sur le début de la mémoire marquée
tout en sautant au prochain chunk. On renvoie ainsi grâce à la fonction un pointeur sur la première case mémoire où
l'utilisateur peut écrire.

*/

void *
emalloc_small(unsigned long size)
{
    if (arena.chunkpool == NULL){
        mem_realloc_small();
    }

    void * ptr_head = arena.chunkpool;
    void * ptr_mem = mark_memarea_and_get_user_ptr(ptr_head, CHUNKSIZE,SMALL_KIND);
    arena.chunkpool += CHUNKSIZE;

    return (void *) ptr_mem;
}

/*
    Pour replacer le chunk décrit en A à la tête de la liste chainée, il suffit de faire pointer la tête sur 
    ce dernier.
*/

void efree_small(Alloc a) {
    arena.chunkpool = a.ptr;
}
