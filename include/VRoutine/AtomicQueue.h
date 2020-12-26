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
		bool append(T* item)
		{
			AtomicQueueItem* begin = new AtomicQueueItem(NULL, item);
			if (begin == NULL)
			{
				return false;
			}
			AtomicQueueItem* end = begin;
			AtomicQueueBase::append(begin, end, 1);
			return true;
		}
		T* pop()
		{
			AtomicQueueItem* item = AtomicQueueBase::pop();
			if (item == 0)
			{
				return NULL;
			}
			T* ptr = (T*)item->_ptr;
			delete item;
			return ptr;
		}
		T* pop(std::function<bool(T*)> filterFunc)
		{
			AtomicQueueItem* item = AtomicQueueBase::pop([filterFunc](void* _ptr){
				return filterFunc((T*)_ptr);
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
			AtomicQueueBase::front([recver](const void* _ptr){
				return recver((T*)_ptr);
			});
		}
	};
}

#endif