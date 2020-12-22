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

#ifndef STRONGPTRBASE_H_VROUTINE
#define STRONGPTRBASE_H_VROUTINE

#include "../VRoutineDef.h"

namespace VictorRoutine
{
	class RefObject;

	class VROUTINE_API StrongPtrBase
	{
	protected:
		StrongPtrBase() : m_ptr(0){}
		StrongPtrBase(RefObject* ptr);
		StrongPtrBase(const StrongPtrBase& other);
		~StrongPtrBase();
		void setPtr(RefObject* ptr);
		RefObject* getPtr() const;
		RefObject* release();
	private:
		RefObject*	m_ptr;
	};
}

#endif