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

#ifndef MULTITHREADSHARED_H_VROUTINE
#define MULTITHREADSHARED_H_VROUTINE

#include "./VRoutineDef.h"
#include "./RefObject.h"
#include <atomic>

namespace VictorRoutine
{
	class Dispatcher;
	class Task;
	class TaskQueue;

	class VROUTINE_API MultiThreadShared : public RefObject
	{
	public:
		virtual ~MultiThreadShared();
	protected:
		MultiThreadShared();
	private:
		//先将task插入无锁队列，再置位
		//返回 false 就绪成功但执行延期，true 就绪成功且可立即执行
		void preempt(Task* task, Dispatcher* dispatcher, int depth);
		void release(Dispatcher* dispatcher, int depth);
	private:
		void schedule(Dispatcher* dispatcher, int depth);
	private:
		TaskQueue*			m_taskQueue;
		std::atomic<bool>	m_preemptFlag;
		std::atomic<int>	m_sharedCount;

		friend class Task;

	};
}

#endif