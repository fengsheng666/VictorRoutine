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

#include <thread>
#include <vector>

//引入所需的文件
#include <VRoutine/MultiThreadShared.h>
#include <VRoutine/Routine.h>
#include <VRoutine/StrongPtr.h>

//包装需要临界访问的数据
class TestData : public VictorRoutine::MultiThreadShared
{
public:
	TestData()
	{
		m_testDatas.resize(10, 0);
	}
	~TestData(){}
	inline std::vector<int>& getTestDatas()
	{
		return m_testDatas;
	}
private:
	std::vector<int>	m_testDatas;
};

void RoutineCallThread(int tid, int forCount, TestData* td)
{
	for (int fci = 0; fci < forCount; fci++)
	{
		//构建routine，设置依赖，然后执行。
		bool suc = VictorRoutine::Routine([tid, fci, td](){
			std::vector<int>& datas = td->getTestDatas();
			if (tid % 2 == 0)
			{
				srand(tid * 100);
				int new_size = rand() % 10 + 1;
				datas.resize(new_size);
				for (int i = 0; i < (int)datas.size(); i++)
				{
					srand(tid * 10 + i);
					datas[i] = rand();
				}
				printf("###tid = %d, fci = %d, modify###\n\n", tid, fci);
			}
			else
			{
				printf("+++tid = %d, fci = %d, begin read+++\n", tid, fci);
				std::string mid = "";
				for (int i = 0; i < (int)datas.size(); i++)
				{
					if (0 != i)
					{
						mid += ", ";
					}
					else
					{
						printf("%d", datas[i]);
					}
					char buf[100] = { '\0' };
					itoa(datas[i], buf, 10);
					mid += buf;
				}
				printf("%s\n", mid.c_str());
				printf("---tid = %d, fci = %d, end read---\n\n", tid, fci);
			}
		}).addDependence(td, tid % 2 == 0).go(NULL, 128);
		if (!suc)
		{
			printf("\nRoutine go fault!\n");
		}
		if (tid % 2 == 0)
		{
			srand(time(NULL) + tid);
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
		}
	}
}

int main(int argc, char* argv[])
{
	int threadNum = 8;
	int forCount = 2;
	VictorRoutine::StrongPtr<TestData> td = new TestData;
	for (int i = 0; i < threadNum; i++)
	{
		std::thread background_thread(RoutineCallThread, i, forCount, td.get());
		background_thread.detach();
	}
	while (true) { }

	return 0;
}