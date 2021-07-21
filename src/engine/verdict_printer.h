/*
 * print_verdict.h
 *
 *  Created on: Dec 9, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_VERDICT_PRINTER_H_
#define SRC_LIB_VERDICT_PRINTER_H_


void verdict_printer_init( const char *file_string, int interval );

void verdict_printer_send( const char* msg );

void verdict_printer_free( );

#endif /* SRC_LIB_VERDICT_PRINTER_H_ */
