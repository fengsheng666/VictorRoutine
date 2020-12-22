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

#include <VRoutine/inner/StrongPtrBase.h>
#include <VRoutine/RefObject.h>

using namespace VictorRoutine;

StrongPtrBase::StrongPtrBase(RefObject* ptr) : m_ptr(ptr)
{
	if (m_ptr)
	{
		m_ptr->addRef();
	}
}

StrongPtrBase::StrongPtrBase(const StrongPtrBase& other) : m_ptr(other.m_ptr)
{
	if (m_ptr)
	{
		m_ptr->addRef();
	}
}

StrongPtrBase::~StrongPtrBase()
{
	if (m_ptr && 0 == m_ptr->unRef())
	{
		delete m_ptr;
	}
}

void StrongPtrBase::setPtr(RefObject* ptr)
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

RefObject* StrongPtrBase::getPtr() const
{
	return m_ptr;
}

RefObject* StrongPtrBase::release()
{
	if (m_ptr)
	{
		m_ptr->unRef();
	}
	RefObject* ptr = m_ptr;
	m_ptr = 0;
	return ptr;
}