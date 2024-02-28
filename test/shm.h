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

//  ---------------------------------------
//  shm_mgr_t| bucket_size| bucket_size|...
//  ---------------------------------------

typedef struct {
    int bucket_size;
    int bucketn;
    char name[128];

    sem_t sem;

    int num;
    int r;
    int w;
} shm_mgr_t;

/// @brief: free a shm object
void sys_shm_serv_free( shm_mgr_t * ctx);
/// @breif: create a shm server
//  @name: string 
//  @bucket_size: space of shm data bucket
//  @bucketn: number of shm data bucket
//  @return: pointer of shm mgr
shm_mgr_t * sys_shm_serv_create( char * name, int bucket_size, int bucketn );
/// @brief: push data into shm
//  @ctx: return by [sys_shm_serv_create]
//  @data: user data that need to be push
//  @datan: length of data, need equal to bucketn in [sys_shm_serv_create]
//  @return: 0:success -1:failed 1:in progress, need wait and retry
int sys_shm_serv_push( shm_mgr_t * ctx, char * data, int datan );

/// @brief: connect to a shm server.
//  @name: same as [sys_shm_serv_create]
//  @bucket_size: same as [sys_shm_serv_create]
//  @bucketn: same as [sys_shm_serv_create]
shm_mgr_t * sys_shm_cli_connect( char * name, int bucket_size, int bucketn);
/// @brief: pull data form shm
//  @ctx: return by [sys_shm_cli_connect]
//  @data: addr for stroge pull data
//  @datan: space of data, need equal to bucketn in [sys_shm_cli_connect]
//  @return: 0:success -1:failed  1:in progress, need wait and retry
int sys_shm_cli_pull( shm_mgr_t * ctx, char * data, int datan );


