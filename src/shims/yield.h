/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_YIELD__
#define __DISPATCH_SHIMS_YIELD__

#pragma mark -
#pragma mark _dispatch_wait_until

// _dispatch_wait_until() is used for cases when we're waiting on a thread to
// finish a critical section that is a few instructions long and cannot fail
// (IOW has a guarantee of making forward progress).
//
// Using _dispatch_wait_until() has two implications:
// - there's a single waiter for the specified condition,
// - the thing it is waiting on has a strong guarantee of forward progress
//   toward resolving the condition.
//
// For these reasons, we spin shortly for the likely case when the other thread
// is on core and we just caught it in the inconsistency window. If the
// condition we're waiting for doesn't resolve quickly, then we yield because
// it's very likely the other thread that can unblock us is preempted, and we
// need to wait for it to be scheduled again.
//
// Its typical client is the enqueuer/dequeuer starvation issue for the dispatch
// enqueue algorithm where there is typically a 1-10 instruction gap between the
// exchange at the tail and setting the head/prev pointer.
#if DISPATCH_HW_CONFIG_UP
#define _dispatch_wait_until(c) ({ \
		__typeof__(c) _c; \
		int _spins = 0; \
		for (;;) { \
			if (likely(_c = (c))) break; \
			_spins++; \
			_dispatch_preemption_yield(_spins); \
		} \
		_c; })
#else
#ifndef DISPATCH_WAIT_SPINS_WFE
#define DISPATCH_WAIT_SPINS_WFE 10
#endif
#ifndef DISPATCH_WAIT_SPINS // <rdar://problem/15440575>
#define DISPATCH_WAIT_SPINS 1024
#endif
#define _dispatch_wait_until(c) ({ \
		__typeof__(c) _c; \
		int _spins = -(DISPATCH_WAIT_SPINS); \
		for (;;) { \
			if (likely(_c = (c))) break; \
			if (unlikely(_spins++ >= 0)) { \
				_dispatch_preemption_yield(_spins); \
			} else { \
				dispatch_hardware_pause(); \
			} \
		} \
		_c; })
#endif

DISPATCH_NOT_TAIL_CALLED DISPATCH_EXPORT
void *_dispatch_wait_for_enqueuer(void **ptr, void **tailp);

#pragma mark -
#pragma mark _dispatch_contention_wait_until

#if DISPATCH_HW_CONFIG_UP
#define _dispatch_contention_wait_until(c) false
#else
#ifndef DISPATCH_CONTENTION_SPINS_MAX
#define DISPATCH_CONTENTION_SPINS_MAX (128 - 1)
#endif
#ifndef DISPATCH_CONTENTION_SPINS_MIN
#define DISPATCH_CONTENTION_SPINS_MIN (32 - 1)
#endif
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR) || DISPATCH_TARGET_DK_EMBEDDED
#define _dispatch_contention_spins() \
		((DISPATCH_CONTENTION_SPINS_MIN) + ((DISPATCH_CONTENTION_SPINS_MAX) - \
		(DISPATCH_CONTENTION_SPINS_MIN)) / 2)
#elif defined(_WIN32)
// Use randomness to prevent threads from resonating at the same frequency and
// permanently contending. Windows doesn't provide rand_r(), so use a simple
// LCG. (msvcrt has rand_s(), but its security guarantees aren't optimal here.)
#define _dispatch_contention_spins() ({ \
		static os_atomic(unsigned int) _seed = 1; \
		unsigned int _next = os_atomic_load(&_seed, relaxed); \
		os_atomic_store(&_seed, _next * 1103515245 + 12345, relaxed); \
		((_next >> 24) & (DISPATCH_CONTENTION_SPINS_MAX)) | \
				(DISPATCH_CONTENTION_SPINS_MIN); })
#else
// Use randomness to prevent threads from resonating at the same
// frequency and permanently contending.
#define _dispatch_contention_spins() ({ \
		((unsigned int)rand() & (DISPATCH_CONTENTION_SPINS_MAX)) | \
				(DISPATCH_CONTENTION_SPINS_MIN); })
#endif
#define _dispatch_contention_wait_until(c) ({ \
		bool _out = false; \
		unsigned int _spins = _dispatch_contention_spins(); \
		while (_spins--) { \
			dispatch_hardware_pause(); \
			if (likely(_out = (c))) break; \
		}; _out; })
#endif

#pragma mark -
#pragma mark dispatch_hardware_pause

#if defined(__x86_64__) || defined(__i386__)
#define dispatch_hardware_pause() __asm__("pause")
#elif (defined(__arm__) && defined(_ARM_ARCH_7) && defined(__thumb__)) || \
		defined(__arm64__)
#define dispatch_hardware_pause() __asm__("yield")
#define dispatch_hardware_wfe()   __asm__("wfe")
#else
#define dispatch_hardware_pause() __asm__("")
#endif

#pragma mark -
#pragma mark _dispatch_preemption_yield

/* Don't allow directed yield to enqueuer if !_pthread_has_direct_tsd() */
#ifndef DISPATCH_HAVE_YIELD_TO_ENQUEUER
#if PTHREAD_HAVE_YIELD_TO_ENQUEUER && !TARGET_OS_SIMULATOR
#define DISPATCH_HAVE_YIELD_TO_ENQUEUER 1
#else
#define DISPATCH_HAVE_YIELD_TO_ENQUEUER 0
#endif
#endif /* DISPATCH_HAVE_YIELD_TO_ENQUEUER */

#if HAVE_MACH
#if defined(SWITCH_OPTION_OSLOCK_DEPRESS)
#define DISPATCH_YIELD_THREAD_SWITCH_OPTION SWITCH_OPTION_OSLOCK_DEPRESS
#else
#define DISPATCH_YIELD_THREAD_SWITCH_OPTION SWITCH_OPTION_DEPRESS
#endif

#define _dispatch_preemption_yield(n) thread_switch(MACH_PORT_NULL, \
		DISPATCH_YIELD_THREAD_SWITCH_OPTION, (mach_msg_timeout_t)(n))
#define _dispatch_preemption_yield_to(th, n) thread_switch(th, \
		DISPATCH_YIELD_THREAD_SWITCH_OPTION, (mach_msg_timeout_t)(n))
#elif HAVE_PTHREAD_YIELD_NP
#define _dispatch_preemption_yield(n) { (void)n; pthread_yield_np(); }
#define _dispatch_preemption_yield_to(th, n) { (void)n; pthread_yield_np(); }
#elif defined(_WIN32)
#define _dispatch_preemption_yield(n) { (void)n; Sleep(0); }
#define _dispatch_preemption_yield_to(th, n) { (void)n; Sleep(0); }
#else
#define _dispatch_preemption_yield(n) { (void)n; sched_yield(); }
#define _dispatch_preemption_yield_to(th, n) { (void)n; sched_yield(); }
#endif // HAVE_MACH

#if DISPATCH_HAVE_YIELD_TO_ENQUEUER
#define _dispatch_set_enqueuer_for(ptr) \
		_dispatch_thread_setspecific(dispatch_enqueue_key, (void *) (ptr));
#define _dispatch_clear_enqueuer() \
		_dispatch_thread_setspecific(dispatch_enqueue_key, NULL);
#define _dispatch_yield_to_enqueuer(q, n) \
		(void) _pthread_yield_to_enqueuer_4dispatch(dispatch_enqueue_key, q, n)

#define _dispatch_wait_for_enqueuer_until(q, c) ({ \
		__typeof__(c) _c; \
		int _spins = -(DISPATCH_WAIT_SPINS); \
		for (;;) { \
			if (likely(_c = (c))) break; \
			if (unlikely(_spins++ >= 0)) { \
				_dispatch_yield_to_enqueuer((void *) (q), (unsigned int) (_spins)); \
			} else { \
				dispatch_hardware_pause(); \
			} \
		} \
		_c; })

#else
#define _dispatch_set_enqueuer_for(ptr)
#define _dispatch_clear_enqueuer(ptr)
#define _dispatch_yield_to_enqueuer(q, n) \
		((void) (q), _dispatch_preemption_yield(n))
#endif /* DISPATCH_HAVE_YIELD_TO_ENQUEUER */

#if DISPATCH_HAVE_YIELD_TO_ENQUEUER
#define FIREHOSE_YIELD_TO_ENQUEUER 1
#else
#define FIREHOSE_YIELD_TO_ENQUEUER 0
#endif /* FIREHOSE_YIELD_TO_ENQUEUER */

#pragma mark -
#pragma mark _dispatch_contention_usleep

#ifndef DISPATCH_CONTENTION_USLEEP_START
#if defined(_WIN32)
#define DISPATCH_CONTENTION_USLEEP_START 1000   // Must be >= 1ms for Sleep()
#else
#define DISPATCH_CONTENTION_USLEEP_START 500
#endif
#endif
#ifndef DISPATCH_CONTENTION_USLEEP_MAX
#define DISPATCH_CONTENTION_USLEEP_MAX 100000
#endif

#if HAVE_MACH
#if defined(SWITCH_OPTION_DISPATCH_CONTENTION)
#define _dispatch_contention_usleep(u) thread_switch(MACH_PORT_NULL, \
		SWITCH_OPTION_DISPATCH_CONTENTION, (u))
#else
#define _dispatch_contention_usleep(u) thread_switch(MACH_PORT_NULL, \
		SWITCH_OPTION_WAIT, (((u)-1)/1000)+1)
#endif
#else
#if defined(_WIN32)
#define _dispatch_contention_usleep(u) Sleep((u) / 1000)
#else
#define _dispatch_contention_usleep(u) usleep((u))
#endif
#endif // HAVE_MACH

#endif // __DISPATCH_SHIMS_YIELD__
