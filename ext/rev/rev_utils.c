/*
 * Copyright (C) 2007 Tony Arcieri
 * You may redistribute this under the terms of the Ruby license.
 * See LICENSE for details
 */

#include "ruby.h"

#include <sys/resource.h>

#ifdef HAVE_SYS_PARAM_H && HAVE_SYS_SYSCTL_H
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

/* Module and object handles */
static VALUE mRev = Qnil;
static VALUE cRev_Utils = Qnil;

/* Method implementations */
static VALUE Rev_Utils_ncpus(VALUE self);
static VALUE Rev_Utils_maxfds(VALUE self);
static VALUE Rev_Utils_setmaxfds(VALUE self, VALUE max);

/*
 * Assorted utility routines
 */
void Init_rev_utils()
{
  mRev = rb_define_module("Rev");
  cRev_Utils = rb_define_module_under(mRev, "Utils");

  rb_define_singleton_method(cRev_Utils, "ncpus", Rev_Utils_ncpus, 0);
  rb_define_singleton_method(cRev_Utils, "maxfds", Rev_Utils_maxfds, 0);
  rb_define_singleton_method(cRev_Utils, "maxfds=", Rev_Utils_setmaxfds, 1);
}

/**
 *  call-seq:
 *    Rev::Utils.ncpus -> Integer
 * 
 * Return the number of CPUs in the present system
 */
static VALUE Rev_Utils_ncpus(VALUE self)
{
  int ncpus = 0;

#ifdef HAVE_SYS_PARAM_H && HAVE_SYS_SYSCTL_H
#define HAVE_REV_UTILS_NCPUS
  size_t size = sizeof(int);

  if(sysctlbyname("hw.ncpu", &ncpus, &size, NULL, 0)) 
    return INT2NUM(1);
#endif

#ifdef HAVE_LINUX_PROCFS_H
#define HAVE_REV_UTILS_NCPUS
  char buf[512];
  FILE *cpuinfo;
  
  if(!(cpuinfo = fopen("/proc/cpuinfo", "r")))
    rb_sys_fail("fopen");

  while(fgets(buf, 512, cpuinfo)) {
    if(!strncmp(buf, "processor", 9))
      ncpus++;
  }
#endif

#ifndef HAVE_REV_UTILS_NCPUS
  rb_raise(rb_eRuntimeError, "operation not supported");
#endif

  return INT2NUM(ncpus);
}

/**
 *  call-seq:
 *    Rev::Utils.maxfds -> Integer
 * 
 * Return the maximum number of files descriptors available to the process
 */
static VALUE Rev_Utils_maxfds(VALUE self)
{
  struct rlimit rlim;

  if(getrlimit(RLIMIT_NOFILE, &rlim) < 0)
    rb_sys_error("getrlimit");

  return INT2NUM(rlim.rlim_cur);
}

/**
 *  call-seq:
 *    Rev::Utils.maxfds=(count) -> Integer
 * 
 * Set the number of file descriptors available to the process.  May require
 * superuser privileges.
 */
static VALUE Rev_Utils_setmaxfds(VALUE self, VALUE max)
{
  struct rlimit rlim;

  rlim.rlim_cur = rlim.rlim_max = NUM2INT(max);

  if(setrlimit(RLIMIT_NOFILE, &rlim) < 0)
    rb_sys_error("setrlimit");

  return max;
}