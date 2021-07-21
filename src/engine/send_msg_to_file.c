#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <pthread.h>

#include "../lib/mmt_lib.h"

#define MAX_FILE_NAME_LEN 5000

static FILE *file       = NULL;
static char file_name[ MAX_FILE_NAME_LEN ] = {0};
static uint16_t period  = 5;   //period to create a new file
static time_t timestamp = 0;	 //the moment the current file was created
static pthread_mutex_t mutex_lock;

static void (*send_message_fn)(const char *) = NULL;

void send_message_to_file( const char *msg ){
	send_message_fn( msg );
}

static inline void _close_file( FILE *f ){

	int ret = fclose( f );
	if( unlikely( ret == EOF ) )
		log_write( LOG_ERR,"Proc %d: Cannot close file %p. %s", getpid(), f, strerror( errno ) );
}

static inline void _close_current_file_and_create_semaphore(){

	char str[ MAX_FILE_NAME_LEN + 1] = {0};
	int len;
	if( file != NULL ){
		//close .csv file
		_close_file( file );

		//create semaphore for the current file
		len = snprintf( str, MAX_FILE_NAME_LEN, "%sreport-%ld-%d.csv.sem", file_name, timestamp, (int)getpid() );
		str[ len ] = '\0';
		file = fopen( str, "w" );
		if( unlikely( file == NULL) )
			log_write( LOG_ERR, "%d creation of \"%s\" failed: %s\n" , errno , str , strerror( errno ) );
		else
			_close_file( file );

		file = NULL;
	}
}


static inline void send_message_to_single_file( const char * message ) {
	int ret = fprintf( file, "%s\n", message );
	if( unlikely( ret < 0 ) )
		log_write( LOG_ERR,"Error when writing data to file %p: %s", file, strerror( errno ) );
}

static inline void send_message_to_sampled_file( const char * message ) {
	char str[ MAX_FILE_NAME_LEN + 1];
	int len, ret;
	time_t now = time( 0 );

	//lock
	ret = pthread_mutex_lock( &mutex_lock );
	if( ret != 0 ){
		log_write( LOG_ERR,"Error %d: Cannot lock file while writing", ret );
		return;
	}

	//create a new file if need
	if( now - timestamp >= period || file == NULL ){
		//close the current file if it is opening
		_close_current_file_and_create_semaphore();

		//create new csv file
		timestamp = now;

		len = snprintf( str, MAX_FILE_NAME_LEN, "%sreport-%ld-%d.csv", file_name, timestamp, (int)getpid() );
		str[ len ] = '\0';

		file = fopen( str, "w" );
		if( unlikely( file == NULL) )
			ABORT( "%d creation of \"%s\" failed: %s\n" , errno , str , strerror( errno ) );
	}

	send_message_to_single_file( message );

	//unlock
	while( pthread_mutex_unlock( &mutex_lock) != 0 );
}


/**
 * args: /home/toto/data/:5
 */
void init_file(const char *filename, int period_sample ) {
	char str[ MAX_FILE_NAME_LEN ] = {0};
	int len = 0, ret;

	strcpy( file_name, filename );
	period = period_sample;

	ret = pthread_mutex_init( &mutex_lock, NULL );
	switch( ret ){
	case EAGAIN:
		ABORT("Cannot initialize mutex. All kernel synchronization objects are in use.");
		break;
	case EBUSY:
		ABORT("Cannot initialize mutex.The given mutex was previously initialized and hasn't been destroyed.");
		break;
	case EFAULT:
		ABORT("Cannot initialize mutex.A fault occurred when the kernel tried to access mutex or attr.");
	}

	if( period == 0 ){
		len = snprintf( str, MAX_FILE_NAME_LEN, "%smmt-5greplay-%d.csv", file_name, (int)getpid() );
		str[ len ] = '\0';

		file = fopen( str, "w" );
		if( unlikely( file == NULL) )
			ABORT( "%d creation of \"%s\" failed: %s\n" , errno , str, strerror( errno ) );

		send_message_fn = send_message_to_single_file;
	}else
		send_message_fn = send_message_to_sampled_file;
}


void close_file(){
	if( period != 0 )
		_close_current_file_and_create_semaphore();
	else if( file != NULL )
		_close_file( file );

	file = NULL;
	pthread_mutex_destroy( &mutex_lock );
}


//end report message
