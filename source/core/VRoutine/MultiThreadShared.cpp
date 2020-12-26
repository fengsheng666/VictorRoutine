// Copyright (c) 2020 FengSheng(EN. Victor Fung)
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
		TaskQueue() 
			: AtomicQueueBase(VROUTINE_TASK_QUEUE_MAX_LENGTH) { }
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

bool MultiThreadShared::preempt(Task* task)
{
	AtomicQueueBase::AtomicQueueItem* ti = task->getQueueNode();
	VROUTINE_CHECKER(task == ti->_ptr);
	while (!m_taskQueue->append(ti, ti, 1)) { }
	volatile const Task* head = NULL;
	m_taskQueue->front([&](const void* ptr){
		head = (const Task*)ptr;
	});
	if (head != task)
	{
		return false;
	}
	bool expected = false;
	if (!m_preemptFlag.compare_exchange_strong(expected, true))
	{
		return false;
	}
	m_sharedCount.fetch_add(1);
	Task* item = (Task*)m_taskQueue->pop()->_ptr;
	VROUTINE_CHECKER(item == task);
	return true;
}

void MultiThreadShared::release(Dispatcher* dispatcher)
{
	if (m_sharedCount.fetch_sub(1) > 1)
	{
		return;
	}

	bool expected = false;
	do 
	{
		AtomicQueueBase::AtomicQueueItem* pi = m_taskQueue->pop();
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
						return ((Task*)ptr)->exclusiveOnObject(currentObject) == false;});
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
				std::function<void()> schedule = [item, dispatcher, currentObject](){
					bool task_run_success = item->execute(currentObject, dispatcher);
					VROUTINE_CHECKER(task_run_success);
				};
				if (dispatcher)
				{
					if (!dispatcher->post(schedule))
					{
						printf("dispatcher post task fault !!!\n");
						schedule();
					}
				}
				else
				{
					schedule();
				}
			}
			
			return;
		}
		//将 m_preemptFlag 复位后，若无新task 就退出、反之进行抢占式调度
		m_preemptFlag.store(false);
		volatile const Task* head = NULL;
		m_taskQueue->front([&](const void* ptr){
			head = (const Task*)ptr;
		});
		if (head == NULL)
		{
			return;
		}
		expected = false;
	} while (m_preemptFlag.compare_exchange_strong(expected, true));
}