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

#ifndef WEAKPTR_H_VROUTINE
#define WEAKPTR_H_VROUTINE

#include "./inner/WeakPtrBase.h"
#include "./StrongPtr.h"

namespace VictorRoutine
{
	// T 为 RefObject的子类，而非RefWeak的子类
	template<class T>
	class WeakPtr : public WeakPtrBase
	{
	public:
		WeakPtr() { }
		WeakPtr(T* ptr) : WeakPtrBase(ptr) { }
		WeakPtr(const WeakPtr& other)
			: WeakPtrBase(other) { }
		~WeakPtr() { }
		WeakPtr& operator = (const WeakPtr& other)
		{
			WeakPtrBase::setPtr(other.get());
			return *this;
		}
		WeakPtr& operator = (T* ptr)
		{
			WeakPtrBase::setPtr(ptr ? ptr->getRefWeak() : 0);
			return *this;
		}
		//
		StrongPtr<T> getStrong()
		{
			RefObject* ptr = WeakPtrBase::toStrong();
			StrongPtr<T> sptr(dynamic_cast<T*>(ptr));
			if (ptr && ptr->unRef() == 0)
			{
				static bool dynamic_cast_RefObject_to_template_T = false;
				VROUTINE_CHECKER(dynamic_cast_RefObject_to_template_T);
				delete ptr;
			}
			return sptr;
		}
	};
}

#endif