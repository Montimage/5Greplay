/*
 * parse.c
 *
 *  Created on: 26 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <string.h>
#include "../src/lib/base.h"
#include "../src/lib/expression.h"
#include "../src/lib/mmt_log.h"
#include "../src/lib/mmt_alloc.h"

int main(){
	expression_t *expr;
	char *string = mmt_mem_alloc( 1000 ), *tmp;
	memcpy( string, "( (x.y.1 >= #fn(0,1) ) && ((a.z==b.y) || ((1 != 1) + (#strstr( #strlen(x.y.1) ) ) ) )", 1000 );

	parse_expression( &expr, string, 1000 );

	expr_stringify_expression( &tmp, expr );

	printf("stringify: %s", tmp );
	mmt_mem_free( tmp );
	expr_free_an_expression( expr, YES );
	mmt_mem_free( string );
	return 0;
}
