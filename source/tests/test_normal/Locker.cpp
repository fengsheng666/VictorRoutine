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

#include "./Locker.h"

Locker::Locker()
{
	m_hMutex = CreateMutex(
		NULL,
		FALSE,
		NULL);
}

Locker::~Locker()
{
	CloseHandle(m_hMutex);
}

bool Locker::lock() const
{
	if (m_hMutex == NULL)
	{
		return false;
	}
	DWORD dwMilliseconds = INFINITE;
	DWORD resul = WaitForSingleObject(
		m_hMutex,
		dwMilliseconds);
	if (resul == WAIT_OBJECT_0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Locker::unLock() const
{
	return ReleaseMutex(m_hMutex);
}