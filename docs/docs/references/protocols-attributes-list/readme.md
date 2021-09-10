# List of protocols and their attributes

The list of protocols and their attributes are given in [protocols-attributes-list.txt](protocols-attributes-list.txt).

We can also get this list by running `./5greplay list`.

For example:

```
Protocol id 904 	 Name nas_5g
	- Attribute id 1, 	 Name protocol_discriminator 
	- Attribute id 2, 	 Name message_type 
	- Attribute id 3, 	 Name security_type 
	- Attribute id 4, 	 Name procedure_transaction_id 
	- Attribute id 5, 	 Name message_authentication_code 
	- Attribute id 6, 	 Name sequence_number 
	- Attribute id 4096, 	 Name p_hdr 
	- Attribute id 4097, 	 Name p_data 
	- Attribute id 4098, 	 Name p_payload 
	- Attribute id 4099, 	 Name packet_count 
	- Attribute id 4100, 	 Name data_count 
	- Attribute id 4101, 	 Name payload_count 
	- Attribute id 4102, 	 Name first_packet_time 
	- Attribute id 4103, 	 Name last_packet_time 
	- Attribute id 4104, 	 Name stats 
```

Protocol `nas_5g` having 6 *main* attributes and 9 *metadata* attributes which are used internally by DPI engine of 5Greplay.
We can use `nas_5g.sequence_number` in rules to reference to sequence number attribute of NAS protocol.

# 5G protocols and attributes

We list below a short group of attributes focused on 5G networks, 
on wich 5Greplay can parse the  NGAP, NAS-5G and DIAMETER protocols.



```
meta.direction
meta.args
meta.utime
meta.packet_len
meta.proto_hierarchy
meta.classified
meta.probe_id
meta.source_id
meta.p_hdr
meta.p_data
meta.p_payload
meta.packet_count
meta.data_count
meta.payload_count
meta.first_packet_time
meta.last_packet_time
meta.stats
ethernet.proto
ethernet.dst
ethernet.src
ethernet.p_hdr
ethernet.p_data
ethernet.p_payload
ethernet.packet_count
ethernet.data_count
ethernet.payload_count
ethernet.first_packet_time
ethernet.last_packet_time
ethernet.stats
ip.version
ip.header_len
ip.proto_tos
ip.tot_len
ip.identification
ip.rf_flag
ip.df_flag
ip.mf_flag
ip.frag_offset
ip.proto_ttl
ip.proto_id
ip.checksum
ip.src
ip.dst
ip.client_addr
ip.server_addr
ip.client_port
ip.server_port
ip.ip_frag_packets_count
ip.ip_frag_data_volume
ip.ip_df_packets_count
ip.ip_df_data_volume
ip.ip_session_count
ip.ip_active_session_count
ip.ip_timedout_session_count
ip.p_hdr
ip.p_data
ip.p_payload
ip.packet_count
ip.data_count
ip.payload_count
ip.first_packet_time
ip.last_packet_time
ip.stats
ip.session_id
sctp.src_port
sctp.dest_port
sctp.verif_tag
sctp.checksum
sctp.ch_type
sctp.ch_flags
sctp.ch_length
sctp.p_hdr
sctp.p_data
sctp.p_payload
sctp.packet_count
sctp.data_count
sctp.payload_count
sctp.first_packet_time
sctp.last_packet_time
sctp.stats
sctp_data.ch_type
sctp_data.ch_flags
sctp_data.ch_length
sctp_data.data_tsn
sctp_data.data_stream
sctp_data.data_ssn
sctp_data.data_ppid
sctp_data.p_hdr
sctp_data.p_data
sctp_data.p_payload
sctp_data.packet_count
sctp_data.data_count
sctp_data.payload_count
sctp_data.first_packet_time
sctp_data.last_packet_time
sctp_data.stats
ngap.procedure_code
ngap.pdu_present
ngap.amf_ue_id
ngap.ran_ue_id
ngap.p_hdr
ngap.p_data
ngap.p_payload
ngap.packet_count
ngap.data_count
ngap.payload_count
ngap.first_packet_time
ngap.last_packet_time
ngap.stats
nas_5g.protocol_discriminator
nas_5g.message_type
nas_5g.security_type
nas_5g.procedure_transaction_id
nas_5g.message_authentication_code
nas_5g.sequence_number
nas_5g.p_hdr
nas_5g.p_data
nas_5g.p_payload
nas_5g.packet_count
nas_5g.data_count
nas_5g.payload_count
nas_5g.first_packet_time
nas_5g.last_packet_time
nas_5g.stats
diameter.version
diameter.message_length
diameter.flag_r
diameter.flag_p
diameter.flag_e
diameter.flag_t
diameter.command_code
diameter.application_id
diameter.hop_to_hop_id
diameter.end_to_end_id
diameter.p_hdr
diameter.p_data
diameter.p_payload
diameter.packet_count
diameter.data_count
diameter.payload_count
diameter.first_packet_time
diameter.last_packet_time
diameter.stats
```
