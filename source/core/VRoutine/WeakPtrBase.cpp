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

#include <VRoutine/inner/WeakPtrBase.h>
#include <VRoutine/RefObject.h>
#include "./RefWeak.h"

using namespace VictorRoutine;

WeakPtrBase::WeakPtrBase(RefObject* ptr) : m_ptr(ptr ? ptr->getRefWeak() : 0)
{
	if (m_ptr)
	{
		m_ptr->addRef();
	}
}

WeakPtrBase::WeakPtrBase(const WeakPtrBase& other) : m_ptr(other.m_ptr)
{
	if (m_ptr)
	{
		m_ptr->addRef();
	}
}

WeakPtrBase::~WeakPtrBase()
{
	if (m_ptr && 0 == m_ptr->unRef())
	{
		delete m_ptr;
	}
}

void WeakPtrBase::setPtr(RefWeak* ptr)
{
	if (ptr == m_ptr)
	{
		return;
	}
	if (m_ptr && 0 == m_ptr->unRef())
	{
		delete m_ptr;
	}
	m_ptr = ptr;
	if (m_ptr)
	{
		m_ptr->addRef();
	}
}

RefWeak* WeakPtrBase::getPtr() const
{
	return m_ptr;
}

RefObject* WeakPtrBase::toStrong()
{
	return m_ptr ? m_ptr->getRefObject() : 0;
}
