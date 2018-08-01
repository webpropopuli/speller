// Implements a dictionary's functionality
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>

#include "dictionary.h"
#define DJM             // for debugging


typedef struct _DICT {
    int size;
    char *buf;
} DICT;

typedef struct _NODE {
    char *addr;
    struct _NODE *next;
} NODE;

typedef struct _HEAD {  // Head pointers for hash array
    NODE *pNode;
    int hasData;        // will be 0 until first use
} HEAD;

typedef unsigned hkey;

#define LOG printf ("fn:%s() ln:%d in %s\t",\
                      __func__, __LINE__, __FILE__);

#define WAIT {puts("<any key>"); getchar();}
//2^16	65536
//2^17	131072
//2^18	262144
#define HASHSIZE 262144 // 143477, 105557   // size of hash table. This is a prime for mathy reasons beyond the scope of this problem and my brain
#define HASHMASK 262143                    // turn all bit on 1 less than HASHSIZE (which MUST BE A Power of 2)

DICT dict;              // The Dictionary we build

HEAD HASH[HASHSIZE];    // The Hash Table we use. An array of singly linked lists

// some crappy hash fn found on interwebs. This is secondary though an optimal fn is sorta important
inline static unsigned int GetHashKey(const char* word)
{
register unsigned long hash = 0;
register int c;

        while ((c = *word++))
            hash = c + (hash << 6) + (hash << 16) - hash;

#if 0
    return (hash % HASHSIZE);
#else
    return (hash & HASHMASK); // this should be faster though smart compiler prolly means irrelevant
#endif
}


/***                    CreateNode()
 *  allocate and init a node, set data to str; next = NULL
 *  Returns NODE * or NULL on error
 */
NODE* CreateNode(char *str)
{
    NODE* np = (NODE*)malloc(sizeof(NODE));
    if(np == NULL)
    {
        LOG printf("malloc error in CreateNode()\n");
        return NULL;
    }
    np->addr = str;
    np->next = NULL;

    return np;
}



/*****
 * *                    check()
 * * */
// Returns true if word found in dictionary else false
bool check(const char *word)
{
register char* p;
register NODE *np;
static char temp[LENGTH];  // a reusable area to munge the input word to lower

// make a copy and lowercase it
strcpy(temp, word);
p = temp;
for (; *p; ++p)
    //if(*p < 'a')
        *p |= 32;   // this is a bitmask. Go look at the ascii table...


unsigned int key = GetHashKey(temp);
//printf("%u|", key);

if( 0 == (&HASH[key])->hasData)     // ...never assigned anything to this row
    return false;

np = (&HASH[key])->pNode;

while(np /*!= NULL*/)    // this list is not empty
    {
    if(temp[0] == *np->addr)        //hacking strcmp- don't call func it first chars don't match
        if (0 == strcmp(temp, np->addr))
            {
            return true;    // found
            }
    np = np->next;
    }

return false;
}

/*****
 * *                    load()
 * * */
// Loads dictionary into memory, returning true if successful else false
bool load(const char *dictionary)
{
dict.size = 0;

memset(HASH, 0, HASHSIZE*sizeof(HEAD));  //InitHashtable();

//LOG printf("->Opening dict %s\n", dictionary);

FILE *fileDict = fopen(dictionary, "r");
if(fileDict == NULL)
    {
    LOG puts("Can't fopen(dictionary file)\n");
    return false;
    }

// Get length of dictionary file
fseek(fileDict, 0L, SEEK_END);    /* Position to end of file */
long lFileLen = ftell(fileDict);  /* Get file length */
rewind(fileDict);                 /* Back to start of file */

/* Read the entire dictionary file into memory */
dict.buf = calloc(lFileLen + 1, sizeof(char));
//DJM printf("Inputfilesize is %ld\n", lFileLen);

if(dict.buf == NULL )
    {
    LOG printf("\nInsufficient memory to read file.\n");
    return false;
    }

fread(dict.buf, lFileLen, 1, fileDict); /* Read the entire file into DICT */

// Now we're working from memory...

char *pMem=dict.buf;    // ptr into to DICT text
char *endMem = dict.buf + lFileLen;  // ptr to end of buffer
HEAD * pHEAD;

char *strStart;
do
{
    if(!(*pMem))
    {
        pMem++;
        continue;
    }

    strStart = pMem;    // save the begin of str in DICT

    // find end of string so we can null term it
    while(*pMem > 0x0d) // input strings break at newlines
    {
         pMem++;
    }
    *(pMem) = 0;  // null terminate both
    pMem++;

    /* Create it, insert it and update count
    */
    if(*strStart) // not NULL
        {
        unsigned int key = GetHashKey(strStart);
        pHEAD = &HASH[key];

        NODE *pNode = CreateNode(strStart);

        // INSERT node at HEAD position
        pNode->next = pHEAD->pNode;     // new->next to current head
        pHEAD->pNode = pNode;           // then HEAD is the new guy
        pHEAD->hasData++;
        dict.size++;
        }
} while(pMem < endMem);   // while < endBuf

// Cleanup
fclose (fileDict);

//LOG printf("load() read %i lines from %s\n", dict.size, dictionary);


return true;
}

/*****
 * *                    size()
 * *
 * * Returns number of words in dictionary if loaded else 0 if not yet loaded
 * */
inline unsigned int size(void)
{
return dict.size;
}



/*****
 * *                    unload()
 * * */
// Unloads dictionary from memory, returning true if successful else false
bool unload(void)
{
NODE *p, *tmp;

    // Walk each list and unload from the end
    for(int x=0; x<HASHSIZE; x++)
    {
    if(HASH[x].hasData == 0)
        continue;
    else
        {
        HEAD *pH = &HASH[x];

        p = pH->pNode;

        while (NULL != p)
            {
            tmp = p;
            p = p->next;
            free(tmp);
            }
        }
    }

    free(dict.buf);
    dict.buf = NULL;

return true;
}
