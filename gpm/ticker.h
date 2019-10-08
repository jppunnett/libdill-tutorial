// Coroutine-based timer utilities
#pragma once

// tickEvery: returns a channel through which the caller will receive the
// current time at interval ms. An interval of zero means the receiver
// will block forever.
int
TickEvery(int64_t ms);
