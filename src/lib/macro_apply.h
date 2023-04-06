/*
 * macro_apply.h
 *
 *  Created on: Apr 23, 2018
 *          by: Huu Nghia Nguyen
 */

#ifndef SRC_LIB_MACRO_APPLY_H_
#define SRC_LIB_MACRO_APPLY_H_

//Count number of arguments inside __VA_ARGS__
//Limit of number of arguments is 89
#define __COUNT_ARGS(X,\
		X89,X88,X87,X86,X85,X84,X83,X82,X81,X80,\
		X79,X78,X77,X76,X75,X74,X73,X72,X71,X70,\
		X69,X68,X67,X66,X65,X64,X63,X62,X61,X60,\
		X59,X58,X57,X56,X55,X54,X53,X52,X51,X50,\
		X49,X48,X47,X46,X45,X44,X43,X42,X41,X40,\
		X39,X38,X37,X36,X35,X34,X33,X32,X31,X30,\
		X29,X28,X27,X26,X25,X24,X23,X22,X21,X20,\
		X19,X18,X17,X16,X15,X14,X13,X12,X11,X10,\
		X9,X8,X7,X6,X5,X4,X3,X2,X1,\
		N,...) N
#define COUNT_ARGS(...) __COUNT_ARGS(0, __VA_ARGS__ ,\
		89,88,87,86,85,84,83,82,81,80,\
		79,78,77,76,75,74,73,72,71,70,\
		69,68,67,66,65,64,63,62,61,60,\
		59,58,57,56,55,54,53,52,51,50,\
		49,48,47,46,45,44,43,42,41,40,\
		39,38,37,36,35,34,33,32,31,30,\
		29,28,27,26,25,24,23,22,21,20,\
		19,18,17,16,15,14,13,12,11,10,\
		9,8,7,6,5,4,3,2,1,0)
//example: COUNT_ARGS( a, b, c, "") => 4

//Supported macro for APPLY macro function
#define __APPLY_0(  SEPA, FUN, a, ... )
#define __APPLY_1(  SEPA, FUN, a, ... ) FUN( a )
#define __APPLY_2(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_1(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_3(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_2(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_4(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_3(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_5(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_4(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_6(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_5(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_7(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_6(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_8(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_7(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_9(  SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_8(  SEPA, FUN, __VA_ARGS__ )

#define __APPLY_10( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_9(  SEPA, FUN, __VA_ARGS__ )
#define __APPLY_11( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_10( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_12( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_11( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_13( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_12( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_14( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_13( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_15( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_14( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_16( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_15( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_17( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_16( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_18( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_17( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_19( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_18( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_20( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_19( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_21( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_20( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_22( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_21( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_23( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_22( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_24( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_23( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_25( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_24( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_26( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_25( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_27( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_26( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_28( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_27( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_29( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_28( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_30( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_29( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_31( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_30( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_32( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_31( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_33( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_32( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_34( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_33( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_35( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_34( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_36( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_35( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_37( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_36( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_38( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_37( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_39( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_38( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_40( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_39( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_41( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_40( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_42( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_41( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_43( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_42( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_44( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_43( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_45( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_44( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_46( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_45( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_47( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_46( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_48( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_47( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_49( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_48( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_50( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_49( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_51( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_50( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_52( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_51( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_53( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_52( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_54( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_53( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_55( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_54( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_56( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_55( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_57( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_56( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_58( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_57( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_59( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_58( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_60( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_59( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_61( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_60( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_62( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_61( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_63( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_62( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_64( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_63( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_65( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_64( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_66( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_65( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_67( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_66( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_68( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_67( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_69( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_68( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_70( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_69( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_71( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_70( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_72( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_71( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_73( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_72( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_74( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_73( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_75( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_74( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_76( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_75( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_77( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_76( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_78( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_77( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_79( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_78( SEPA, FUN, __VA_ARGS__ )

#define __APPLY_80( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_79( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_81( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_80( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_82( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_81( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_83( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_82( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_84( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_83( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_85( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_84( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_86( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_85( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_87( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_86( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_88( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_87( SEPA, FUN, __VA_ARGS__ )
#define __APPLY_89( SEPA, FUN, a, ... ) FUN( a ) SEPA() __APPLY_88( SEPA, FUN, __VA_ARGS__ )


#define __APPLY_N( N, ... ) __APPLY_ ##N( __VA_ARGS__ )
//force one level of expansion.
#define __APPLY_N1( N, ... ) __APPLY_N(N, __VA_ARGS__ )
//Apply a macro function on each argument. The results will be concatenated and separated by SEP
#define APPLY( SEPA, FUN, ... ) __APPLY_N1( COUNT_ARGS( __VA_ARGS__ ), SEPA, FUN, __VA_ARGS__ )

//Example1 :
//#define STRINGIFY( a ) #a
//#define SEPARATOR() ,
//APPLY( SEPARATOR, STRINGIFY, hihi, huhu, haha ) ==> "hihi" , "huhu" , "haha"

//Example 2: See ./string_builder.h
//Example 3: See ../configure_override.h

#endif /* SRC_LIB_MACRO_APPLY_H_ */
