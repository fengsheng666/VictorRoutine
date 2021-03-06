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

#include "./Task.h"
#include <thread>
#include <atomic>
#include <VRoutine/inner/AtomicQueueBase.h>
#include <VRoutine/Dispatcher.h>
#include <VRoutine/Routine.h>

using namespace VictorRoutine;

Routine::Routine(std::function<void()> func) : m_task(new Task(func))
{

}

Routine::~Routine()
{
	if (m_task)
	{
		delete m_task;
	}
}

void* Routine::operator new(size_t size)
{
	return malloc(size);
}

void Routine::operator delete(void* ptr)
{
	if (ptr)
	{
		free(ptr);
	}
}

Routine& Routine::addDependence(MultiThreadShared* obj, bool bExclusive)
{
	m_deps[obj] = bExclusive;
	return *this;
}

bool Routine::go(Dispatcher* dispatcher, int maxDepth)
{
	if (m_task == NULL)
	{
		return false;
	}
	if (maxDepth <= 0)
	{
		m_task->setMaxDepth(m_deps.size());
	}
	if (m_task->schedules().size() != m_deps.size())
	{
		m_task->schedules().resize(m_deps.size());
		std::map<MultiThreadShared*, bool>::const_iterator iter = m_deps.begin();
		for (int i = 0; i < m_task->schedules().size(); i++, iter++)
		{
			(m_task->schedules())[i].m_object = iter->first;
			(m_task->schedules())[i].m_bExclusive = iter->second;
		}
	}
	if (m_task->execute(NULL, dispatcher, 0))
	{
		m_task = NULL;
		return true;
	}
	else
	{
		return false;
	}
}