#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "shm.h"


int main( int argc, char ** argv)
{
	/// parents
        shm_mgr_t * ctx = sys_shm_serv_create("my_shm", 1920*1080, 5 );
        if( ctx == NULL ) {
                printf("shm create failed\n");
                return -1;
        }
	printf("shm serv fin\n");

	int rc = fork();
	if( rc < 0 ) {
		printf("fork child failed\n");
		return -1;
	} else if ( rc == 0 ) {
		/// child

		shm_mgr_t * ctx_child = sys_shm_cli_connect( "my_shm", 1920*1080, 5 );
		if( ctx_child == NULL ) {
			printf("shm connect failed\n");
			return -1;
		}
		printf("shm connect fin\n");
		sleep(1);
		
		char * addr = malloc(1920*1080);
		int i = 0;
		while( i < 10 ) {

			int pull_rc = sys_shm_cli_pull( ctx_child, addr, 1920*1080 );
			if( pull_rc == -1 ) {
				printf("shm pull error\n");
			} else if ( pull_rc == 0 ) {
				printf("shm pull. [%d]\n", i );
			} else if ( pull_rc == 1 ) {
				usleep(1000*50);
				continue;
			}	
			i++;
		}
		free(addr);
	} else if ( rc > 0 ) {
		/// parents
		
		char * addr = malloc( 1920*1080 );
		int i = 0;
		while(i < 10) {
			
			int push_rc = sys_shm_serv_push( ctx, addr, 1920*1080 );
			if( push_rc == -1 ) {
				printf("shm push error.\n");
				break;
			} else if( push_rc == 0 ) {
				printf("shm push. [%d]\n", i);
			} else if ( push_rc == 1 ) {
				usleep(1000*50);
				continue;
			}
			i++;
		}
		free(addr);

		sleep(2);
		sys_shm_serv_free(ctx);
	}
	return 0;
}
