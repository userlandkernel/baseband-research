#ifndef __idl4_macros_h__
#define __idl4_macros_h__

#ifndef IDL4_ARCH
#error IDL4_ARCH not defined
#endif

#ifndef IDL4_API
#error IDL4_API not defined
#endif

#define IDL4_INC_ARCH(x)    <idl4biguuid_arm/arch/IDL4_ARCH/x>
#define IDL4_INC_API(x)     <idl4biguuid_arm/api/IDL4_API/x>
#define IDL4_INC_GLUE(x)    <idl4biguuid_arm/glue/IDL4_API-IDL4_ARCH/x>

#define IDL4_NO_RESPONSE 	-1

#if (__GNUC__>=3) && !defined(__ARMCC_VERSION)
#define IDL4_EXPECT_TRUE(x)		__builtin_expect((x), 1)
#define IDL4_EXPECT_FALSE(x)		__builtin_expect((x), 0)
#define IDL4_EXPECT_VALUE(x,val)	__builtin_expect((x), (val))
#else /* __GNUC__<3 */
#define IDL4_EXPECT_TRUE(x)		(x)
#define IDL4_EXPECT_FALSE(x)		(x)
#define IDL4_EXPECT_VALUE(x,val)	(x)
#endif /* __GNUC__<3 */

#ifdef __PIC__
#error IDL4 does not yet support PIC code
#endif

#endif /* __idl4_macros_h__ */
