#pragma once

// [ DEFINING ] //

// Bit length detection
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__) || (defined(__riscv) && __riscv_xlen == 64)
	#define C_BITLEN 64
#elif defined(__LP32__) || defined(__ILP32__) || defined(_WIN32) || defined(__i386__) || defined(_M_IX86) || defined(__arm__) || defined(_M_ARM) || defined(__ARM_ARCH_ISA_THUMB) || defined(__powerpc__) || (defined(__mips__) && __mips == 32) || (defined(__riscv) && __riscv_xlen == 32)
	#define C_BITLEN 32
#else
	#define C_BITLEN 16
#endif

// Compiler detection
#if defined(__GNUC__)
	#define C_COMPILER_GNU
#elif defined(__clang__)
	#define C_COMPILER_CLANG
#elif defined(_MSC_VER)
	#define C_COMPILER_MSVC
#elif defined(__INTEL_COMPILER)
	#define C_COMPILER_INTEL
#else
	#define C_COMPILER_UNKNOWN
#endif

// Standard detection
#if defined(__STDC__) || defined(__STDC_VERSION__)
	#if  (__STDC_VERSION__ > 201710L)
		#define C_STANDARD 2023
	#elif (__STDC_VERSION__ >= 201710L)
		#define C_STANDARD 2017
	#elif (__STDC_VERSION__ >= 201112L)
		#define C_STANDARD 2011
	#elif (__STDC_VERSION__ >= 199901L)
		#define C_STANDARD 1999
	#elif (__STDC_VERSION__ >= 199409L)
		#define C_STANDARD 1989
	#else
		#define C_STANDARD 1989
	#endif
#else
	#define C_STANDARD 1989
#endif

// Platform detection
#if   defined(_WIN32) || defined(_WIN64)
	#define C_PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
	#if   defined(TARGET_OS_IPAD)
		#define C_PLATFORM_IPAD
	#elif defined(TARGET_OS_IPHONE) || defined(TARGET_OS_IOS)
		#define C_PLATFORM_IOS
	#elif defined(TARGET_OS_MAC) || defined(TARGET_OS_OSX)
		#define C_PLATFORM_MACOS
	#elif defined(TARGET_OS_SIMULATOR)
		#define C_PLATFORM_SIMIOS
	#endif
#elif defined(__linux__)
	#define C_PLATFORM_LINUX
#elif defined(__ANDROID__)
	#define C_PLATFORM_ANDROID
#elif defined(__CYGWIN__)
	#define C_PLATFORM_CYGWIN
#elif defined(__FreeBSD__)
	#define C_PLATFORM_FREEBSD
#elif defined(__NetBSD__)
	#define C_PLATFORM_NETBSD
#elif defined(__OpenBSD__)
	#define C_PLATFORM_OPENBSD
#else
	#define C_PLATFORM_UNKNOWN
#endif

// [ DEFINING ] //

// Byte length type (size_t)
#if (C_BITLEN == 64)
	typedef unsigned long long size_t;
	#define SIZE_MAX 0xFFFFFFFFFFFFFFFFULL
#elif (C_BITLEN == 32)
	typedef unsigned int size_t;
	#define SIZE_MAX 0xFFFFFFFFU
#elif (C_BITLEN == 16)
	typedef unsigned short size_t;
	#define SIZE_MAX 65535
#endif

// Defining NULL
#ifndef NULL
	#define NULL ((void*)0)
#endif

// Boolean types
#if (C_STANDARD < 1999)
	typedef enum {false, true} bool;
#elif (C_STANDARD < 2023)
	typedef _Bool bool;
	#define true 1
	#define false 0
#endif
// C23 removes _Bool and introduces bool, true, and false as keywords

// Underscored keywords
#if (C_STANDARD < 2023) && (C_STANDARD >= 2011)
	#define alignas _Alignas
	#define alignof _Alignof
	#define static_assert _Static_assert
	#define thread_local _Thread_local
	#define no_return _Noreturn
#endif

#if (C_STANDARD >= 2011)
	#define atomic _Atomic
#endif

#if (C_STANDARD >= 1999)
	#define imaginary _Imaginary
#endif

// This macros
#if defined(__FILE__)
	#define C_FILE __FILE__
#endif
#if defined(__LINE__)
	#define C_LINE __LINE__
#endif
#if defined(__func__)
	#define C_FUNC __func__
#endif