/*
Copyright(C) 2015 Alexander Vasilevsky aka kalaider

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * The header contains definitions for worker thread queue.
 * 
 * Such queue may be used to create executor service, or thread
 * pool, with any number of worker threads.
 */

#pragma once

#include <list>
#include "drawable.h"

kScienceNSHeader()

namespace thread {

	//====================================================================
	//===================== Class Task ===================================
	//====================================================================

	/**
	 * The class defines a task that is to be performed in worker thread.
	 * 
	 * The task contains its owner ID and task ID. These IDs are
	 * used by queue pushers to determine if the queue already
	 * contains such task and to deal with that in right way.
	 */
	class Task
	{

	private: // Fields ===================================================

		const int ownerId;
		const int taskId;

	protected: // Constructors ===========================================

		Task(int ownerId, int taskId)
			: ownerId(ownerId), taskId(taskId)
		{
		}

	public: // Methods (Accessors) =======================================

		int getOwnerId() const throw ()
		{
			return ownerId;
		}

		int getTaskId() const throw ()
		{
			return taskId;
		}

	public: // Methods ===================================================

		/**
		 * Does all the job.
		 * 
		 * The method is invoked in a task consumer thread.
		 */
		virtual void doTask() throw ()
		{
		}

		/**
		 * Does all the job to cancel the task.
		 * 
		 * The method is invoked in a thread that caused
		 * this task to be detached (task consumer thread;
		 * e.g. by pushing another task to the queue with
		 * some special rule).
		 */
		virtual void taskDetached() throw ()
		{
		}

		/**
		 * Does all the job to release all resources allocated
		 * by this task.
		 * 
		 * The method may be invoked either in task producer
		 * or task consumer thread.
		 * 
		 * If the task is self-managed, this method must do
		 * 'delete this' (done by default).
		 */
		virtual void taskDone(bool detached) throw ()
		{
			delete this;
		}
	};

	//====================================================================
	//===================== Class Queue ==================================
	//====================================================================

	/**
	 * The class implements the simple producer(s)-consumer(s) thread
	 * queue. Such a queue can be used to implement thread pool,
	 * or executor service, for any number of worker threads.
	 */
	class Queue
	{

	public: // Static Inner Classes ======================================

		/**
		 * The class provides access to the internal structure
		 * of the queue.
		 */
		static class Accessor
		{
		private:
			Queue &queue;
		public:
			Accessor(Queue &queue) : queue(queue)
			{
			}
		public:
			virtual std::list<Task *>& getTasks()
			{
				return queue.tasks;
			};
		};

		friend class Accessor;

		/**
		 * The class determines a rule used to push the task
		 * to the queue.
		 * 
		 * E.g., the predefined UniqueOwnerRule detaches all the
		 * tasks with the same owner ID before pushing the given one.
		 */
		static class PushingRule
		{
		public:
			static PushingRule
				// Appends the task to the end of the queue
				*AppendRule,
				// Inserts the task in a first position of the queue
				*PrependRule,
				// Filters the queue detaching all the tasks
				// with the same owner ID, inserts the given
				// task at the position of the closest to the
				// queue head detached task or adds the task
				// to the end of the queue
				*UniqueOwnerRule,
				// Differs from UniqueOwnerRule only in
				// this rule filters by task ID
				*UniqueTaskRule,
				// Differs from UniqueOwnerRule only in
				// this rule detaches only the tasks which
				// owner ID and task ID both equal
				// those of the given task
				*UniqueOwnerTaskRule;
		public:
			virtual void doPush(Accessor &queue, Task *task) throw () = 0;
		};

		/**
		 * The class is able to walk through the queue.
		 */
		static class Walker
		{
		public:
			virtual void doWalk(Accessor &queue) throw () = 0;
		};

	protected: // Fields (Multithreading) ================================
		CRITICAL_SECTION queueLocker;
		CONDITION_VARIABLE queueWaiter;

	protected: // Fields (Iternal) =======================================
		Accessor *accessor;

	protected: // Fields =================================================
		std::list<Task *> tasks;
		bool closed;

	public: // Constructors ==============================================

		Queue() : accessor(new Accessor(*this)), closed(false)
		{
			InitializeCriticalSection(&queueLocker);
			InitializeConditionVariable(&queueWaiter);
		}

	public: // Methods ===================================================

		/**
		 * The method pushes the given task to the queue.
		 * 
		 * If the queue is closed, false is returned.
		 * 
		 * The methods locks on the queue lock and notifies
		 * all the threads waiting on 'waitForTasks' method.
		 * 
		 * @param task        The task to be pushed
		 * @param pushingRule The rule to be used to push the task
		 * 
		 * @return false if queue is closed, true otherwise
		 */
		virtual bool push
			(
			Task *task,
			PushingRule *pushingRule = PushingRule::AppendRule
			)
			throw ();

		/**
		 * Retrieves and returns the heading task of the queue.
		 * 
		 * If the queue is empty, returns false. Returns true
		 * and modifies the output parameter 'task' otherwise.
		 *
		 * The methods locks on the queue lock.
		 * 
		 * @param task The output parameter - the task to be returned
		 * 
		 * @return true if the task was retrieved (the queue was not
		 *         empty), false otherwise
		 */
		virtual bool pop(Task **task) throw ();

		/**
		 * The method allows any thread to modify the queue
		 * under the queue lock (in thread-safe manner).
		 * 
		 * None, that the walker has an exclusive right to
		 * fully modify the queue, even when it is closed.
		 *
		 * The methods locks on the queue lock.
		 * 
		 * @param walker The object to be used to walk through the queue
		 */
		virtual void walk(Walker *walker) throw ();

		/**
		 * The method allows the thread to suspend its execution
		 * till the queue is empty.
		 * 
		 * If the queue is not empty, the method immediately returns.
		 * 
		 * Otherwise, it suspends the thread until another thread
		 * calls 'push'.
		 * 
		 * The methods locks on the queue lock.
		 * 
		 * @return false if queue is closed and empty, true otherwise
		 */
		virtual bool wait() throw ();

		/**
		 * The method closes the queue.
		 *
		 * Clears the queue if requested.
		 *
		 * The methods locks on the queue lock.
		 */
		virtual void close(bool clear) throw ();

		/**
		 * Returns the queue state.
		 * 
		 * The methods locks on the queue lock.
		 */
		virtual bool isClosed();

	public: // Destructor ================================================

		/**
		 * The destructor wakes all the threads waiting on the queue lock.
		 * It also cancels (detaches) all tasks in the queue.
		 */
		virtual ~Queue() throw ();

	};

}

kScienceNSFooter()