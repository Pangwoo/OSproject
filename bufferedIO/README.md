# Buffered I/O

## Overview
This project implements a **custom buffered I/O layer** in C, similar in concept
to the standard `read()` system call, but built **entirely on top of low-level
block I/O APIs**.

The goal is to understand how buffering works internally by manually managing
fixed-size blocks (`B_CHUNK_SIZE = 512`) and efficiently transferring data to
caller-provided buffers.

---

## Learning Objectives
- Implement buffered file I/O from scratch
- Work with **low-level block-based storage APIs**
- Manage per-file state using **File Control Blocks (FCBs)**
- Handle partial reads and end-of-file conditions correctly
- Efficient memory copying using `memcpy`
- Understand why buffering is critical in file systems

---

## Implemented Functions

The following functions are implemented in `b_io.c`:

```c
b_io_fd b_open (char *filename, int flags);
int b_read (b_io_fd fd, char *buffer, int count);
int b_close (b_io_fd fd);
