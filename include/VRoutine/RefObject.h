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

#ifndef REFOBJECT_H_VROUTINE
#define REFOBJECT_H_VROUTINE

#include "./VRoutineDef.h"

#include <atomic>

namespace VictorRoutine
{
	class RefWeak;

	class VROUTINE_API RefObject
	{
	public:
		virtual ~RefObject();
	protected:
		RefObject();
	private:
		int addRef();
		int unRef();
		RefWeak* getRefWeak();
	private:
		std::atomic<int>		m_refCount;
		std::atomic<RefWeak*>	m_refWeak;

		friend class RefWeak;
		friend class StrongPtrBase;
		friend class WeakPtrBase;
		friend class AtomicQueueImpl;
	};
}

#endif