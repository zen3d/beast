/* Birnet
 * Copyright (C) 2006 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <birnet/birnet.h>

BIRNET_EXTERN_C_BEGIN();

/* === auxillary macros for test programs === */

/* provide dummy i18n function for test programs */
#ifndef	  _
#  define _(x) 		(x)
#  define Q_(x)		(x)
#  define N_(x)		(x)
#  define U_(x)		(x)
#endif

/* --- macros --- */
/* macros used for testing.
 * note that g_print() does fflush(stdout) automatically.
 * however we're using g_printerr() for test messages to also allow testing
 * of programs which generate output on stdout.
 */
#ifdef	TEST_VERBOSE
#define TSTART(...)	birnet_test_intro (":\n", __VA_ARGS__)		/* test intro */
#define TOK()           do { g_printerr ("OK.\n"); } while (0)		/* subtest OK */
#define TICK()          TOK()						/* subtest OK */
#define TACK()          do { g_printerr ("ACK.\n"); } while (0)		/* alternate OK */
#define TFAIL()         do { g_printerr ("FAIL.\n"); } while (0)	/* allowed failure */
#define	TPRINT(...)	g_printerr (__VA_ARGS__)			/* misc messages */
#define	TASSERT(code)	TASSERT_impl (code)				/* test assertion */
#define	TERROR(...)	TERROR_impl (__VA_ARGS__)			/* test assertion */
#define TDONE()         do { g_printerr ("DONE.\n"); } while (0)	/* test outro */
#else
#define TSTART(...)	birnet_test_intro (": [", __VA_ARGS__)		/* test intro */
#define TOK()           do { g_printerr ("-"); } while (0)		/* subtest OK */
#define TICK()          TOK()						/* subtest OK */
#define TACK()          do { g_printerr ("+"); } while (0)		/* alternate OK */
#define TFAIL()         do { g_printerr ("X"); } while (0)		/* allowed failure */
#define	TPRINT(...)	do { g_printerr ("*"); } while (0)		/* skip messages */
#define	TASSERT(code)	TASSERT_impl (code)				/* test assertion */
#define	TERROR(...)	TERROR_impl (__VA_ARGS__)			/* test assertion */
#define TDONE()         do { g_printerr ("]\n"); } while (0)		/* test outro */
#endif

/* --- prototypes --- */
static inline void birnet_init_test (int*, char***);
static inline void birnet_test_intro (const char *postfix,
				      const char *format,
				      ...) G_GNUC_PRINTF (2, 3);

#define TASSERT_impl(code)	do {			\
  if (code) TOK (); else {				\
  TFAIL();						\
  g_error ("%s:%u:%s(): assertion failed: %s",		\
           __FILE__, __LINE__, __PRETTY_FUNCTION__,	\
           #code); }					\
} while (0)

#define TERROR_impl(...)	do {			\
  TFAIL();						\
  char *_error_msg_ = g_strdup_printf (__VA_ARGS__);	\
  g_error ("%s:%u:%s(): %s",				\
           __FILE__, __LINE__, __PRETTY_FUNCTION__,	\
           _error_msg_);				\
  g_free (_error_msg_);					\
} while (0)



/* --- implmentation details --- */
static inline void
birnet_test_intro (const char *postfix,
		   const char *format,
		   ...)
{
  va_list args;
  va_start (args, format);
  char *msg = g_strdup_vprintf (format, args);
  va_end (args);
  g_printerr ("%s%s", msg, postfix);
  g_free (msg);
}

/* convenience initialization functions for tests */
static inline void
birnet_init_test (int    *argc,
		  char ***argv)
{
  birnet_init (argc, argv, NULL);
  unsigned int flags = g_log_set_always_fatal ((GLogLevelFlags) G_LOG_FATAL_MASK);
  g_log_set_always_fatal ((GLogLevelFlags) (flags | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL));
  g_printerr ("TESTING: %s\n", g_get_prgname());
}

BIRNET_EXTERN_C_END();