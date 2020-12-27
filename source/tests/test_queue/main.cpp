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

#include <VRoutine/AtomicQueue.h>
#include <ctime>
#include <chrono>
#include <limits>
#include <vector>
#include <atomic>
#include <thread>

class AtomicQueueTest
{
public:
	AtomicQueueTest() : m_testQueue(0) {}
	~AtomicQueueTest()
	{

	}
	virtual void post(int v)
	{
		int* pv = new int(v);
		while (!m_testQueue.append(pv)){ }
	}
	int pop()
	{
		int* pv = m_testQueue.pop();
		if (pv)
		{
			int v = *pv;
			*pv += 111;
			delete pv;
			return v;
		}
		else
		{
			return -1;
		}
	}
private:
	VictorRoutine::AtomicQueue<int> m_testQueue;
};

AtomicQueueTest gAQT;

volatile int g_TestFlag = 0;

static std::atomic<int> g_TestCount(0);

void TestThread(bool bConsumer, unsigned char* pc)
{
	int k = 0;
	while (true)
	{
		if (bConsumer)
		{
			int v = gAQT.pop();
			if (v == 555)
			{
				int t = g_TestCount.fetch_sub(1);
				//printf("t = %d\n", t);
			}
		}
		else
		{
			if (k < 1024 * 1024)
			{
				gAQT.post(555);
				g_TestCount.fetch_add(1);
				k++;
			}
			else
			{
				*pc = 1;
				return;
			}
		}
	}
}

int malloc_test()
{
	for (int k = 0; k < 2; k++)
	{
		char* ptr1 = (char*)malloc(1023);
		const int arr_size = 1024 * 1024;
		//char* mids[arr_size] = { NULL };
		std::vector<char*> mids;
		mids.resize(arr_size, 0);
		int total = 0;
		for (int i = 0; i < arr_size; i++)
		{
			total += (i % 64 + 1) * 1024;
			mids[i] = (char*)malloc((i % 64 + 1) * 1024);
		}
		printf("total == %d k\n", total / 1024);
		char* buf = (char*)malloc(1025);
		for (int i = 0; i < arr_size; i++)
		{
			if (mids[i])
			{
				delete mids[i];
			}
		}
	}
	printf("begin circle\n");
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	return 1;
}

int main(int argc, char* argv[])
{ 
	unsigned char* pc = (unsigned char*)(&g_TestFlag);
	for (int i = 0; i < 1; i++)
	{
		std::thread background_thread(TestThread, true, pc);
		background_thread.detach();
	}
	for (int i = 0; i < 4; i++)
	{
		std::thread background_thread(TestThread, false, pc + i);
		background_thread.detach();
	}
	while (true)
	{
		int testCount = g_TestCount.load();
		printf("testCount = %d, g_TestFlag = %d\n", testCount, g_TestFlag);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return 1;
}
