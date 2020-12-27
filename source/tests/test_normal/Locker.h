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

#ifndef LOCKER_H_FSDKOS
#define LOCKER_H_FSDKOS

#include <errno.h>
#if defined(_WIN32)
#	include <Windows.h>
#endif

class Locker
{
public:
	Locker();
	virtual ~Locker();

	bool lock() const;
	bool unLock() const;

private:
#if defined(_WIN32)
	HANDLE m_hMutex;
#endif
};

#endif