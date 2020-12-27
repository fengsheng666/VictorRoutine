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

#ifndef ROUTINE_H_VROUTINE
#define ROUTINE_H_VROUTINE

#include <VRoutine/StrongPtr.h>
#include <VRoutine/MultiThreadShared.h>
#include <map>
#include <functional>

namespace VictorRoutine
{
	class Dispatcher;
	class MultiThreadShared;
	class Task;

	class VROUTINE_API Routine
	{
	public:
		Routine(std::function<void()> func);
		~Routine();
		Routine& addDependence(MultiThreadShared* obj, bool bExclusive = true);
		bool go(Dispatcher* dispatcher, int maxDepth = 0);

	private:
		std::map<MultiThreadShared*, bool>	m_deps;
		Task*								m_task;
	};
}

#endif