﻿// Copyright (c) 2020 FengSheng(EN. Victor Fung)
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

#include <ctime>
#include <chrono>
#include <limits>
#include <vector>
#include <atomic>
#include <thread>
#include <VRoutine/MultiThreadShared.h>
#include <VRoutine/AtomicQueue.h>
#include <VRoutine/Dispatcher.h>
#include <VRoutine/Routine.h>
#include "./Locker.h"

struct SharedItem
{
	SharedItem(int value) : _value(value){ }
	int	_value;
};

class SharedData : public VictorRoutine::MultiThreadShared
{
public:
	SharedData()
	{
		m_data.resize(2, 0);
	}
	void modifyOnLocker()
	{
		m_locker.tryLock();
		
		SharedItem* itemOne = m_data[0];
		srand(itemOne ? itemOne->_value : 0);
		m_data[0] = new SharedItem(rand());

		SharedItem* itemTwo = m_data[1];
		srand(itemTwo ? itemTwo->_value : 1);
		m_data[1] = new SharedItem(rand());

		delete itemOne;
		delete itemTwo;
		
		m_locker.unLock();
	}
	void modifyOnRoutine()
	{
		SharedItem* itemOne = m_data[0];
		srand(itemOne ? itemOne->_value : 0);
		m_data[0] = new SharedItem(rand());

		SharedItem* itemTwo = m_data[1];
		srand(itemTwo ? itemTwo->_value : 1);
		m_data[1] = new SharedItem(rand());

		delete itemOne;
		delete itemTwo;
	}

private:
	std::vector<SharedItem*>	m_data;
	Locker						m_locker;
};

class TestDispatcher : public VictorRoutine::Dispatcher, public VictorRoutine::RefObject
{
public:
	struct FuncWrapper
	{
		FuncWrapper() { }
		FuncWrapper(std::function<void()> func) : _func(func){ }
		std::function<void()> _func;
	};
	TestDispatcher() : m_funcQueue(0) {}
	virtual bool post(std::function<void()> func)
	{
		FuncWrapper* fw = new FuncWrapper(func);
		m_funcQueue.append(fw);
		return true;
	}
	std::function<void()> pop(bool &ok)
	{
		FuncWrapper* fw = m_funcQueue.pop([](FuncWrapper* item){
			return true;
		});
		std::function<void()> func;
		if (fw)
		{
			ok = true;
			func = fw->_func;
			delete fw;
		}
		else
		{
			ok = false;
		}
		return func;
	}
private:
	VictorRoutine::AtomicQueue<FuncWrapper> m_funcQueue;
};

void ThreadFunc(VictorRoutine::StrongPtr<TestDispatcher> dispatcher)
{
	bool ok = false;
	std::function<void()> func = dispatcher->pop(ok);
	for (; ok; func = dispatcher->pop(ok))
	{
		func();
	}
}

long long caculateTimes(int threadNum, int dataNum, int forCount, bool runOnRoutine)
{
	std::atomic<int> ticks;
	long long	timeEnd = 0;

	TestDispatcher* dptr = new TestDispatcher;
	VictorRoutine::StrongPtr<TestDispatcher> dispatcher = dptr;

	ticks.store(0);

	std::vector<VictorRoutine::StrongPtr<SharedData>> sharedDatas;
	sharedDatas.resize(dataNum);
	int maxTicks = dataNum * forCount;
	for (int k = 0; k < dataNum; k++)
	{
		SharedData* sd = new SharedData;
		sharedDatas[k] = sd;
		for (int i = 0; i < forCount; i++)
		{
			dispatcher->post([sd, maxTicks, runOnRoutine, dptr, &ticks, &timeEnd](){
				if (runOnRoutine)
				{
					bool suc = VictorRoutine::Routine([sd](){
						sd->modifyOnRoutine();
					}).addDependence(sd, true).go(dptr);
					if (!suc)
					{
						printf("Routine go fault!\n");
					}
				}
				else
				{
					sd->modifyOnLocker();
				}
				int old = ticks.fetch_add(1);
				if (old + 1 == maxTicks)
				{
					//结束计时
					timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now().time_since_epoch()).count();
					ticks.fetch_add(1);
				}
			});
		}
	}

	//开始计时
	long long timeBegin = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	for (int i = 0; i < threadNum; i++)
	{
		std::thread background_thread(ThreadFunc, dispatcher);
		background_thread.detach();
	}
	while (ticks <= maxTicks){}
	return (long long)timeEnd - (long long)timeBegin;
}

int main(int argc, char* argv[])
{
	//threadNum个线程执行：	竞争dataNum个数据，每个数据执行forCount次
	//						共dataNum * forCount个请求
	int threadNum = 8;
	int dataNum = 16;
	int forCount = 1024;

	//统计statisticTimes次 求平均时间消耗
	long long statisticTimes = 1024;

	long long timeBegin = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	long long mutexTotal = 0;
	for (long long i = 0; i < statisticTimes; i++)
	{
		mutexTotal += caculateTimes(threadNum, dataNum, forCount, false);
	}
	printf("work on mutex: statistic %lld times, total   times = %lld\n", 
		statisticTimes, mutexTotal);
	printf("work on mutex: statistic %lld times, average times = %lld\n", 
		statisticTimes, mutexTotal / statisticTimes);
	long long timeMid = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	printf("%lld\n", timeMid - timeBegin);

	long long routineTotal = 0;
	for (long long i = 0; i < statisticTimes; i++)
	{
		routineTotal += caculateTimes(threadNum, dataNum, forCount, true);
	}
	printf("work on routine: statistic %lld times,  total   times = %lld\n", 
		statisticTimes, routineTotal);
	printf("work on routine: statistic %lld times,  average times = %lld\n", 
		statisticTimes, routineTotal / statisticTimes);
	long long timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	printf("%lld\n", timeEnd - timeMid);

	while (true) {}

	return 0;
}