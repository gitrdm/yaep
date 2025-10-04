#!/bin/sh
# test/compare_parsers/compare_parsers.sh.  Generated from compare_parsers.sh.in by configure.
#
# Copyright (c) 1997-2018  Vladimir Makarov <vmakarov@gcc.gnu.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Script to comparison of Bison, Yacc (byacc), YAEP (Yet
# Another Earley Parser), MARPA Earley's parser, and MSTA.  The script
# can find installed MARPA in /usr and /usr/local.  If MARPA parser is
# installed somewhere else then use environment variable MARPA_DIR to
# point its installation directory.
#
# Usage: compare.tst
#
# The script uses Bison, Yacc (byacc), gcc, lex, MARPA.
#

YACC=
if type yacc|fgrep /yacc; then
    YACC=yacc
elif type byacc|fgrep /byacc; then
    YACC=byacc;
fi
BISON=
if type bison|fgrep /bison; then
    BISON=bison
fi
MSTA=
if test -f ./msta; then
    MSTA=./msta
fi

YAEPLIB=../../src/libyaep.a

GCC='gcc -O3 -w'
TEST=small_test.i
BIG_TEST=./whole_gcc_test.i
outfile=./a.out

# Default behavior flags
# By default we perform setup (preflight) and then full run; use --setup-only to just do preflight.
SETUP_ONLY=0
# Robust timing uses /usr/bin/time -v when available. Default: enabled.
ROBUST_TIMING=1
# Logging: default enabled (logs go to ./logs)
DO_LOG=1
LOGDIR=./logs
# Summary: print aggregated summary after runs (default: enabled)
DO_SUMMARY=1
# Number of runs for statistics
RUNS=1

# Parse command-line flags
while [ "$#" -gt 0 ]; do
  case "$1" in
    --setup-only)
      SETUP_ONLY=1
      shift;;
    --no-robust-timing)
      ROBUST_TIMING=0
      shift;;
    --no-log)
      DO_LOG=0
      shift;;
    --log-dir)
      shift; LOGDIR="$1"; shift;;
    --runs)
      shift; RUNS="$1"; shift;;
    --no-summary)
      DO_SUMMARY=0; shift;;
    --help|-h)
      echo "Usage: $0 [--setup-only] [--no-robust-timing] [--no-log] [--log-dir DIR] [--runs N] [--no-summary]";
      exit 0;;
    *)
      echo "Unknown option: $1"; exit 1;;
  esac
done

# Ensure numeric RUNS
case "$RUNS" in
  ''|*[!0-9]* ) echo "Invalid runs value: $RUNS"; exit 1;;
esac

# Ensure log dir exists if logging enabled
if [ "$DO_LOG" -eq 1 ]; then
  mkdir -p "$LOGDIR" || exit 1
fi

# Helper: sanitize label for filename
sanit() {
  echo "$1" | tr ' /' '__' | tr -cd 'A-Za-z0-9_-.\n'
}

# Helper: run a command with robust timing/logging and repeat RUNS times
# Usage: run_and_log "Label" "command-string" "stdin-file"
run_and_log() {
  label="$1"; shift
  cmd="$1"; shift
  stdin="$1"; shift || true
  base=$(sanit "$label")
  i=1
  overall_status=0
  while [ $i -le $RUNS ]; do
    logfile="$LOGDIR/${base}_run${i}.log"
    echo "===== $label run #$i =====" >> "$logfile"
    echo "date: $(date -u +'%Y-%m-%dT%H:%M:%SZ')" >> "$logfile"
    if [ "$DO_LOG" -eq 1 ]; then
      if [ "$ROBUST_TIMING" -eq 1 ] && command -v /usr/bin/time >/dev/null 2>&1; then
        # GNU time with verbose output and append
        /usr/bin/time -v -o "$logfile" -a bash -c "$cmd < \"$stdin\" >> \"$logfile\" 2>&1"
        status=$?
      else
        # Fallback: use shell time and append stdout/stderr
        ( time bash -c "$cmd < \"$stdin\"" ) >> "$logfile" 2>&1
        status=$?
      fi
    else
      # No logging: still perform robust timing live
      if [ "$ROBUST_TIMING" -eq 1 ] && command -v /usr/bin/time >/dev/null 2>&1; then
        /usr/bin/time -v bash -c "$cmd < \"$stdin\"" >/dev/null 2>&1
        status=$?
      else
        ( time bash -c "$cmd < \"$stdin\"" ) >/dev/null 2>&1
        status=$?
      fi
    fi
    if [ $status -ne 0 ]; then
      echo "Command failed (status $status): $cmd" >> "$logfile"
      overall_status=1
    fi
    i=$((i+1))
  done
  return $overall_status
}


# Aggregate logs and print a compact summary table.
aggregate_logs() {
  # Only run if LOGDIR exists
  if [ ! -d "$LOGDIR" ]; then
  echo "No logs directory found: $LOGDIR" >&2
  return 0
  fi

  # Use an embedded Python script for robust parsing and stats computation.
  python3 - "$LOGDIR" <<'PY'
import sys,os,re,statistics
logdir=sys.argv[1]
files=[f for f in os.listdir(logdir) if f.endswith('.log')]
data={} # label -> list of (time_s, rss_kb)
re_elapsed=re.compile(r'Elapsed .*: ([0-9:]+(?:\.[0-9]+)?)')
re_real=re.compile(r'^real\s+(?:(\d+)m)?([0-9.]+)s', re.M)
re_rss=re.compile(r'Maximum resident set size .*: *([0-9]+)')
for fn in files:
  m=re.match(r'^(.*)_run\d+\.log$', fn)
  if not m: continue
  label=m.group(1)
  path=os.path.join(logdir,fn)
  s=open(path,'r',errors='ignore').read()
  t=None
  m1=re_elapsed.search(s)
  if m1:
    tstr=m1.group(1)
    parts=tstr.split(':')
    if len(parts)==3:
      h=int(parts[0]); m=int(parts[1]); sec=float(parts[2]); t=h*3600+m*60+sec
    elif len(parts)==2:
      m=int(parts[0]); sec=float(parts[1]); t=m*60+sec
    else:
      try:
        t=float(parts[0])
      except:
        t=None
  else:
    m2=re_real.search(s)
    if m2:
      mins=m2.group(1)
      secs=float(m2.group(2))
      mins=int(mins) if mins else 0
      t=mins*60+secs
  rss=None
  m3=re_rss.search(s)
  if m3:
    try:
      rss=int(m3.group(1))
    except:
      rss=None
  if t is None:
    # skip entries without time
    continue
  data.setdefault(label,[]).append((t,rss if rss is not None else 0))

if not data:
  print('No timing data found in logs.')
  sys.exit(0)

# Print header
hdr='{:<36s} {:>6s} {:>8s} {:>8s} {:>8s} {:>8s} {:>8s}'.format('test', 'runs', 'median(s)', 'mean(s)', 'stdev(s)', 'min(s)', 'max(s)')
print(hdr)
print('-'*len(hdr))
for label,vals in sorted(data.items()):
  times=[v[0] for v in vals]
  rss=[v[1] for v in vals]
  runs=len(times)
  median=statistics.median(times)
  mean=statistics.mean(times)
  stdev=statistics.pstdev(times) if runs>1 else 0.0
  mn=min(times)
  mx=max(times)
  print('{:<36s} {:6d} {:8.3f} {:8.3f} {:8.3f} {:8.3f} {:8.3f}'.format(label, runs, median, mean, stdev, mn, mx))
  # print RSS summary
  if any(rss):
    meanrss=statistics.mean(rss)
    print('  RSS mean: {:.0f} KB over {} runs'.format(meanrss, runs))
PY
}


echo This script compares speed of parser generated by Yacc, Bison, Msta, MARPA
echo for parsing real C program.

echo Now test file preparation

cp ./test.i $TEST
for i in 0 1 2 3 4 5 6 7 8 9;do cat ./test1.i >>$TEST;done

# --- Preflight: ensure generated/copyable test assets present ---
echo "Preflight: checking for generated/copyable test assets"
# Ensure generated lexer implementation (ansic.c) exists; generate from ../ansic.l if possible.
if [ ! -f ./ansic.c ]; then
  if command -v flex >/dev/null 2>&1; then
    echo "ansic.c not found — running: flex -o ansic.c ../ansic.l"
    if ! flex -o ansic.c ../ansic.l; then
      echo "flex failed to generate ansic.c" >&2
      exit 1
    fi
  else
    if [ -f ../ansic.c ]; then
      cp ../ansic.c ./ansic.c
      echo "copied ../ansic.c -> ./ansic.c"
    else
      echo "ansic.c is required but not found and flex is not available" >&2
      echo "Please install flex or generate ansic.c manually." >&2
      exit 1
    fi
  fi
else
  echo "ansic.c already present"
fi

# Ensure ANSI grammar files are present
if [ ! -f ./ansi_grammar.txt ] && [ -f ../common/ansi_grammar.txt ]; then
  cp ../common/ansi_grammar.txt ./ansi_grammar.txt
  echo "copied ../common/ansi_grammar.txt -> ./ansi_grammar.txt"
fi
if [ ! -f ./ansi_grammar_data.h ] && [ -f ../common/ansi_grammar_data.h ]; then
  cp ../common/ansi_grammar_data.h ./ansi_grammar_data.h
  echo "copied ../common/ansi_grammar_data.h -> ./ansi_grammar_data.h"
fi

# Ensure the YAEP library exists; attempt to build it if missing.
if [ ! -f "$YAEPLIB" ]; then
  echo "$YAEPLIB not found; attempting to build YAEP library"
  if [ -f ../../configure ]; then
    echo "running: (cd ../../ && ./configure && make -j$(nproc))"
    (cd ../../ && ./configure && make -j$(nproc)) || {
      echo "Failed to configure/build YAEP. Please build libyaep.a manually." >&2
      exit 1
    }
  else
    echo "No configure script found; attempting 'make' in project root"
    if (cd ../../ && make -j$(nproc)); then
      echo "built YAEP via make in project root"
    else
      echo "Could not build YAEP library automatically. Please run configure/make in the project root." >&2
      exit 1
    fi
  fi
else
  echo "Found YAEP library: $YAEPLIB"
fi

# If setup-only requested, exit after preflight
if [ "$SETUP_ONLY" -eq 1 ]; then
  echo "--setup-only requested; preflight complete. Exiting."
  exit 0
fi

if test x$BISON != x; then
  # BISON
  echo ++++++++Bison: pure parsing
  if $BISON -y ./ansic.y && $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yyparse.c $YAEPLIB -o $outfile; then
    run_and_log "Bison: pure parsing" "$outfile" "$TEST" || { echo Failure with $BISON; exit 1; }
  else
    echo Failure with $BISON
    exit 1
  fi
fi

if test x$BISON != x; then
  # BISON
  echo Bison -- Big test
  if $BISON -y ./ansic.y && $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yyparse.c $YAEPLIB -o $outfile; then
    run_and_log "Bison: big test" "$outfile" "$BIG_TEST" || { echo Failure with $BISON on the big test; exit 1; }
  else
    echo Failure with $BISON on the big test
    exit 1
  fi
fi

if test x$YACC != x; then
  # YACC
  echo ++++++++Yacc: pure parsing
    
  if $YACC ./ansic.y && $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yyparse.c $YAEPLIB -o $outfile; then
    run_and_log "Yacc: pure parsing" "$outfile" "$TEST" || { echo Failure with $YACC; exit 1; }
  else
    echo Failure with $YACC
    exit 1
  fi
fi

if test x$YACC != x; then
  # YACC
  echo YACC -- Big test
    
  if $YACC ./ansic.y && $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yyparse.c $YAEPLIB -o $outfile; then
    run_and_log "Yacc: big test" "$outfile" "$BIG_TEST" || { echo Failure with $YACC on the big test; exit 1; }
  else
    echo Failure with $YACC on the big test
    exit 1
  fi
fi

if test x$MSTA != x; then
    # MSTA
    echo ++++++++Msta: pure parsing
  if $MSTA -yacc-file-names ./ansic.y && $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yyparse.c $YAEPLIB -o $outfile; then
  	run_and_log "Msta: pure parsing" "$outfile" "$TEST" || { echo Failure with $MSTA; exit 1; }
  else
  	echo Failure with $MSTA
  	exit 1
  fi
fi

if test x$MSTA != x; then
    # MSTA
    echo Msta -- Big test
  if $MSTA -yacc-file-names ./ansic.y && $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yyparse.c $YAEPLIB -o $outfile; then
  	run_and_log "Msta: big test" "$outfile" "$BIG_TEST" || { echo Failure with $MSTA on the big test; exit 1; }
  else
  	echo Failure with $MSTA on the big test
  	exit 1
  fi
fi

# YAEP (Yet Another Earley Parser)
echo ++++++++YAEP '(Yet Another Earley Parser)': pure parsing

echo No Lookahead
if $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yaep.c $YAEPLIB -o $outfile; then
  run_and_log "YAEP No Lookahead" "$outfile 0 0" "$TEST" || { echo Failure with 'YAEP (Yet Another Earley Parser)'; exit 1; }
else
  echo Failure with 'YAEP (Yet Another Earley Parser)'
  exit 1
fi

echo Static Lookahead
if $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yaep.c $YAEPLIB -o $outfile; then
  run_and_log "YAEP Static Lookahead" "$outfile 1 0" "$TEST" || { echo Failure with 'YAEP (Yet Another Earley Parser)'; exit 1; }
else
  echo Failure with 'YAEP (Yet Another Earley Parser)'
  exit 1
fi

echo Dynamic Lookahead
if $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yaep.c $YAEPLIB -o $outfile; then
  run_and_log "YAEP Dynamic Lookahead" "$outfile 2 0" "$TEST" || { echo Failure with 'YAEP (Yet Another Earley Parser)'; exit 1; }
else
  echo Failure with 'YAEP (Yet Another Earley Parser)'
  exit 1
fi

echo Static Lookahead -- Big test
if $GCC -I. -I.. -I. -I../../test -I../../src ./../ticker.c ./test_yaep.c $YAEPLIB -o $outfile; then
  run_and_log "YAEP Static Lookahead big" "$outfile 1 0" "$BIG_TEST" || { echo Failure with 'YAEP (Yet Another Earley Parser)'; exit 1; }
else
  echo Failure with 'YAEP (Yet Another Earley Parser)'
  exit 1
fi


# MARPA Earley parser
if test x$MARPA_DIR != x \
   || test -f /usr/include/marpa.h \
   && (test -f /usr/lib/libmarpa.a || test -f /usr/lib64/libmarpa.a) \
   || test -f /usr/local/include/marpa.h \
   && (test -f /usr/local/lib/libmarpa.a || test -f /usr/local/lib64/libmarpa.a); then

if test x$MARPA_DIR = x; then
   if  test -f /usr/include/marpa.h \
       && (test -f /usr/lib/libmarpa.a || test -f /usr/lib64/libmarpa.a); then
       MARPA_DIR=/usr
   else
       MARPA_DIR=/usr/local
   fi
fi

echo ++++++++MARPA Earley parser from $MARPA_DIR: pure parsing

MARPA_YACC_FILE=./test.y
MARPA_TEST=marpa_test.c

if test x$MSTA != x; then
    $MSTA $MARPA_YACC_FILE 2>&1 | fgrep -v 'look ahead'
elif test x$BISON != x; then
    $BISON -y $MARPA_YACC_FILE && mv y.tab.c $MARPA_TEST
elif test x$YACC != x; then
    $YACC $MARPA_YACC_FILE && mv y.tab.c $MARPA_TEST
else
    echo We need yacc, bison, or msta to test MARPA.
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARPA_DIR/lib:$MARPA_DIR/lib64
if test -f $MARPA_TEST && $GCC -I. -I.. -I. -I../../test -I../../src -I$MARPA_DIR/include -L$MARPA_DIR/lib -L$MARPA_DIR/lib64 ./../ticker.c $MARPA_TEST $YAEPLIB -lmarpa -o $outfile; then
  run_and_log "MARPA: pure parsing" "$outfile" "$TEST" || { echo Failure with MARPA parser; exit 1; }
  echo
  echo MARPA Earley parser -- Big test
  if test x$MARPA_BIG_TEST != x; then
     run_and_log "MARPA: big test" "$outfile" "$BIG_TEST" || { echo Failure with MARPA parser on the big test; exit 1; }
  else
      echo To test MARPA on the biggest file, define env. variable MARPA_BIG_TEST.
      echo Please be aware -- MARPA uses 30GB for this test.
  fi
else
  echo Failure with MARPA parser
  exit 1
fi
fi

rm -f $outfile
rm -f $MARPA_TEST y.tab.c $TEST
if [ "$DO_LOG" -eq 1 ] && [ "$DO_SUMMARY" -eq 1 ]; then
  aggregate_logs
fi
exit 0
