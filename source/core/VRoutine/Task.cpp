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

#include "./Task.h"

using namespace VictorRoutine;

void Task::consume(MultiThreadShared* obj, Dispatcher* dispatcher)
{
	if (obj != NULL)
	{
		assert(m_blockPos >= 0 && m_blockPos < m_schedules.size());
		assert(m_schedules[m_blockPos].m_object == obj);
	}
	for (m_blockPos++; m_blockPos < m_schedules.size(); m_blockPos++)
	{
		bool preempted = 
			m_schedules[m_blockPos].m_object->preempt(this);
		if (!preempted)
		{
			return;
		}
	}

	m_function();

	for (int pos = m_schedules.size() - 1; pos >= 0; pos--)
	{
		m_schedules[m_blockPos].m_object->release(dispatcher);
	}

	delete this;
}


/*
#include <VRoutine/StrongPtr.h>
#include <VRoutine/WeakPtr.h>
#include <VRoutine/RefObject.h>

class RefTest : public VictorRoutine::RefObject
{
public:
	static void Test()
	{
		VictorRoutine::StrongPtr<RefTest> sptr = new RefTest;
		VictorRoutine::WeakPtr<RefTest> wptr = sptr.get();
	}
	RefTest(){ }
}; 
*/