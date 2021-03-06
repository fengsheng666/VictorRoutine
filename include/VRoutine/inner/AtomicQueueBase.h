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

#ifndef ATOMICQUEUEBASE_H_VROUTINE
#define ATOMICQUEUEBASE_H_VROUTINE

#include "../VRoutineDef.h"
#include <atomic>
#include <functional>

namespace VictorRoutine
{
	class VROUTINE_API AtomicQueueBase
	{
	public:
		struct AtomicQueueItem
		{
			AtomicQueueItem() : _ptr(0) 
			{
				_next.store(0);
			}
			AtomicQueueItem(AtomicQueueItem* next, void* ptr)
				: _ptr(ptr){
				_next.store(next);
			}
			inline AtomicQueueItem* getNext()
			{
				return _next.load();
			}
			inline void setNext(AtomicQueueItem* next)
			{
				_next.store(next);
			}
			std::atomic<AtomicQueueItem*>	_next;
			void*							_ptr;
		};
		~AtomicQueueBase();
		void append(AtomicQueueItem* begin, AtomicQueueItem* end, int length);
		AtomicQueueItem* pop();
		AtomicQueueItem* pop(std::function<bool(void*)> filterFunc);
		void front(std::function<void(const void*)> recver);
	protected:
		AtomicQueueBase();
	private:
		std::atomic<AtomicQueueItem*>	m_head;
		std::atomic<AtomicQueueItem*>	m_tail;
	};
}

#endif