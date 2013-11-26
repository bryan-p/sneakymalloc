/*
 * This library includes two functions that pull memory from a
 * given memory address by intercepting calls to malloc and free. 
 *
 * The data is stored in memory until program termination, at which time
 * the data is written to a file on disk.  The name of the file is 
 *
 * memspy.<pid>.dat 
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

#define __USE_GNU
#define RTLD_NEXT ((void *) -1l)

#define MAX_FILENAME_LENGTH   24

void writetofile();

/* structure to hold data from memory obtained by calls to malloc and free */
struct spy_list_t{

   void *data;     /* holds the stolen memory */
   char *address;  /* memory location to pull data from */
   int ispulled;     /* used to see if memory at this location has been freed */
   int size;       /* size of the memory allocation */
   struct spy_list_t *next; /* next node in the list */
};

/* structs for the head and tail of list */
static struct spy_list_t *spylisthead = NULL;
static struct spy_list_t *spylisttail = NULL;

/* 
 * This function is my implemtation of malloc(), which obtains the memory address
 * returned from the true library function, malloc(), and stores it in a list.
 */
void *malloc(size_t size){

   void *ptr = NULL; /* used to hold return value of the real malloc function */
   static void *(*real_malloc)(size_t); /* pointer to function malloc() in memory */
   struct spy_list_t *current; /* the node to be added to the list */

   /* get pointer to address of real malloc function in memory */   
   if(!real_malloc)
      real_malloc = (void * (*) (size_t)) dlsym(RTLD_NEXT, "malloc");
   
   /* get the value from the real malloc that will be passed back to calling function */
   ptr = real_malloc(size);

   /* need to use the real malloc so it doesn't call this malloc */
   current = real_malloc(sizeof(struct spy_list_t));
   
   current->data = calloc(size, sizeof(char));
   
   /* set variables for spy_list struct */
   current->address = ptr; 
   current->ispulled = 0; 
   current->size = size;
   current->next = NULL;

   /* add to the list */
   /* **************************** */
   if(spylisthead == NULL)
      spylisthead = current;

   else
      spylisttail->next = current;
   
   spylisttail = current;
   /* *************************** */
  
   return ptr; 
}

/*
 * This function has the same signature as the stdlib free() but 'steals' memory
 * from the address in memory of any call that has been made to malloc
 */
void free(void *ptr){

   static void (*real_free)(void *); /* function pointer for the real free function */
   struct spy_list_t *current; 

   /* get the pointer to the real free function in memory */
   if(!real_free){
      real_free = (void * (*) (void *)) dlsym(RTLD_NEXT, "free");
 
      /* registers shutdown_hook to call writetofile() on exit */   
      atexit(writetofile);
   }

   current = spylisthead;

   /* loop through the list of memory and steal contents, but only if no memory has been stolen
    * from that location yet */ 
   while( current != NULL){

     /* check to see if memory has been stolen from here yet */
     if(current->ispulled == 0){
          
        memcpy(current->data, current->address, current->size);

        current->ispulled = 1;
      }

      current = current->next;
   }
 
   /* call the real free */
   real_free(ptr);

} 
   
/*
 * This function writes the contents of the saved data in memory
 * collected by free to a file named memspy.<pid>
 */
void writetofile(){

   FILE *fp = NULL; /* file pointer */
   char file_base[] = "/tmp/memspy."; /* used for the base name */
   char file_name[MAX_FILENAME_LENGTH]; /* used to hold the pid part of the file name */
   struct spy_list_t *current; 

   /* append pid to the file name */
   snprintf(file_name, MAX_FILENAME_LENGTH, "%s%d.dat",  file_base, getpid());

   /* open the file in append mode */
   if ( (fp = fopen(file_name, "a")) == NULL)
      return;

   current = spylisthead;

   /* go through list writing contents to the file */
   while(current != NULL){
      fwrite(current->data, 1, current->size, fp);
      current = current->next;
   }

   fclose(fp);
}   
