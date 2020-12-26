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

#include <VRoutine/RefObject.h>
#include "./RefWeak.h"

using namespace VictorRoutine;

RefObject::RefObject()
{
	m_refWeak.store(0);
	m_refCount.store(0);
}

RefObject::~RefObject()
{
	RefWeak* refWeak = m_refWeak.load();
	if (0 == refWeak)
	{
		return;
	}
	RefObject* expected = this;
	//if refWeak->m_refStrong==this set refWeak->m_refStrong=NULL
	while (!refWeak->m_refStrong.compare_exchange_weak(expected, 0))
	{
		VROUTINE_CHECKER(expected != 0);
		expected = this;
	}
	VROUTINE_CHECKER(expected == this);
	int newRefCount = refWeak->unRef();
	if(newRefCount == 0) 
	{
		delete refWeak;
	}
}

int RefObject::addRef()
{
	return m_refCount.fetch_add(1) + 1;
}

int RefObject::unRef()
{
	return m_refCount.fetch_sub(1) - 1;
}

RefWeak* RefObject::getRefWeak()
{
	RefWeak* oldWeak = m_refWeak.load();
	if (0 != oldWeak) 
	{
		return oldWeak;
	}
	RefWeak* newWeak = new RefWeak(this);
	int rw_init_ref_count = newWeak->addRef();
	VROUTINE_CHECKER(rw_init_ref_count == 1);
	RefWeak* expected = 0;
	//if m_refWeak==NULL set m_refWeak=newWeak
	if (m_refWeak.compare_exchange_strong(expected, newWeak))
	{
		VROUTINE_CHECKER(expected == 0);
		return newWeak;
	}
	else
	{
		delete newWeak;
		return expected;
	}
	
}