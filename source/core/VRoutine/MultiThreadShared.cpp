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

#include <condition_variable>
#include <mutex>
#include <list>
#include <thread>
#include <atomic>
#include <VRoutine/MultiThreadShared.h>
#include "./Task.h"
#include <VRoutine/inner/AtomicQueueBase.h>
#include <VRoutine/Dispatcher.h>


#if defined(VROUTINE_PARALLEL_THREAD_COUNT) && (VROUTINE_PARALLEL_THREAD_COUNT > 0)
namespace VictorRoutine
{
	class ParallelThreadPool : public AtomicQueueBase, public RefObject
	{
	public:
		struct ThreadInfo
		{
			ThreadInfo() : _queueNode(NULL, this) { }
			AtomicQueueBase::AtomicQueueItem	_queueNode;

			std::mutex							_mutex;
			std::condition_variable				_cond;

			Task*								_task;
			MultiThreadShared*					_object;
			Dispatcher*							_dispatcher;
		};
		ParallelThreadPool()
		{
			m_runFlag.store(true);
			m_thredNum.store(VROUTINE_PARALLEL_THREAD_COUNT);
		}
		~ParallelThreadPool()
		{

		}
		void startup()
		{
			for (int i = 0; i < VROUTINE_PARALLEL_THREAD_COUNT; i++)
			{
				ThreadInfo* ti = new ThreadInfo;
				if (ti == NULL)
				{
					printf("startup parallel thread %d fault!!!\n", i);
					continue;
				}
				StrongPtr<ParallelThreadPool> pool = this;
				std::thread background_thread(ParallelThread, pool, ti);
				background_thread.detach();
			}
		}
		void shutdown()
		{
			m_runFlag.store(false);

			while (m_thredNum.load() > 0)
			{
				printf("wait parallel thread exit!!!\n");
				AtomicQueueBase::AtomicQueueItem* item = AtomicQueueBase::pop();
				for (; item != NULL; item = AtomicQueueBase::pop())
				{
					ThreadInfo* ti = (ThreadInfo*)item->_ptr;
					ti->_cond.notify_one();
				}
			}
		}
		bool post(Task* task, MultiThreadShared* _obj, Dispatcher* dsp)
		{
			AtomicQueueBase::AtomicQueueItem* item = AtomicQueueBase::pop();
			if (!item)
			{
				return false;
			}
			ThreadInfo* ti = (ThreadInfo*)item->_ptr;
			ti->_task = task;
			ti->_object = _obj;
			ti->_dispatcher = dsp;

			std::unique_lock<std::mutex> lck(ti->_mutex);
			ti->_cond.notify_one();
			return true;
		}
	private:
		static void ParallelThread(StrongPtr<ParallelThreadPool> pool, ThreadInfo* ti)
		{
			while (pool->m_runFlag.load())
			{
				std::unique_lock<std::mutex> lck(ti->_mutex);
				pool->append(&ti->_queueNode, &ti->_queueNode, 1);
				ti->_cond.wait(lck);
				if (!pool->m_runFlag.load())
				{
					break;
				}
				VROUTINE_CHECKER(ti->_task != NULL && ti->_object != NULL);
				bool execute_task_success = ti->_task->execute(ti->_object, ti->_dispatcher, 0);
				VROUTINE_CHECKER(execute_task_success);
				ti->_task = NULL;
				ti->_object = NULL;
				ti->_dispatcher = NULL;
			}
			pool->m_thredNum.fetch_sub(1);
		}
	private:
		std::atomic<bool>	m_runFlag;
		std::atomic<int>	m_thredNum;
	};
	class ParallelThreadPoolWrapper
	{
	public:
		ParallelThreadPoolWrapper() { }
		~ParallelThreadPoolWrapper()
		{
			if (m_pool.valid())
			{
				//m_pool->shutdown();
			}
		}
		ParallelThreadPool* getThreadPool()
		{
			if (!m_pool.valid())
			{
				m_pool = new ParallelThreadPool;
				VROUTINE_CHECKER(m_pool != NULL);
				m_pool->startup();
			}
			return m_pool.get();
		}
	private:
		StrongPtr<ParallelThreadPool> m_pool;
	};
}
VictorRoutine::ParallelThreadPoolWrapper g_parallelThread;
#endif //VROUTINE_PARALLEL_THREAD_COUNT

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

	if (schedule(dispatcher, depth + 1))
	{
		return;
	}
	if (m_sharedCount.load() > 0)
	{
		MultiThreadShared* currentObject = this;
		AtomicQueueBase::AtomicQueueItem* pi = m_taskQueue->pop([currentObject](void* ptr){
			int expected = currentObject->m_sharedCount.load();
			return ((Task*)ptr)->exclusiveOnObject(currentObject) == false
				&& expected > 0
				&& currentObject->m_sharedCount.compare_exchange_strong(expected, expected + 1);
		});
		if (pi == NULL)
		{
			return;
		}
		Task* first = (Task*)pi->_ptr;
		bool execute_task_success = first->execute(currentObject, dispatcher, depth + 1);
		VROUTINE_CHECKER(execute_task_success);
	}
}

void MultiThreadShared::release(bool bExclusive, Dispatcher* dispatcher, int depth)
{
	int delta = (bExclusive ? -1 : 1);
	if (m_sharedCount.fetch_sub(delta) == delta)
	{
		m_preemptFlag.store(false);
		schedule(dispatcher, depth + 1);
	}
}

bool MultiThreadShared::schedule(Dispatcher* dispatcher, int depth)
{
	bool expected = false;
	if (!m_preemptFlag.compare_exchange_strong(expected, true))
	{
		return false;
	}
	AtomicQueueBase::AtomicQueueItem* pi = m_taskQueue->pop();
	Task* first = (pi ? (Task*)pi->_ptr : NULL);
	int delta = (first ? (first->exclusiveOnObject(this) ? -1 : 1) : 0);
	int initCount = m_sharedCount.fetch_add(delta);
	VROUTINE_CHECKER(initCount == 0);
	while (true)
	{
		if (first)
		{
			MultiThreadShared* currentObject = this;
			AtomicQueueBase::AtomicQueueItem* next = NULL;
#if defined(VROUTINE_PARALLEL_THREAD_COUNT) && (VROUTINE_PARALLEL_THREAD_COUNT > 0)
			if (delta > 0)
			{
				for (next = m_taskQueue->pop([currentObject](void* ptr){
						return ((Task*)ptr)->exclusiveOnObject(currentObject) == false; });
					next != NULL;
					next = m_taskQueue->pop([currentObject](void* ptr){
						return ((Task*)ptr)->exclusiveOnObject(currentObject) == false; }))
				{
					m_sharedCount.fetch_add(delta);
					if (!g_parallelThread.getThreadPool()->post((Task*)next->_ptr, this, dispatcher))
					{
						break;
					}
				}
			}
#endif // VROUTINE_PARALLEL_THREAD_COUNT
			if (next == NULL) m_sharedCount.fetch_add(delta); 
			if (depth <= first->getMaxDepth())
			{
				bool execute_task_success = first->execute(currentObject, dispatcher, depth);
				VROUTINE_CHECKER(execute_task_success);
			}
			else
			{
				std::function<void()> func = [first, dispatcher, currentObject](){
					bool execute_task_success = first->execute(currentObject, dispatcher, 0);
					VROUTINE_CHECKER(execute_task_success);
				};
				if (!dispatcher || !dispatcher->post(func))
				{
					printf("task execute in too depth : %d !!!\n", depth);
					bool execute_task_success = first->execute(currentObject, dispatcher, depth);
					VROUTINE_CHECKER(execute_task_success);
				}
			}

			if (next != NULL)
			{
				pi = next;
				first = (pi ? (Task*)pi->_ptr : NULL);
				continue;
			}
			int sharedCount = m_sharedCount.load();
			if (sharedCount == delta)
			{
				pi = m_taskQueue->pop([currentObject, delta](void* ptr){
					return ((Task*)ptr)->exclusiveOnObject(currentObject) == (delta < 0);
				});
				first = (pi ? (Task*)pi->_ptr : NULL);
				if (first)
				{
					continue;
				}
			}
			VROUTINE_CHECKER(sharedCount != 0);
		}

		if (m_sharedCount.fetch_sub(delta) != delta) //还有只读的task在其他线程执行
		{
			return true;
		}
		else
		{
			m_preemptFlag.store(false);
		}

		volatile const Task* head = NULL;
		m_taskQueue->front([&](const void* ptr){
			head = (const Task*)ptr;
		});
		if (head == NULL)
		{
			return true;
		}

		//重新抢占进入下一轮循环
		expected = false;
		if (!m_preemptFlag.compare_exchange_strong(expected, true))
		{
			return true;
		}
		pi = m_taskQueue->pop();
		first = (pi ? (Task*)pi->_ptr : NULL);
		delta = (first ? (first->exclusiveOnObject(this) ? -1 : 1) : 0);
		initCount = m_sharedCount.fetch_add(delta);
		VROUTINE_CHECKER(initCount == 0);
	}
	static bool exception_while_break = false;
	VROUTINE_CHECKER(exception_while_break);
	return true;
}
