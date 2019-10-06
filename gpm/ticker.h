#pragma once
// Coroutine-based timer utilities

// tickEvery: returns a channel through which the caller will receive the
// current time at interval ms.
int
TickEvery(int64_t ms);
