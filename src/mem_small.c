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
        unsigned long taille = mem_realloc_small();
        void *ptr =arena.chunkpool; //adr du block
        for (int i=0; i<taille; i+=CHUNKSIZE){ //on découpe notre block en petits de taille CHUNKSIZE
            *(void **)ptr = (ptr + CHUNKSIZE); //on ecrit l'adresse du suivant 
            ptr+= CHUNKSIZE; //on avance au prochain Chunk

        }
        ptr-=CHUNKSIZE; //revient en arrière pour etre au dernier élement
        
        *(void **)ptr= 0; //point vers 0 la fin de liste
    }

    void * ptr_head = arena.chunkpool;
    void * ptr_mem = mark_memarea_and_get_user_ptr(ptr_head, CHUNKSIZE,SMALL_KIND);
    
    arena.chunkpool =*(void **)ptr_head;

    return (void *) ptr_mem;
}

/*
    Pour replacer le chunk décrit en A à la tête de la liste chainée, il suffit de faire pointer la tête sur 
    ce dernier.
*/

void efree_small(Alloc a) {
    void **adr=a.ptr;
    *adr=arena.chunkpool; //on écrit l'adresse du prochain chunk
    arena.chunkpool = a.ptr; //on mets le chink au début
}
