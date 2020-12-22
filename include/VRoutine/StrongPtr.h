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

#ifndef STRONGPTR_H_VROUTINE
#define STRONGPTR_H_VROUTINE

#include "./inner/StrongPtrBase.h"

namespace VictorRoutine
{
	// T 为 RefObject的子类，而非RefWeak的子类
	template<class T>
	class StrongPtr : public StrongPtrBase
	{
	public:
		StrongPtr() { }
		StrongPtr(T* ptr) 
			: StrongPtrBase(ptr) { }
		StrongPtr(const StrongPtr& other) 
			: StrongPtrBase(other) { }
		~StrongPtr(){ }
		StrongPtr& operator = (const StrongPtr& other)
		{
			StrongPtrBase::setPtr(other.get());
			return *this;
		}
		StrongPtr& operator = (T* ptr)
		{
			StrongPtrBase::setPtr(ptr);
			return *this;
		}
		//
		bool operator == (const StrongPtr& other) const
		{
			return (getPtr() == other.getPtr());
		}
		bool operator == (const T* ptr) const
		{
			return (getPtr() == ptr);
		}
		//
		bool operator != (const StrongPtr& other) const
		{
			return (getPtr() != other.getPtr());
		}
		bool operator != (const T* ptr) const
		{
			return (getPtr() != ptr);
		}
		//
		bool operator < (const StrongPtr& other) const
		{
			return (getPtr() < other.getPtr());
		}
		bool operator < (const T* ptr) const
		{
			return (getPtr() < ptr);
		}
		//
		T& operator*() const
		{
			T* impl =
				dynamic_cast<T*>(StrongPtrBase::getPtr());
			return *impl;
		}
		T* operator->() const
		{
			T* impl =
				dynamic_cast<T*>(StrongPtrBase::getPtr());
			return impl;
		}
		bool operator!() const
		{
			return 0 == StrongPtrBase::getPtr();
		}
		//
		bool valid() const
		{
			return 0 != StrongPtrBase::getPtr();
		}
		T* get() const
		{
			T* impl =
				dynamic_cast<T*>(StrongPtrBase::getPtr());
			return impl;
		}
		T* release()
		{
			RefObject* ptr = StrongPtrBase::release()
			T* impl = dynamic_cast<T*>(ptr);
			assert((ptr == NULL && impl == NULL) || (ptr != NULL && impl != NULL));
			return impl;
		}
	};

}

#endif