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
#include <thread>
#include <atomic>
#include <VRoutine/inner/AtomicQueueBase.h>
#include <VRoutine/Dispatcher.h>
#include <VRoutine/Routine.h>

#if defined(VROUTINE_DISPATCH_THREAD_COUNT) && (VROUTINE_DISPATCH_THREAD_COUNT > 0)
namespace VictorRoutine
{
	class DispatcherQueue : public AtomicQueueBase
	{
	public:
		struct FuncInfo
		{
			FuncInfo() : _queueNode(NULL, this) { }
			AtomicQueueBase::AtomicQueueItem	_queueNode;
			std::function<void()>				_function;
		};
		DispatcherQueue()
		{
			m_validFlag.store(true);
		}
		~DispatcherQueue()
		{

		}
		void close()
		{
			m_validFlag.store(false);
		}
		bool append(const std::function<void()>& func)
		{
			if (!m_validFlag.load())
			{
				return false;
			}
			FuncInfo* fi = new FuncInfo;
			if (fi == NULL)
			{
				return false;
			}
			fi->_function = func;
			AtomicQueueBase::append(&fi->_queueNode, &fi->_queueNode, 1);
			return true;
		}
		FuncInfo* pop()
		{
			AtomicQueueBase::AtomicQueueItem* item = AtomicQueueBase::pop();
			if (!item)
			{
				return NULL;
			}
			return (FuncInfo*)item->_ptr;
		}
	private:
		std::atomic<bool>	m_validFlag;
	};
	class DefaultDispatcher : public Dispatcher, public RefObject
	{
	public:
		
		DefaultDispatcher()
		{
			m_execCount.store(0);
			m_threadNum.store(0);
		}
		~DefaultDispatcher() { }
		void shutdown()
		{
			m_funcQueue.close();
			while (m_threadNum.load() > 0) 
			{
				printf("wait default dispatch thread exit!!!\n");
			}
		}
		virtual bool post(std::function<void()> func)
		{
			if (!m_funcQueue.append(func))
			{
				return false;
			}
			int oldCount = m_execCount.fetch_add(1);
			if (oldCount + 1 <= VROUTINE_DISPATCH_THREAD_COUNT)
			{
				m_threadNum.fetch_add(1);
				StrongPtr<DefaultDispatcher> dp = this;
				std::thread background_thread(ExecuteThread, dp);
				background_thread.detach();
			}
			else
			{
				m_execCount.fetch_sub(1);
			}
			return true;
		}
	private:
		static void ExecuteThread(StrongPtr<DefaultDispatcher> dp)
		{
			DispatcherQueue::FuncInfo* fi = dp->m_funcQueue.pop();
			while (true)
			{
				for (; fi != NULL; fi = dp->m_funcQueue.pop())
				{
					fi->_function();
					delete fi;
				}
				dp->m_execCount.fetch_sub(1);

				std::this_thread::yield(); //ÇÐ³öcpu

				fi = dp->m_funcQueue.pop();
				if (fi == NULL)
				{
					break;
				}
				dp->m_execCount.fetch_add(1);
			}
			dp->m_threadNum.fetch_sub(1);
		}
	private:
		DispatcherQueue		m_funcQueue;
		//m_execCount >= m_threadNum
		std::atomic<int>	m_execCount;

		std::atomic<int>	m_threadNum;
	};
	class DefaultDispatcherWrapper
	{
	public:
		DefaultDispatcherWrapper()
		{
			m_dispatcher = new DefaultDispatcher;
			VROUTINE_CHECKER(m_dispatcher != NULL);
		}
		~DefaultDispatcherWrapper()
		{
			m_dispatcher->shutdown();
		}
		Dispatcher* getDefaultDispatcher()
		{
			return m_dispatcher.get();
		}
	private:
		StrongPtr<DefaultDispatcher> m_dispatcher;
	};
}

VictorRoutine::DefaultDispatcherWrapper g_defualtDW;

#endif //VROUTINE_DISPATCH_THREAD_COUNT

using namespace VictorRoutine;

Routine::Routine(std::function<void()> func) : m_task(new Task(func))
{

}

Routine::~Routine()
{
	if (m_task)
	{
		delete m_task;
	}
}

void* Routine::operator new(size_t size)
{
	return malloc(size);
}

void Routine::operator delete(void* ptr)
{
	if (ptr)
	{
		free(ptr);
	}
}

Routine& Routine::addDependence(MultiThreadShared* obj, bool bExclusive)
{
	m_deps[obj] = bExclusive;
	return *this;
}

bool Routine::go(Dispatcher* dispatcher, int maxDepth)
{
	if (m_task == NULL)
	{
		return false;
	}
	if (maxDepth <= 0)
	{
		m_task->setMaxDepth(m_deps.size());
	}
	if (m_task->schedules().size() != m_deps.size())
	{
		m_task->schedules().resize(m_deps.size());
		std::map<MultiThreadShared*, bool>::const_iterator iter = m_deps.begin();
		for (int i = 0; i < m_task->schedules().size(); i++, iter++)
		{
			(m_task->schedules())[i].m_object = iter->first;
			(m_task->schedules())[i].m_bExclusive = iter->second;
		}
	}
#if defined(VROUTINE_DISPATCH_THREAD_COUNT) && (VROUTINE_DISPATCH_THREAD_COUNT > 0)
	if (NULL == dispatcher)
	{
		dispatcher = g_defualtDW.getDefaultDispatcher();
	}
#endif
	if (m_task->execute(NULL, dispatcher, 0))
	{
		m_task = NULL;
		return true;
	}
	else
	{
		return false;
	}
}