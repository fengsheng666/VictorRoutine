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

#include <VRoutine/inner/AtomicQueueBase.h>

using namespace VictorRoutine;

static AtomicQueueBase::AtomicQueueItem* INVALID_PTR = (AtomicQueueBase::AtomicQueueItem*)(-1);

AtomicQueueBase::AtomicQueueBase(int maxSize) : m_maxSize(maxSize)
{
	m_head.store(0);
	m_tail.store(0);

	m_size.store(0);
}

AtomicQueueBase::~AtomicQueueBase()
{
	//VROUTINE_CHECKER(m_head.load() == NULL);
}

bool AtomicQueueBase::append(AtomicQueueItem* begin, AtomicQueueItem* end, int length)
{
	if (m_maxSize > 0)
	{
		int expected = m_size.load();
		for (; true; expected = m_size.load())
		{
			if (expected + length > m_maxSize)
			{
				return false;
			}
			if (m_size.compare_exchange_weak(expected, expected + length))
			{
				break;
			}

		}
	}
	else
	{
		m_size.fetch_add(length);
	}
	VROUTINE_CHECKER(end->getNext() == NULL);
	//<<<insert：在此插入“关闭”中断机制
	AtomicQueueItem* old = m_tail.exchange(end);
	if (0 == old)
	{
		if (begin == NULL)
		{
			abort();
		}
		AtomicQueueItem* oldHead = m_head.exchange(begin);
		//<<<insert：在此插入“恢复”中断机制
		VROUTINE_CHECKER(oldHead == NULL || oldHead == INVALID_PTR);
	}
	else
	{
		VROUTINE_CHECKER(old->getNext() == NULL);
		old->setNext(begin);
		//<<<insert：在此插入“恢复”中断机制
	}
	return true;
}

AtomicQueueBase::AtomicQueueItem* AtomicQueueBase::pop()
{
	AtomicQueueItem* invalid_expected = INVALID_PTR;
	//<<<insert：在此插入“关闭”中断机制
	AtomicQueueItem* head = m_head.exchange(INVALID_PTR);
	for (; true; head = m_head.exchange(INVALID_PTR), invalid_expected = INVALID_PTR)
	{
		if (head == NULL)
		{
			m_head.compare_exchange_strong(invalid_expected, NULL);
			//<<<insert：在此插入“恢复”中断机制
			return NULL;
		}
		if (head != INVALID_PTR)
		{
			AtomicQueueItem* next = head->_next.load();
			//此时m_head == INVALID_PTR || m_head == NULL
			m_head.store(next);
			//<<<insert：在此插入“恢复”中断机制
			m_size.fetch_sub(1);
			if (next != NULL)
			{
				head->setNext(NULL);
				return head;
			}
			AtomicQueueItem* expected = head;
			if (!m_tail.compare_exchange_strong(expected, NULL))
			{
				VROUTINE_CHECKER(expected != NULL);
				//<<<insert：在此插入“关闭”中断机制
				//等待append将begin接入
				while (!head->getNext()){};
				AtomicQueueItem* ptr = m_head.exchange(head->getNext());
				//<<<insert：在此插入“恢复”中断机制
				VROUTINE_CHECKER(ptr == NULL || ptr == INVALID_PTR);
				head->setNext(NULL);
			}
			VROUTINE_CHECKER(head->getNext() == NULL);
			return head;
		}
	}
	return INVALID_PTR;
}

AtomicQueueBase::AtomicQueueItem* AtomicQueueBase::pop(std::function<bool(void*)> filterFunc)
{
	AtomicQueueItem* invalid_expected = INVALID_PTR;
	//<<<insert：在此插入“关闭”中断机制
	AtomicQueueItem* head = m_head.exchange(INVALID_PTR);
	for (; true; head = m_head.exchange(INVALID_PTR), invalid_expected = INVALID_PTR)
	{
		if (head == NULL)
		{
			m_head.compare_exchange_strong(invalid_expected, NULL);
			//<<<insert：在此插入“恢复”中断机制
			return NULL;
		}
		if (head != INVALID_PTR)
		{
			if (!filterFunc(head->_ptr))
			{
				m_head.store(head);
				//<<<insert：在此插入“恢复”中断机制
				return NULL;
			}
			AtomicQueueItem* next = head->_next.load();
			//此时m_head == INVALID_PTR || m_head == NULL
			m_head.store(next);
			//<<<insert：在此插入“恢复”中断机制
			m_size.fetch_sub(1);
			if (next != NULL)
			{
				head->setNext(NULL);
				return head;
			}
			AtomicQueueItem* expected = head;
			if (!m_tail.compare_exchange_strong(expected, NULL))
			{
				VROUTINE_CHECKER(expected != NULL);
				//<<<insert：在此插入“关闭”中断机制
				//等待append将begin接入
				while (!head->getNext()){ };
				AtomicQueueItem* ptr = m_head.exchange(head->getNext());
				//<<<insert：在此插入“恢复”中断机制
				VROUTINE_CHECKER(ptr == NULL || ptr == INVALID_PTR);
				head->setNext(NULL);
			}
			VROUTINE_CHECKER(head->getNext() == NULL);
			return head;
		}
	}
	return INVALID_PTR;
}

void AtomicQueueBase::front(std::function<void(const void*)> recver)
{
	AtomicQueueItem* invalid_expected = INVALID_PTR;
	//<<<insert：在此插入“关闭”中断机制
	AtomicQueueItem* head = m_head.exchange(INVALID_PTR);
	for (; true; head = m_head.exchange(INVALID_PTR), invalid_expected = INVALID_PTR)
	{
		if (head == NULL)
		{
			m_head.compare_exchange_strong(invalid_expected, NULL);
			//<<<insert：在此插入“恢复”中断机制
			recver(NULL);
			return;
		}
		if (head != INVALID_PTR)
		{
			recver(head->_ptr);
			//此时m_head == INVALID_PTR || m_head == NULL
			m_head.store(head);
			return;
		}
	}
	recver(INVALID_PTR);
}