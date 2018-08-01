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
    char *pWord;
    struct _NODE *next;
} NODE;

typedef struct _HEAD {  // Head pointers for hash array
    //int cnt;
    NODE *pNode;
} HEAD;

typedef int hkey;

#define LOG printf ("fn:%s() ln:%d in %s\t",\
                      __func__, __LINE__, __FILE__);

#define WAIT {puts("<any key>"); getchar();}

#define HASHSIZE 200000 //143477 105557   // size of hash table. This is a prime for mathy reasons beyond the
                        // scope of this problem and my brain. 7919 is the 1000th prime fwiw.

DICT dict;              // The Dictionary we build

HEAD HASH[HASHSIZE];    // The Hash Table we use. An array of singly linked lists


/*****
 * *                    InitHashTable()
 * * */
void InitHashtable(void)
{
    HEAD *p = &HASH[0];

    for(int x=0; x<HASHSIZE; x++)
    {
        p->pNode = NULL;
        //p->cnt = 0;
    }
}

// The Hash func
hkey GetHashKey(const char* word)
{
   unsigned int length = strlen(word);
   unsigned int hash = 1315423911;
   unsigned int i    = 0;

  for (i = 0; i < length; ++word, ++i)
  {
     hash ^= ((hash << 5) + (*word) + (hash >> 2));
  }

    return (hkey)(hash % HASHSIZE);
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
    np->pWord = str;
    np->next = NULL;

    return np;
}

/***                    LookupNode()
 *find a node
 * return NODE * if found, else NULL
 */
bool LookupNode(char *s, HEAD *pHEAD)
{
    if(pHEAD->pNode == NULL)    // this list is empty
        return false;

    NODE *np = pHEAD->pNode;

    while(np != NULL)
        {
        if (0 == strcmp(s, np->pWord))
          return true;    /* found */
        np = np->next;
        }
    return false;        /* not found */
}


/***                    InsertNode()
 * Insert node at head
 *  new->next = Head
 *  replace HEAD with new
 */
bool InsertNode(NODE *pNode, HEAD *pHEAD)
{
    pNode->next = pHEAD->pNode;
    pHEAD->pNode = pNode;
    //pHEAD->cnt++;
    return true;
}

/*****
 * *                    check()
 * * */
// Returns true if word found in dictionary else false
bool check(const char *word)
{
static char* p;
static hkey key;
static HEAD *pHEAD;
static NODE *np = NULL;

char *tmp = malloc(strlen(word) + 1);

// make a copy and lowercase it
tmp = malloc(strlen(word)*sizeof(char));
p = tmp;

for (; *p; ++p) *p = tolower(*p);

key = GetHashKey(tmp);

pHEAD = &HASH[key];

if(pHEAD->pNode == NULL)    //empty
    return false;

bool bFound = false;

np = pHEAD->pNode;

while(np != NULL)
    {
    if (0 == strcmp(tmp, np->pWord))
        {
        bFound = true; // found
        break;
        }
    else
        np = np->next;
    }

free(tmp);
return bFound;
}

/*****
 * *                    load()
 * * */
// Loads dictionary into memory, returning true if successful else false
bool load(const char *dictionary)
{
dict.size = 0;
//InitHashtable();
memset(HASH, 0, HASHSIZE*sizeof(HEAD));

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

char *strStart;
do
{
    strStart = pMem;    // save the begin of str in DICT

    // find end of string so we can null term it
    while(*pMem > 0x0d) // input strings break at newlines
    {
         pMem++;
    }
    *(pMem++) = 0;  // null terminate both

    hkey key = GetHashKey(strStart);
    HEAD *pHEAD = &HASH[key];

    // if not in list then create it and update count
//DJM printf("Found Dict word %s(keyval %d)\n", strStart, (int)key);
    if(strlen(strStart))
        {
        NODE *pNode = CreateNode(strStart);
        //assert (pNode);

        if (InsertNode(pNode, pHEAD))
            dict.size++;
        }
} while(pMem < endMem);   // while < endBuf

// Cleanup
fclose (fileDict);

//LOG printf("load() read %i lines from %s\n", dict.size, dictionary);

/*
DJM // dump the hash table
for(int z = 0; z<HASHSIZE; z++)
{
    NODE *p = HASH[z].pNode;

    //printf("\nHASH[%d] %d elements\n", z, HASH[z].cnt);

    if(p != NULL)
        do
        {
            printf("\t[%s]", p->pWord);
            p = p->next;
        }
        while (p != NULL);
}
DJM WAIT
*/

return true;
}

/*****
 * *                    size()
 * * */
// Returns number of words in dictionary if loaded else 0 if not yet loaded
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

    /* Walk each list and
            save next,
            unload current,
            repeat while ptr != NULL
    */
    for(int x=0; x<HASHSIZE; x++)
    {
//DJM printf("Freeing HASH[%d]\t", x);
        HEAD *pH = &HASH[x];

        p = pH->pNode;

        while (NULL != p)
            {
                tmp = p;
                p = p->next;
                free(tmp);
            }
    }

    free(dict.buf);
    dict.buf = NULL;

return true;
}
