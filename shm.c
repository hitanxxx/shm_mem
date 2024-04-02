/// module for create a share memory in system. 
/// for different porcess to exchange data
#include "shm.h"

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

void sys_shm_serv_free( void * p )
{
    shm_mgr_t * ctx = (shm_mgr_t*)p;
    sem_destroy(&ctx->sem);
    shm_unlink(ctx->name);
    return;
}

/// @breif create a shm pool
void * sys_shm_serv_create( char * name, int bucket_size, int bucketn )
{
	assert(strlen(name) > 0);
	assert(bucket_size > 0);
	assert(bucketn > 0);

	shm_mgr_t * ctx = NULL;

	int ret = -1;
	int fd = shm_open( name, O_CREAT|O_RDWR, 0666 );
	if( fd == -1 ) {
		printf("[%s]. shm open failed. [%d] [%s]\n", __func__, errno, strerror(errno) );
		return NULL;
	}

	do {
		/*	format 
		 *	---------------------------------------
		 *	shm_mgr_t| bucket_size| bucket_size|...
		 *	--------------------------------------
		 * */

		int wantn = sizeof(shm_mgr_t) + (bucket_size * bucketn);
    		if( -1 == ftruncate(fd, wantn) ) {
			printf("[%s]. shm ftruncate failed. [%d]\n", __func__, errno );
			break;
		}
    		ctx = mmap( NULL, wantn, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0 );
    		if( ctx == MAP_FAILED ) {
        		printf("[%s]. shm mmap failed. [%d]\n", __func__, errno );
        		break;
    		}
		if( 0 != sem_init( &ctx->sem, 1, 1 ) ) {
                        printf("[%s]. shm semaphores init failed\n", __func__ );
                        break;
                }

		/// cache config 
		ctx->bucketn = bucketn;
        	ctx->bucket_size = bucket_size;
        	memset(ctx->name, 0, sizeof(ctx->name));
        	strncpy( ctx->name, name, sizeof(ctx->name)-1 );
		/// circular queue mgr data
		ctx->r = ctx->w = ctx->num = 0;

		ret = 0;
	} while(0);
	
	return ( ret == 0 ? ctx : NULL );
}

/// @breif push data into shm pool
int sys_shm_serv_push( void * p, char * data, int datan )
{
    shm_mgr_t * ctx = (shm_mgr_t*)p;
    assert(data != NULL);
    assert(datan == ctx->bucket_size);

    int ret = -1;

    do {
	int rc = 0;
        do {
            rc = sem_trywait(&ctx->sem);
	    if( rc == -1 ) {
		if(errno == EAGAIN) {
			printf("[%s]. shm in progress, need wait and retry...\n", __func__ );
			return 1;
		} else {
			printf("[%s]. shm trylock failed. [%d]\n", __func__, errno );
			return -1;
		}	
	    }
        } while( rc == -1);
    
	if(0) {
        	if( ctx->num == ctx->bucketn ) {
                	printf("[%s]. shm full.\n", __func__ );
                	break;
        	}
	} else {
		if( ctx->num == ctx->bucketn ) {
			printf("[%s]. shm del oldest object, need to pull faster\n", __func__);
			ctx->r = (ctx->r+1)%ctx->bucketn;
			ctx->num -= 1;
		}	
	}

	/// push data 
	char * addr = (char*)ctx;
	addr += sizeof(shm_mgr_t);
	memcpy( (addr + (ctx->w*ctx->bucket_size)), data, datan );
	ctx->w = (ctx->w+1)%ctx->bucketn;
	ctx->num += 1;	

        ret = 0;
    } while(0);
    sem_post(&ctx->sem);

    return ret;
}

/// @breif connect to a exist shm pool
void * sys_shm_cli_connect( char * name, int bucket_size, int bucketn)
{
	assert(strlen(name) > 0);
	assert(bucket_size > 0);
	assert(bucketn > 0);

	shm_mgr_t * ctx = NULL;

	int fd = shm_open( name, O_RDWR, 0666 );
	if( fd <= 0 ) {
		printf("[%s]. shm open failed. [%d]\n", __func__, errno );
		return NULL;
	}

   	int wantn = sizeof(shm_mgr_t) + (bucket_size * bucketn);
	ctx = mmap( NULL, wantn, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 );
	if( ctx == MAP_FAILED ) {
    		printf("[%s]. shm mmap failed. [%d]\n", __func__, errno );
    		return NULL;
	}
    	return ctx;
}


int sys_shm_cli_pull( void * p, char * data, int datan )
{
    shm_mgr_t * ctx = (shm_mgr_t*)p;
    assert(data != NULL);
    assert(datan == ctx->bucket_size);

    int ret = -1;

    do {
	int rc = 0;
        do {
            int rc = sem_trywait(&ctx->sem);
            if( rc == -1 ) {
                if(errno == EAGAIN) {
                        printf("[%s]. shm in progress, need wait and retry...\n", __func__ );
			return 1;
                } else {
                        printf("[%s]. shm trylock failed. [%d]\n", __func__, errno );
                        return -1;
                }
            }
        } while( rc == -1);

        if( ctx->num == 0 ) {
    		printf("[%s]. shm empty.\n", __func__ );
    		break;
    	}

	/// pull data
	char * addr = (char*)ctx;
	addr += sizeof(shm_mgr_t);
	memcpy( data, (addr + (ctx->r*ctx->bucket_size)), datan );
        ctx->r = (ctx->r+1)%ctx->bucketn;	
	ctx->num -= 1;

        ret = 0;
    } while(0);
    sem_post(&ctx->sem);

    return ret;
}


