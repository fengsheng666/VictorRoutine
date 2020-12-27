// Copyright (c) 2020 Feng Sheng(EN. Victor Fung)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <list>
#include <VRoutine/MultiThreadShared.h>
#include "./Task.h"
#include <VRoutine/inner/AtomicQueueBase.h>
#include <VRoutine/Dispatcher.h>

namespace VictorRoutine
{
	class TaskQueue : public AtomicQueueBase
	{
	public:
		TaskQueue() { }
		~TaskQueue(){ }
	};
}

using namespace VictorRoutine;

MultiThreadShared::MultiThreadShared() : m_taskQueue(new TaskQueue) 
{
	m_preemptFlag.store(false);
	m_sharedCount.store(0);
}

MultiThreadShared::~MultiThreadShared()
{
	delete m_taskQueue;
}

void MultiThreadShared::preempt(Task* task, Dispatcher* dispatcher, int depth)
{
	AtomicQueueBase::AtomicQueueItem* ti = task->getQueueNode();
	VROUTINE_CHECKER(task == ti->_ptr);
	m_taskQueue->append(ti, ti, 1);

	return schedule(dispatcher, depth + 1);
}

void MultiThreadShared::release(Dispatcher* dispatcher, int depth)
{
	if (m_sharedCount.fetch_sub(1) > 1)
	{
		return;
	}

	return schedule(dispatcher, depth + 1);
}

void MultiThreadShared::schedule(Dispatcher* dispatcher, int depth)
{
	bool expected = false;
	if (!m_preemptFlag.compare_exchange_strong(expected, true))
	{
		return;
	}
	AtomicQueueBase::AtomicQueueItem* pi = m_taskQueue->pop();
	while (true)
	{
		Task* first = (pi ? (Task*)pi->_ptr : NULL);
		if (first)
		{
			MultiThreadShared* currentObject = this;
			std::list<Task*> tasks;
			if (first->exclusiveOnObject(this))
			{
				m_sharedCount.fetch_add(1);
				tasks.push_back(first);
			}
			else
			{
				m_sharedCount.fetch_add(1);
				tasks.push_back(first);
				for (AtomicQueueBase::AtomicQueueItem* next = m_taskQueue->pop([currentObject](void* ptr){
					return ((Task*)ptr)->exclusiveOnObject(currentObject) == false; });
						next != NULL;
					next = m_taskQueue->pop([currentObject](void* ptr){
						return ((Task*)ptr)->exclusiveOnObject(currentObject) == false; }))
				{
					tasks.push_back((Task*)next->_ptr);
					m_sharedCount.fetch_add(1);
				}

			}

			std::list<Task*>::const_iterator iter = tasks.begin();
			for (; iter != tasks.end(); iter++)
			{
				Task* item = *iter;
				if (item == first && depth <= item->getMaxDepth())
				{
					bool execute_task_success = item->execute(currentObject, dispatcher, depth);
					VROUTINE_CHECKER(execute_task_success);
				}
				else
				{
					std::function<void()> func = [item, dispatcher, currentObject, depth](){
						bool execute_task_success = item->execute(currentObject, dispatcher, 0);
						VROUTINE_CHECKER(execute_task_success);
					};
					if (!dispatcher || !dispatcher->post(func))
					{
						printf("dispatcher post task fault !!!\n");
						bool execute_task_success = item->execute(currentObject, dispatcher, depth);
						VROUTINE_CHECKER(execute_task_success);
					}
				}
			}

			int sharedCount = m_sharedCount.load();
			if (sharedCount == 0)
			{
				pi = m_taskQueue->pop();
				continue;
			}
			VROUTINE_CHECKER(sharedCount >= 0);
		}
		m_preemptFlag.store(false);
		if (m_sharedCount.load() > 0)
		{
			return;
		}
		volatile const Task* head = NULL;
		m_taskQueue->front([&](const void* ptr){
			head = (const Task*)ptr;
		});
		if (head == NULL)
		{
			return;
		}
		expected = false;
		if (!m_preemptFlag.compare_exchange_strong(expected, true))
		{
			return;
		}
		pi = m_taskQueue->pop();
	}
}
