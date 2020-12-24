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
#include <VRoutine/AtomicQueue.h>
#include <VRoutine/Dispatcher.h>

namespace VictorRoutine
{
	class TaskQueue : public AtomicQueue<Task>
	{
	public:
		TaskQueue() : AtomicQueue<Task>(VROUTINE_TASK_QUEUE_MAX_LENGTH) { }
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
	m_taskQueue->append(task);
	volatile const Task* head = NULL;
	m_taskQueue->front([&](const Task* item){
		head = item;
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
	Task* item = m_taskQueue->pop([](Task* item){
		return true;
	});
	assert(item == task);
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
		Task* first = m_taskQueue->pop([](Task* item){
			return true;
		});
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
				for (Task* next = m_taskQueue->pop([currentObject](Task* item){
							return item->exclusiveOnObject(currentObject) == false;
						});
					next != NULL;
					next = m_taskQueue->pop([currentObject](Task* item){
							return item->exclusiveOnObject(currentObject) == false;
						}) )
				{
					tasks.push_back(next);
					m_sharedCount.fetch_add(1);
				}
				
			}
			
			std::list<Task*>::const_iterator iter = tasks.begin();
			for (; iter != tasks.end(); iter++)
			{
				Task* item = *iter;
				std::function<void()> schedule = [item, dispatcher, currentObject](){
					bool task_run_success = item->execute(currentObject, dispatcher);
					assert(task_run_success);
				};
				if (!dispatcher || !dispatcher->post(schedule))
				{
					schedule();
				}
			}
			
			return;
		}
		//将 m_preemptFlag 复位后，若无新task 就退出、反之进行抢占式调度
		m_preemptFlag.store(false);
		volatile const Task* head = NULL;
		m_taskQueue->front([&](const Task* item){
			head = item;
		});
		if (head == NULL)
		{
			return;
		}
		expected = false;
	} while (m_preemptFlag.compare_exchange_strong(expected, true));
}