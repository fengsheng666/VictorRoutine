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

#ifndef WEAKPTRBASE_H_VROUTINE
#define WEAKPTRBASE_H_VROUTINE

#include "../VRoutineDef.h"

namespace VictorRoutine
{
	class RefObject;
	class RefWeak;

	class VROUTINE_API WeakPtrBase
	{
	protected:
		WeakPtrBase() : m_ptr(0){}
		WeakPtrBase(RefObject* ptr);
		WeakPtrBase(const WeakPtrBase& other);
		~WeakPtrBase();
		void setPtr(RefWeak* ptr);
		RefWeak* getPtr() const;
		RefObject* toStrong();
	private:
		RefWeak*	m_ptr;
	};
}

#endif