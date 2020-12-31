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

#include "./Task.h"

using namespace VictorRoutine;

#if defined(VROUTINE_TASK_POOL_CACHE_COUNT) && (VROUTINE_TASK_POOL_CACHE_COUNT > 0)

static std::atomic<int> g_taskNewCount(0);

namespace VictorRoutine
{
	class TaskPool : public AtomicQueueBase
	{
	public:
		TaskPool() 
		{

		}
		~TaskPool()
		{

		}
		Task* alloc()
		{
			AtomicQueueItem* item = AtomicQueueBase::pop();
			if (NULL == item)
			{
				return NULL;
			}
			return (Task*)item->_ptr;
		}
		void dealloc(Task* ptr)
		{
			VROUTINE_CHECKER(ptr != NULL);
			AtomicQueueItem* item = ptr->getQueueNode();
			AtomicQueueBase::append(item, item, 1);
		}
	};
}

VictorRoutine::TaskPool g_taskPool;

void* Task::operator new(size_t size)
{
	VROUTINE_CHECKER(sizeof(Task) == size);
	void* ptr = g_taskPool.alloc();
	if (ptr)
	{
		return ptr;
	}
	ptr = malloc(size);
	if (ptr == NULL)
	{
		return NULL;
	}
	g_taskNewCount.fetch_add(1);
	return ptr;
}

void Task::operator delete(void* ptr)
{
	if (!ptr)
	{
		return;
	}
	int oldCount = g_taskNewCount.fetch_sub(1);
	if (oldCount > VROUTINE_TASK_POOL_CACHE_COUNT)
	{
		free(ptr);
	}
	else
	{
		g_taskPool.dealloc((Task*)ptr);
		g_taskNewCount.fetch_add(1);
	}
}

#endif //VROUTINE_TASK_POOL_CACHE_COUNT

#if defined(VROUTINE_ACTIVE_TASK_MAX_COUNT) && (VROUTINE_ACTIVE_TASK_MAX_COUNT > 0)
static std::atomic<int> g_activeCount(0);
#endif // VROUTINE_ACTIVE_TASK_MAX_COUNT

bool Task::execute(MultiThreadShared* obj, Dispatcher* dispatcher, int depth)
{
	if (obj != NULL)
	{
		VROUTINE_CHECKER(m_blockPos >= 0 && m_blockPos < m_schedules.size());
		VROUTINE_CHECKER(m_schedules[m_blockPos].m_object == obj);
	}
	else
	{
#if defined(VROUTINE_ACTIVE_TASK_MAX_COUNT) && (VROUTINE_ACTIVE_TASK_MAX_COUNT > 0)
		int oldCount = g_activeCount.fetch_add(1);
		if (oldCount + 1 > VROUTINE_ACTIVE_TASK_MAX_COUNT)
		{
			g_activeCount.fetch_sub(1);
			return false;
		}
#endif // VROUTINE_ACTIVE_TASK_MAX_COUNT
	}

	m_blockPos++;
	if (m_blockPos < m_schedules.size())
	{
		m_schedules[m_blockPos].m_object->preempt(this, dispatcher, depth);
		return true;
	}

	m_function();

	for (int pos = m_schedules.size() - 1; pos >= 0; pos--)
	{
		m_schedules[pos].m_object->release(m_schedules[pos].m_bExclusive, dispatcher, depth);
	}

#if defined(VROUTINE_ACTIVE_TASK_MAX_COUNT) && (VROUTINE_ACTIVE_TASK_MAX_COUNT > 0)
	g_activeCount.fetch_sub(1);
#endif // VROUTINE_ACTIVE_TASK_MAX_COUNT
	delete this;
	return true;
}
