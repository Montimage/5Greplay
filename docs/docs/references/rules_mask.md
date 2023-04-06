Rules mask is a string. It is used to distribute rules on each thread.

## Syntax
It respects the following BNF syntax:

```
rules_mask    :== (thread_id:rule_id_range)+
thread_id     := number
rule_id_range := number | number-rule_id_range | number,rule_id_range
```

`thread_id` must be an integer and starts from 1

### Example

`
"(1:1-4,6)(2:5,7-10)"
`

This rules mask will attribute rules 1,2,3,4,6 to the first thread, rules 5,7,8,9,10 to the second thread. 