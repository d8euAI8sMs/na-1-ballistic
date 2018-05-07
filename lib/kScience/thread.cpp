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

#include "thread.h"

namespace
{

	using namespace kScienceNS::thread;

	class AppendPrependPushingRule : public Queue::PushingRule
	{
	private:
		bool prepend;
	public:
		AppendPrependPushingRule(bool prepend) : prepend(prepend)
		{
		}
		virtual void doPush(Queue::Accessor &queue, Task *task) throw () override
		{
			if (!prepend) queue.getTasks().push_back(task);
			else queue.getTasks().push_front(task);
		}
	} _append_rule(false), _prepend_rule(true);

	class UniquePushingRule : public Queue::PushingRule
	{
	private:
		bool uniqueOwner, uniqueTask;
	public:
		UniquePushingRule(bool uniqueOwner, bool uniqueTask)
			: uniqueOwner(uniqueOwner), uniqueTask(uniqueTask)
		{
		}
		virtual void doPush(Queue::Accessor &queue, Task *task) throw () override
		{
			bool exists = false;
			
			std::list<Task *> &tasks = queue.getTasks();

			for (auto it = tasks.begin(); it != tasks.end(); it++)
			{
				bool lexists = false;

				if (uniqueOwner)
				{
					if ((*it)->getOwnerId() == task->getOwnerId())
					{
						if (uniqueTask)
						{
							if ((*it)->getTaskId() == task->getTaskId())
							{
								lexists = true;
							}
						}
						else
						{
							lexists = true;
						}
					}
				}
				else if (uniqueTask)
				{
					if ((*it)->getTaskId() == task->getTaskId())
					{
						lexists = true;
					}
				}

				if (lexists)
				{
					(*it)->taskDetached();
					(*it)->taskDone(true);
					if (!exists) (*it) = task;
					else
					{
						auto it0 = it; it--;
						tasks.erase(it0);
					}
					exists = true;
				}
			}
			
			if (!exists) tasks.push_back(task);
		}
	} _unique_owner_rule(true, false),
		_unique_task_rule(false, true),
		_unique_owner_task_rule(true, true);

}

Queue::PushingRule *Queue::PushingRule::AppendRule = &_append_rule;
Queue::PushingRule *Queue::PushingRule::PrependRule = &_prepend_rule;
Queue::PushingRule *Queue::PushingRule::UniqueOwnerRule = &_unique_owner_rule;
Queue::PushingRule *Queue::PushingRule::UniqueOwnerTaskRule = &_unique_owner_task_rule;
Queue::PushingRule *Queue::PushingRule::UniqueTaskRule = &_unique_task_rule;

bool Queue::push(
	Task *task,
	Queue::PushingRule *rule /* = Queue::PushingRule::AppendRule */
	)
	throw ()
{
	bool rr;

	EnterCriticalSection(&queueLocker);

	rr = closed;

	if (!closed)
	{
		rule->doPush(*accessor, task);
		WakeAllConditionVariable(&queueWaiter);
	}

	LeaveCriticalSection(&queueLocker);

	return rr;
}

void Queue::walk(Walker *walker) throw ()
{
	EnterCriticalSection(&queueLocker);

	walker->doWalk(*accessor);

	LeaveCriticalSection(&queueLocker);
}

bool Queue::pop(Task **task)
{
	EnterCriticalSection(&queueLocker);

	bool exists = false;

	if (!tasks.empty())
	{
		*task = tasks.front();
		exists = true;
		tasks.pop_front();
	}

	LeaveCriticalSection(&queueLocker);

	return exists;
}

bool Queue::wait()
{
	bool rr = true;

	EnterCriticalSection(&queueLocker);

	if (tasks.empty())
	{
		// Pre assertion (if changed already)
		if (closed)
		{
			rr = false;
		}
		else
		{
			SleepConditionVariableCS(
				&queueWaiter, &queueLocker, INFINITE);
		}
		// Post assertion (if changed while sleeping)
		if (closed && tasks.empty())
		{
			rr = false;
		}
	}

	LeaveCriticalSection(&queueLocker);

	return rr;
}

void Queue::close(bool clear)
{
	EnterCriticalSection(&queueLocker);

	this->closed = true;

	if (clear)
	{
		for (auto it = tasks.begin(); it != tasks.end(); it++)
		{
			(*it)->taskDetached();
			(*it)->taskDone(true);
		}
		tasks.clear();
	}

	WakeAllConditionVariable(&queueWaiter);

	LeaveCriticalSection(&queueLocker);
}

bool Queue::isClosed()
{
	bool rr;

	EnterCriticalSection(&queueLocker);

	rr = closed;

	LeaveCriticalSection(&queueLocker);

	return rr;
}

Queue::~Queue()
{
	close(true);
	delete accessor;
}