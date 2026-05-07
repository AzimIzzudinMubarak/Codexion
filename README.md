# Codexion

*This project has been created as part of the 42 curriculum by azmubara.*

---

## Description

Codexion is a concurrency simulation inspired by the classic dining philosophers problem. Multiple coders sit around a circular co-working hub, each needing two USB dongles simultaneously to compile their quantum code. The challenge is to orchestrate their access to these shared resources using POSIX threads, mutexes, and condition variables — ensuring every coder compiles regularly without ever burning out.

The simulation runs until either a coder burns out (too long without compiling) or every coder has reached the required number of compiles. Two arbitration policies are supported: **FIFO** (first come, first served) and **EDF** (Earliest Deadline First), implemented via a custom priority queue (min-heap) on each dongle.

---

## Instructions

### Compilation

```bash
make
```

This produces the `codexion` executable. Other available targets:

```bash
make clean    # remove object files
make fclean   # remove object files and binary
make re       # full recompile
```

### Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders (and dongles). Must be ≥ 1. |
| `time_to_burnout` | Milliseconds before a coder burns out without compiling. |
| `time_to_compile` | Milliseconds spent compiling (both dongles held). |
| `time_to_debug` | Milliseconds spent debugging after each compile. |
| `time_to_refactor` | Milliseconds spent refactoring before the next compile attempt. |
| `number_of_compiles_required` | Simulation ends when all coders reach this compile count. |
| `dongle_cooldown` | Milliseconds a dongle is unavailable after being released. |
| `scheduler` | Arbitration policy: `fifo` or `edf`. |

All arguments are mandatory. Non-integer values, negative numbers, or an invalid scheduler string will be rejected.

### Example runs

```bash
# 5 coders, 800ms burnout, 200ms compile, 200ms debug, 200ms refactor, 7 compiles, 0ms cooldown, FIFO
./codexion 5 800 200 200 200 7 0 fifo

# 4 coders, 410ms burnout, 200ms compile, 100ms debug, 100ms refactor, 5 compiles, 50ms cooldown, EDF
./codexion 4 410 200 100 100 5 50 edf

# Single coder — only one dongle on the table
./codexion 1 600 200 100 100 3 0 fifo
```

### Expected log format

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
...
1204 3 burned out
```

Each line: `timestamp_ms coder_id state`

---

## Blocking cases handled

### Deadlock prevention

The classic circular-wait condition (Coffman's fourth condition) is broken by imposing a **global ordering on dongle acquisition**. Each coder is assigned a `first` and `second` dongle based on their IDs: the lower-ID dongle is always acquired first. This means no two adjacent coders can be holding each other's required dongle at the same time — the cycle that causes deadlock cannot form.

### Starvation prevention

Both schedulers prevent indefinite starvation through the priority queue on each dongle:

- **FIFO**: Requests are served in strict arrival order via a monotonically increasing arrival counter. A coder waiting for a dongle will always eventually reach the front of the queue.
- **EDF**: The coder with the earliest burnout deadline (`last_compile_start + time_to_burnout`) is served first. This inherently prioritises coders closest to burning out, ensuring liveness as long as the simulation parameters are feasible.

### Dongle cooldown

After a dongle is released, it is marked unavailable until `get_time_ms() + dongle_cooldown`. Coders waiting for that dongle use `pthread_cond_timedwait` targeting the cooldown expiry, avoiding a busy-wait spin while still waking promptly when the dongle becomes available.

### Precise burnout detection

A dedicated monitor thread polls every 1 ms, checking `get_time_ms() - last_compile` for each coder against `time_to_burnout`. This ensures the burnout log is printed within the required 10 ms of the actual burnout moment. When burnout is detected, the log is printed immediately (under `log_mutex`) before `set_stop()` is called, guaranteeing the message is never suppressed.

### Log serialisation

All output — whether from coder threads or the monitor thread — is protected by a single `log_mutex`. No line can interleave with another. The mutex is also checked before printing to suppress output after the stop flag is set, keeping the log clean.

---

## Thread synchronization mechanisms

### `pthread_mutex_t` — three distinct mutexes

**`dongle.mutex`** (one per dongle): Guards the dongle's state — `in_use`, `owner_id`, `cooldown_until`, and the priority queue (`queue`, `queue_size`, `arrival_counter`). Any read or write to these fields happens only while this mutex is held.

**`sim.log_mutex`**: Serialises all `printf` output and protects `last_compile` and `compile_count` per coder. Whenever a coder updates `last_compile` (at compile start) or `compile_count` (at compile end), it holds this mutex. The monitor reads these fields under the same mutex, preventing torn reads.

**`sim.stop_mutex`**: Protects the global `stop` flag. Both `should_stop()` (read) and `set_stop()` (write) lock this mutex before accessing `stop`, ensuring the flag is always seen in a consistent state across all threads.

### `pthread_cond_t` — per-dongle condition variable

`dongle.cond` is used to block coder threads that are waiting for a dongle. Rather than spinning, a coder calls `pthread_cond_timedwait` targeting the dongle's cooldown expiry. When a dongle is released, `pthread_cond_broadcast` wakes all waiting coders so they can re-evaluate the queue. Using broadcast (rather than signal) is necessary because multiple coders may be waiting and the correct one to wake is determined by the priority queue, not by which thread happens to be signalled.

### Priority queue (custom min-heap)

Because C89 has no standard priority queue, a min-heap is implemented directly on a `t_heap_node` array embedded in each `t_dongle`. `push_to_queue` sifts up; `pop_from_queue` sifts down. The heap invariant ensures `queue[0]` always holds the highest-priority (lowest-value) coder. The coder thread checks `queue[0].coder_id == coder->id` as part of its wait condition — it only proceeds to acquire the dongle when it is genuinely at the front of the queue.

### Race condition prevention — example flow

When coder 3 wants dongle 1:

1. Coder 3 locks `dongle[1].mutex`.
2. It pushes itself onto the priority queue and enters `pthread_cond_timedwait`, atomically releasing the mutex.
3. When coder 2 finishes and calls `release_dongle`, it locks `dongle[1].mutex`, updates `cooldown_until`, sets `in_use = 0`, then calls `pthread_cond_broadcast` before unlocking.
4. Coder 3 re-acquires `dongle[1].mutex`, re-checks all conditions (`in_use`, queue position, cooldown). Only if all three are satisfied does it pop itself from the queue, set `in_use = 1`, and proceed.

This check-and-wait loop ensures no coder can observe a stale state or skip past the queue ordering.

---

## Resources

### References

- [POSIX Threads Programming — Lawrence Livermore National Laboratory](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen Downey](https://greenteapress.com/wp/semaphores/)
- [Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Earliest Deadline First scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- `man pthread_cond_timedwait`, `man gettimeofday`, `man pthread_mutex_init`

### AI usage

AI (Claude, Anthropic) was used during this project for the following tasks:

- Reviewing the overall structure and correctness of the thread synchronization logic.
- Identifying potential data races (e.g., reading `stop` without `stop_mutex` inside `log_state`, and reads of `last_compile` in EDF priority calculation without a mutex).
- Discussing trade-offs between FIFO and EDF scheduling fairness under various parameter combinations.
- Proofreading and structuring this README.

All generated suggestions were reviewed, understood, and validated before being applied to the codebase.
