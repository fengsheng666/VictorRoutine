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

#include "./RefWeak.h"
#include <VRoutine/RefObject.h>

using namespace VictorRoutine;

static RefObject* INVALID_PTR = (RefObject*)(-1);

RefWeak::~RefWeak()
{
	VROUTINE_CHECKER(m_refStrong.load() == 0);
}

int RefWeak::addRef()
{
	return m_refCount.fetch_add(1) + 1;
}

int RefWeak::unRef()
{
	return m_refCount.fetch_sub(1) - 1;
}

RefObject* RefWeak::getRefObject()
{
	RefObject* oldStrong = m_refStrong.load();
	for (; true; oldStrong = m_refStrong.load())
	{
		if (oldStrong == 0) 
		{
			return 0;
		}
		if (oldStrong == INVALID_PTR)
		{
			continue;
		}
		RefObject* expected = oldStrong;
		//<<<insert：在此插入“关闭”中断机制
		//if m_refStrong==oldStrong set m_refStrong=INVALID_PTR
		if (m_refStrong.compare_exchange_weak(expected, INVALID_PTR))
		{
			VROUTINE_CHECKER(expected == oldStrong);
			break;
		}
		//<<<insert：在此插入“恢复”中断机制
	}

	// if oldStrong->m_refCount != 0 set oldStrong->m_refCount++
	int strongRefCount = oldStrong->m_refCount.load();
	for (; true; strongRefCount = oldStrong->m_refCount.load())
	{
		if (strongRefCount == 0)
		{
			m_refStrong.store(oldStrong);
			//<<<insert：在此插入“恢复”中断机制
			return 0;
		}
		int expected = strongRefCount;
		//if oldStrong->m_refCount==strongRefCount set oldStrong->m_refCount++ 即不为零则++
		if (oldStrong->m_refCount.compare_exchange_weak(expected, strongRefCount + 1))
		{
			VROUTINE_CHECKER(expected == strongRefCount);
			break;
		}
	}

	m_refStrong.store(oldStrong);
	//<<<insert：在此插入“恢复”中断机制
	return oldStrong;
}