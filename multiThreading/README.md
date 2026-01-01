# Word Blast

## Overview
**Word Blast** is a multithreaded C program that analyzes a large text file
(*War and Peace*) and counts the frequency of words that are **six or more
characters long**.

The file is divided into equally sized chunks, each processed in parallel
using POSIX threads (`pthread`). Results from all threads are combined to
determine the **top 10 most frequent words**.

---

## Learning Objectives
- Multithreaded programming using **POSIX threads**
- File I/O using **Linux system calls** (`open`, `read`, `lseek`, `pread`)
- Synchronization with **mutex locks**
- Parallel workload partitioning
- Performance measurement and analysis
- Large-scale text processing

---

## Program Design

### Thread Model
- The input file is split into **N chunks**, where `N` is the number of threads
- Each thread:
  - Reads its assigned section of the file
  - Extracts words ≥ 6 characters
  - Maintains local or shared word frequency data
- Threads run **concurrently** and return results to the main thread

### Synchronization
- Critical sections are protected using **mutex locks**
- Thread-safe operations are enforced when accessing shared data

---

## Command Line Usage
```bash
./<program> <FileName> <ThreadCount>
$ make run
./LEE_GWANGWOO_HW4_main WarAndPeace.txt 2
1: word: Pierre count: 1963
2: word: Prince count: 1577
3: word: Natásha count: 1213
4: word: Andrew count: 1143
5: word: himself count: 1017
6: word: French count: 881
7: word: before count: 779
8: word: Rostóv count: 776
9: word: thought count: 766
10: word: CHAPTER count: 730
Total Time was 1.762232918 seconds