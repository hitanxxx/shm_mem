/// Shared memory support for communication between different processes,
/// support for linux platforms. Support for running multiple instances at the same time
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <semaphore.h> 

typedef void shm_ctx_t;

/// @brief: free a shm object
void sys_shm_serv_free( shm_ctx_t* ctx );
shm_ctx_t * sys_shm_serv_create( char * name, int bucket_size, int bucketn );
int sys_shm_serv_push( shm_ctx_t * ctx, char * data, int datan );

shm_ctx_t * sys_shm_cli_connect( char * name, int bucket_size, int bucketn);
int sys_shm_cli_pull( shm_ctx_t * ctx, char * data, int datan );


