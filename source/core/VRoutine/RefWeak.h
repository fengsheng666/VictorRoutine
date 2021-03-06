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

#ifndef REFWEAK_H_VROUTINE
#define REFWEAK_H_VROUTINE

#include <atomic>

namespace VictorRoutine
{
	class RefObject;

	class RefWeak
	{
	public:
		virtual ~RefWeak();
	protected:
		RefWeak(RefObject* refObject)
			: m_refStrong(refObject), m_refCount(0) {}
	private:
		int addRef();
		int unRef();
		//需要对返回的RefObject执行unRef
		RefObject* getRefObject();
	private:
		std::atomic<RefObject*>	m_refStrong;
		std::atomic<int>		m_refCount;

		friend class RefObject;
		friend class WeakPtrBase;
	};
}

#endif