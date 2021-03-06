# First Reader-Writer Problem
This is a solution to the first reader-writer problem implemented in C with
using two mutex locks and a reader counter.

## Assumptions and problem definitions
* Readers and writers need to access a file concurrently
* If a writer is writing, then no other writers or readers can access the file
  for reading or writing as that may corrupt the file.
* Since readers don't modify the data, multiple readers may access the space at
  one time.
* If a reader is reading and another reader enters, it will begin reading
  regardless of whether a writer is waiting. This results in starvation of
  writers and is part of the definition of the first reader-writer problem.

## A few comments
* To describe what's going on, I've added print statements to print to stdout.
  I disabled buffering, but sometimes it doesn't work. Mac OS X doesn't respect
  the buffering, but my Ubuntu Linux VM does.
* I've marked areas of configuration with "/* CONFIGURABLE */". The most
  important one is at the bottom in main. That's where you can specify which
  test function to call and simulate a scenario.

## Parts of soln.c

### READER-WRITER DEFINITIONS
* The declaration of the parts of the solution.
* start_thread: starts a reader or writer thread
* reader, writer: subroutines called by pthread_create. They synchronize access
  to the file. See read_file and write_file.
* read_file, write_file: does the actual reading/writing, called by the
  reader/writer subroutines to actually read or write to the file.

### IMPLEMENTATION SECTION
* The implementations of the above functions.
* The explanation of my synchronization implementation is in the reader/writer
  functions.

### TEST FUNCTIONS
* Functions to simulate specific scenarios.

### MAIN
* The main function and entry point of the soln executable.

## Usage
```
make
./soln {test_file}
# !!!Warning: test_file will likely be overwritten by a writer!!!
# Note: user must have rw access to test_file and it must exist or soln will
#  print an error and exit. A test_file named test_file is created on make.
```

## The Solution
```
mutex rw
mutex r
int rc=0
writer() {
    lock(rw)
    <write>
    unlock(rw)
}
reader() {
    lock(r)
    if (rc++==0)
        lock(rw)
    unlock(r)
    <read>
    lock(r)
    if (--rc==0)
        unlock(rw)
    unlock(r)
}
```

## The Makefile
```
make [all]      : Creates executable, soln
make test       : Creates debug executable, dsoln
make pkg        : Creates a tar with files for submission
make clean      : Cleans
```
