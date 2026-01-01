# CSC415 – Assignment 2: Buffering and Structures

## Overview
This project focuses on **memory structures and block-based buffering** in C.
The assignment is designed to strengthen understanding of low-level memory
management, structure layout in memory, and efficient data transfer using
buffers.

Students implement a program that populates a structured data record,
buffers variable-length input data into fixed-size blocks, and validates
memory correctness through a binary hexdump analysis.

---

## Program Requirements

### Personal Information Structure
- Dynamically allocate and populate a `personalInfo` structure
- Populate fields using command-line arguments:
  - First name
  - Last name
  - Student ID
  - Grade level
  - Programming language knowledge (bitmap)
  - Message string

### Buffering Logic
- Use a fixed block size (`BLOCK_SIZE = 256`)
- Retrieve an unknown number of strings using `getNext()`
- Copy data into the buffer **using `memcpy` (not byte-by-byte)**
- Commit full buffers using `commitBlock()`
- Properly handle partial buffers at the end

### Execution Flow
1. Populate and write the `personalInfo` structure
2. Buffer and commit all data blocks
3. Call `checkIt()` to validate correctness
4. Exit with the return value of `checkIt()`


## 🧪 Sample Output (Excerpt)
```text
------------------------------------ CHECK ------------------------------------
Running the check for Robert Bierman
Student ID: 900000000, Grade Level: Instructor
Languages: FFFFFF
Message:
There will be a message printed here

The Check Succeeded (0, 0)
END-OF-ASSIGNMENT
000000: D1 F2 11 66 FF 7F 00 00 ...
