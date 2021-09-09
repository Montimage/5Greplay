# Add or remove rules at runtime

This feature is developping and coming soon ...

## 1. Add new rules
The new rules in XML format must be compiled (e.g., using `compile` command) to obtain the rules in .so format.
The compiled .so files must be then moved to rule folder  `./rules`.
Subsequently, mmt-security works only on these .so files.


The functions used in the following procedures should be called from the main thread (that is the one it calls `mmt_sec_init`).

### 1.1 Register the new rules to mmt-security
To inform mmt-security to take into account the new rules, call the following function

```C
size_t mmt_sec_add_rules( const char *rules_mask )
```

- `rules_mask` parameter is a string wrt the [rules mask](./rules_mask.md) syntax. It distributes the new rules into the existing threads. 
Currently, this function does not allow to create a new thread to process new rules.

-  The function returns number of new rules to be added. A new rule is not added if one of the following conditions is met:

    - it has the same ID with the one of a rule being verified.
    - it is assigned to a non-existing thread.
    
If the current execution contains only a single mmt-security thread, the new rules will be added to this thread for any `thead_id` existing in `rules_mask`.

### 1.2 Register to MMT-DPI the new protocols/attributes to be extracted
The new rules may use protocols/attributes that never been used by the existing rules.
Consequently one need to register to MMT-DPI these new protocols/attributes to be able to extract their data.

The following funtion allows to retire a list of unique attributes of protocols that are currently being used by mmt-security:

```C
size_t mmt_sec_get_unique_protocol_attributes( proto_attribute_t const*const** proto_atts_array );
```

For example:

```C
size proto_atts_count, i;
proto_attribute_t const*const* proto_atts;
//get list of unique proto_attr
proto_atts_count = mmt_sec_get_unique_protocol_attributes( & proto_atts );
//visite the list
for( i=0; i<proto_atts_count; i++ )
   DEBUG( "Attribute: %s.%s (%d.%d)",
         proto_atts[i]->proto, proto_atts[i]->att,
         proto_atts[i]->proto_id, proto_atts[i]->att_id );
```

## 2. Remove rules

To remove rules from mmt-security, use the following function:

```C
size_t mmt_sec_remove_rules( size_t rules_count, const uint32_t* rules_id_set );
```

- `rules_count` is number of elements of `rules_id_set`
- `rules_id_set` is an array of rules IDs
- the function returns number of rules being removed.
A rule is not removed if it is not being verified by mmt-security.

For example:

```C
uint32_t rm_rules_arr[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
size_t count = mmt_sec_remove_rules(20, rm_rules_arr );
DEBUG("Removed %zu rules", count);
```

After removing rules, some protocols/attributes may not be needed. One can unregister them from MMT-DPI to increase DPI performance.
