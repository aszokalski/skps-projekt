# floorf-ieee.m4 serial 1
dnl Copyright (C) 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This macro is in a separate file (not in floorf.m4 and not inlined in the
dnl module description), so that gl_FUNC_FLOORF can test whether 'aclocal' has
dnl found uses of this macro.

AC_DEFUN([gl_FUNC_FLOORF_IEEE],
[
  m4_divert_text([INIT_PREPARE], [gl_floorf_required=ieee])
  AC_REQUIRE([gl_FUNC_FLOORF])
])