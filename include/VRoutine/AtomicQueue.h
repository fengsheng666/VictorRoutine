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

#ifndef ATOMICQUEUE_H_VROUTINE
#define ATOMICQUEUE_H_VROUTINE

#include "./VRoutineDef.h"
#include "./inner/AtomicQueueBase.h"
#include <list>

namespace VictorRoutine
{
	template<class T>
	class AtomicQueue : public AtomicQueueBase
	{
	public:
		AtomicQueue(int maxSize) : AtomicQueueBase(maxSize) {}
		~AtomicQueue(){ }
		void append(T* item)
		{
			AtomicQueueItem* begin = new AtomicQueueItem(NULL, item);
			AtomicQueueItem* end = begin;
			assert(begin != NULL);
			AtomicQueueBase::append(begin, end, 1);
		}
		T* pop(std::function<bool(T*)> filterFunc)
		{
			AtomicQueueItem* item = AtomicQueueBase::pop([filterFunc](AtomicQueueItem* item){
				return filterFunc((T*)item->_ptr);
			}); 
			if (item == 0)
			{
				return NULL;
			}
			T* ptr = (T*)item->_ptr;
			delete item;
			return ptr;
		}
		void front(std::function<void(const T*)> recver)
		{
			AtomicQueueBase::front([recver](const AtomicQueueItem* item){
				return recver(item ? (T*)(item->_ptr) : NULL);
			});
		}
	};
}

#endif