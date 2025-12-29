# Logging Architecture Decision

We intentionally use **separate logging instances** for the audio processing
side and the controller side of the application.
Although a unified logging system is attractive,
the two halves of the system live in **different processes**
(and potentially different runtimes), which makes a single
shared logger impractical. Logging frameworks such as Speedlog are
process-local; their state, sinks, and configuration cannot be safely or
meaningfully shared across process boundaries.

A fully unified logging system would require **cross-process IPC**
(e.g. sockets, shared memory, or files) to funnel all log messages into
one place. This adds complexity, latency, failure modes, and—critically
for audio processing—**unpredictable timing behavior**.
Any blocking, allocation, or synchronization introduced by cross-process
logging is unacceptable on the real-time audio thread.

Within a single process, bridging Rust and C++ logging via a C ABI shim is
feasible and well understood. Across processes, however, the same approach
does not apply: C ABIs, logger instances, and thread-safety guarantees
all stop at the process boundary. Treating the audio engine and controller
as independently logged systems respects these constraints and keeps
responsibilities clear.

Instead of a single logger instance, we rely on **consistent log formats,
levels, and metadata** across both halves. This allows logs to be
correlated offline (by timestamp, session ID, etc.) without coupling
the systems at runtime, preserving real-time safety and architectural
simplicity.
