// Copyright 2019, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Wrapper around OS threading native functions.
 * @author Jakob Bornecrantz <jakob@collabora.com>
 *
 * @ingroup aux_os
 */

#pragma once

#include "xrt/xrt_compiler.h"
#include "xrt/xrt_config_os.h"

#include "util/u_misc.h"

#include "os/os_time.h"

#if defined(XRT_OS_LINUX)
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#elif defined(XRT_OS_WINDOWS)
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <assert.h>
#else
#error "OS not supported"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * @addtogroup aux_os
 * @{
 */

/*
 *
 * Mutex
 *
 */

/*!
 * A wrapper around a native mutex.
 */
struct os_mutex
{
	pthread_mutex_t mutex;
};

/*!
 * Init.
 */
static inline int
os_mutex_init(struct os_mutex *om)
{
	return pthread_mutex_init(&om->mutex, NULL);
}

/*!
 * Lock.
 */
static inline void
os_mutex_lock(struct os_mutex *om)
{
	pthread_mutex_lock(&om->mutex);
}

static inline int
os_mutex_trylock(struct os_mutex *om)
{
	return pthread_mutex_trylock(&om->mutex);
}

/*!
 * Unlock.
 */
static inline void
os_mutex_unlock(struct os_mutex *om)
{
	pthread_mutex_unlock(&om->mutex);
}

/*!
 * Clean up.
 */
static inline void
os_mutex_destroy(struct os_mutex *om)
{
	pthread_mutex_destroy(&om->mutex);
}


/*
 *
 * Conditional variable.
 *
 */
struct os_cond
{
	pthread_cond_t cond;
};

/*!
 * Init.
 */
static inline int
os_cond_init(struct os_cond *oc)
{
	return pthread_cond_init(&oc->cond, NULL);
}

/*!
 * Signal.
 */
static inline void
os_cond_signal(struct os_cond *oc)
{
	pthread_cond_signal(&oc->cond);
}

/*!
 * Wait.
 */
static inline void
os_cond_wait(struct os_cond *oc, struct os_mutex *om)
{
	pthread_cond_wait(&oc->cond, &om->mutex);
}

/*!
 * Clean up.
 */
static inline void
os_cond_destroy(struct os_cond *oc)
{
	pthread_cond_destroy(&oc->cond);
}



/*
 *
 * Thread.
 *
 */

/*!
 * A wrapper around a native thread.
 */
struct os_thread
{
	pthread_t thread;
};

/*!
 * Run function.
 */
typedef void *(*os_run_func)(void *);

/*!
 * Init.
 */
static inline int
os_thread_init(struct os_thread *ost)
{
	return 0;
}

/*!
 * Start thread.
 */
static inline int
os_thread_start(struct os_thread *ost, os_run_func func, void *ptr)
{
	return pthread_create(&ost->thread, NULL, func, ptr);
}

/*!
 * Join.
 */
static inline void
os_thread_join(struct os_thread *ost)
{
	void *retval;

	pthread_join(ost->thread, &retval);
	U_ZERO(&ost->thread);
}

/*!
 * Destruction.
 */
static inline void
os_thread_destroy(struct os_thread *ost)
{}


/*
 *
 * Semaphore.
 *
 */

/*!
 * A wrapper around a native semaphore.
 */
struct os_semaphore
{
	sem_t sem;
};

/*!
 * Init.
 */
static inline int
os_semaphore_init(struct os_semaphore *os, int count)
{
	return sem_init(&os->sem, 0, count);
}

/*!
 * Release.
 */
static inline void
os_semaphore_release(struct os_semaphore *os)
{
	sem_post(&os->sem);
}

/*!
 * Set @p ts to the current time, plus the timeout_ns value.
 *
 * Intended for use by the threading code only: the timestamps are not interchangeable with other sources of time.
 */
static inline int
os_semaphore_get_realtime_clock(struct timespec *ts, uint64_t timeout_ns)
{
#if defined(XRT_OS_WINDOWS)
	struct timespec relative;
	os_ns_to_timespec(timeout_ns, &relative);
	pthread_win32_getabstime_np(ts, &relative);
	return 0;
#else
	struct timespec now;
	if (clock_gettime(CLOCK_REALTIME, &now) < 0) {
		assert(false);
		return -1;
	}
	uint64_t now_ns = os_timespec_to_ns(&now);
	uint64_t when_ns = timeout_ns + now_ns;

	os_ns_to_timespec(when_ns, ts);
	return 0;
#endif
}

/*!
 * Wait, if @p timeout_ns is zero then waits forever.
 */
static inline void
os_semaphore_wait(struct os_semaphore *os, uint64_t timeout_ns)
{
	if (timeout_ns == 0) {
		sem_wait(&os->sem);
		return;
	}

	struct timespec abs_timeout;
	if (os_semaphore_get_realtime_clock(&abs_timeout, timeout_ns) == -1) {
		assert(false);
	}

	sem_timedwait(&os->sem, &abs_timeout);
}

/*!
 * Clean up.
 */
static inline void
os_semaphore_destroy(struct os_semaphore *os)
{
	sem_destroy(&os->sem);
}


/*
 *
 * Fancy helper.
 *
 */

/*!
 * All in one helper that handles locking, waiting for change and starting a
 * thread.
 */
struct os_thread_helper
{
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	bool running;
};

/*!
 * Initialize the thread helper.
 *
 * @public @memberof os_thread_helper
 */
static inline int
os_thread_helper_init(struct os_thread_helper *oth)
{
	U_ZERO(oth);

	int ret = pthread_mutex_init(&oth->mutex, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = pthread_cond_init(&oth->cond, NULL);
	if (ret) {
		pthread_mutex_destroy(&oth->mutex);
		return ret;
	}

	return 0;
}

/*!
 * Start the internal thread.
 *
 * @public @memberof os_thread_helper
 */
static inline int
os_thread_helper_start(struct os_thread_helper *oth, os_run_func func, void *ptr)
{
	pthread_mutex_lock(&oth->mutex);

	if (oth->running) {
		pthread_mutex_unlock(&oth->mutex);
		return -1;
	}

	int ret = pthread_create(&oth->thread, NULL, func, ptr);
	if (ret != 0) {
		pthread_mutex_unlock(&oth->mutex);
		return ret;
	}

	oth->running = true;

	pthread_mutex_unlock(&oth->mutex);

	return 0;
}

/*!
 * @brief Signal from within the thread that we are stopping.
 *
 * Call with mutex unlocked - it takes and releases the lock internally.
 *
 * @public @memberof os_thread_helper
 */
static inline int
os_thread_helper_signal_stop(struct os_thread_helper *oth)
{
	// The fields are protected.
	pthread_mutex_lock(&oth->mutex);

	// Report we're stopping the thread.
	oth->running = false;

	// Wake up any waiting thread.
	pthread_cond_signal(&oth->cond);

	// No longer need to protect fields.
	pthread_mutex_unlock(&oth->mutex);

	return 0;
}

/*!
 * @brief Stop the thread and wait for it to exit.
 *
 * Call with mutex unlocked - it takes and releases the lock internally.
 *
 * @public @memberof os_thread_helper
 */
static inline int
os_thread_helper_stop(struct os_thread_helper *oth)
{
	void *retval = NULL;

	// The fields are protected.
	pthread_mutex_lock(&oth->mutex);

	if (!oth->running) {
		pthread_mutex_unlock(&oth->mutex);
		return 0;
	}

	// Stop the thread.
	oth->running = false;

	// Wake up the thread if it is waiting.
	pthread_cond_signal(&oth->cond);

	// No longer need to protect fields.
	pthread_mutex_unlock(&oth->mutex);

	// Wait for thread to finish.
	pthread_join(oth->thread, &retval);

	return 0;
}

/*!
 * Destroy the thread helper, externally synchronizable.
 *
 * @public @memberof os_thread_helper
 */
static inline void
os_thread_helper_destroy(struct os_thread_helper *oth)
{
	// Stop the thread.
	os_thread_helper_stop(oth);

	// Destroy resources.
	pthread_mutex_destroy(&oth->mutex);
	pthread_cond_destroy(&oth->cond);
}

/*!
 * Lock the helper.
 *
 * @public @memberof os_thread_helper
 */
static inline void
os_thread_helper_lock(struct os_thread_helper *oth)
{
	pthread_mutex_lock(&oth->mutex);
}

/*!
 * Unlock the helper.
 *
 * @public @memberof os_thread_helper
 */
static inline void
os_thread_helper_unlock(struct os_thread_helper *oth)
{
	pthread_mutex_unlock(&oth->mutex);
}

/*!
 * Is the thread running, or supposed to be running.
 *
 * Call with mutex unlocked - it takes and releases the lock internally.
 * If you already have a lock, use os_thread_helper_is_running_locked().
 *
 * @public @memberof os_thread_helper
 */
static inline bool
os_thread_helper_is_running(struct os_thread_helper *oth)
{
	os_thread_helper_lock(oth);
	bool ret = oth->running;
	os_thread_helper_unlock(oth);
	return ret;
}

/*!
 * Is the thread running, or supposed to be running.
 *
 * Must be called with the helper locked.
 * If you don't have the helper locked for some other reason already,
 * you can use os_thread_helper_is_running()
 *
 * @public @memberof os_thread_helper
 */
static inline bool
os_thread_helper_is_running_locked(struct os_thread_helper *oth)
{
	return oth->running;
}

/*!
 * Wait for a signal.
 *
 * Must be called with the helper locked.
 *
 * @public @memberof os_thread_helper
 */
static inline void
os_thread_helper_wait_locked(struct os_thread_helper *oth)
{
	pthread_cond_wait(&oth->cond, &oth->mutex);
}

/*!
 * Signal a waiting thread to wake up.
 *
 * Must be called with the helper locked.
 *
 * @public @memberof os_thread_helper
 */
static inline void
os_thread_helper_signal_locked(struct os_thread_helper *oth)
{
	pthread_cond_signal(&oth->cond);
}


/*!
 * @}
 */


#ifdef __cplusplus
} // extern "C"
#endif


#ifdef __cplusplus
namespace xrt::auxiliary::os {


//! A class owning an @ref os_mutex
class Mutex
{
public:
	//! Construct a mutex
	Mutex() noexcept
	{
		os_mutex_init(&inner_);
	}
	//! Destroy a mutex when it goes out of scope
	~Mutex()
	{
		os_mutex_destroy(&inner_);
	}

	//! Block until the lock can be taken.
	void
	lock() noexcept
	{
		os_mutex_lock(&inner_);
	}

	//! Take the lock and return true if possible, but do not block
	bool
	try_lock() noexcept
	{
		return 0 == os_mutex_trylock(&inner_);
	}

	//! Release the lock
	void
	unlock() noexcept
	{
		os_mutex_unlock(&inner_);
	}

	//! Get a pointer to the owned mutex: do not delete it!
	os_mutex *
	get_inner() noexcept
	{
		return &inner_;
	}

	// Do not copy or delete these mutexes.
	Mutex(Mutex const &) = delete;
	Mutex(Mutex &&) = delete;
	Mutex &
	operator=(Mutex const &) = delete;
	Mutex &
	operator=(Mutex &&) = delete;

private:
	os_mutex inner_{};
};

} // namespace xrt::auxiliary::os

#endif // __cplusplus
