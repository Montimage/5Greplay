/*
 * main_gen_plugin.c
 *
 *  Created on: 26 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Parse rules in .xml file, then generate .c file, then compile to a plugin .so file.
 */

#include <mmt_core.h>

void _attributes_iterator(attribute_metadata_t * attribute, uint32_t proto_id, void * args) {
	printf("\t- Attribute id %i, \t Name %s \n", attribute->id, attribute->alias);
}

void _protocols_iterator(uint32_t proto_id, void * args) {
	printf("Protocol id %i \t Name %s\n", proto_id, get_protocol_name_by_id(proto_id));
	iterate_through_protocol_attributes(proto_id, _attributes_iterator, NULL);
}

int list( int argc, char** argv ){
	//init mmt_dpi extraction to load list of avaiable protocols
	init_extraction();
	iterate_through_protocols( _protocols_iterator, NULL );
	close_extraction();// close mmt_dpi
	return 0;
}
