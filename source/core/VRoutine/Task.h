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

#ifndef TASK_H_VROUTINE
#define TASK_H_VROUTINE

#include <VRoutine/VRoutineDef.h>
#include <VRoutine/StrongPtr.h>
#include <VRoutine/inner/AtomicQueueBase.h>
#include <VRoutine/MultiThreadShared.h>
#include <vector>
#include <functional>

namespace VictorRoutine
{
	class Dispatcher;

	class Task 
	{
	public:
		struct ScheduleInfo
		{ 
			ScheduleInfo() : m_object(0), m_bExclusive(true) { }
			ScheduleInfo(MultiThreadShared* obj, bool bExclusive) 
				: m_object(obj), m_bExclusive(bExclusive) { }
			~ScheduleInfo() 
			{
				m_object = 0;
			}
			StrongPtr<MultiThreadShared>	m_object; 
			bool							m_bExclusive;
		};
		inline Task(std::function<void()> func)
			: m_function(func), m_blockPos(-1), m_queueNode(NULL, this)
		{
			m_queueNode._ptr = this;
		}
		~Task() { }
		inline AtomicQueueBase::AtomicQueueItem* getQueueNode()
		{
			return &m_queueNode;
		}
		inline std::vector<ScheduleInfo>& schedules()
		{
			return m_schedules;
		}
		inline bool exclusiveOnObject(MultiThreadShared* obj) const
		{
			VROUTINE_CHECKER(m_blockPos >= 0 && m_blockPos < m_schedules.size());
			VROUTINE_CHECKER(m_schedules[m_blockPos].m_object == obj);
			return m_schedules[m_blockPos].m_bExclusive;
		}
		bool execute(MultiThreadShared* obj, Dispatcher* dispatcher);
#if defined(VROUTINE_TASK_POOL_CACHE_COUNT) && (VROUTINE_TASK_POOL_CACHE_COUNT > 0)
		void* operator new(size_t size);
		void operator delete(void* ptr);
#endif //VROUTINE_TASK_POOL_CACHE_COUNT
	private: 
		AtomicQueueBase::AtomicQueueItem	m_queueNode;
		std::function<void()>				m_function;
		std::vector<ScheduleInfo>			m_schedules;
		int									m_blockPos;
	};
}

#endif