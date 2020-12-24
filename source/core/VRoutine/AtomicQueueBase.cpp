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
	assert(m_head.load() == NULL);
}

void AtomicQueueBase::append(AtomicQueueItem* begin, AtomicQueueItem* end, int length)
{
	while (m_maxSize > 0 && m_size.load() > m_maxSize) {}
	if (m_maxSize > 0)
	{
		int expected = m_size.load();
		do
		{
			if (expected + length > m_maxSize)
			{
				continue;
			}
			if (m_size.compare_exchange_weak(expected, expected + length))
			{
				break;
			}

		} while (expected = m_size.load());
	}
	else
	{
		m_size.fetch_add(length);
	}
	//<<<insert：在此插入“关闭”中断机制
	AtomicQueueItem* old = m_tail.exchange(end);
	if (0 == old)
	{
		//<<<insert：在此插入“恢复”中断机制
		AtomicQueueItem* head = m_head.load();
		//正在pop最后一个元素，等待pop完成
		while (head != NULL)
		{
			head = m_head.load();
		}
		m_head.store(begin);
	}
	else
	{
		assert(old->getNext() == NULL);
		old->setNext(begin);
		//<<<insert：在此插入“恢复”中断机制
	}
}

AtomicQueueBase::AtomicQueueItem* AtomicQueueBase::pop(std::function<bool(AtomicQueueItem*)> filterFunc)
{
	// if m_head != INVALID_PTR 且 m_head != NULL set m_head=INVALID_PTR
	AtomicQueueItem* head = m_head.load();
	for (; true; head = m_head.load())
	{
		if (head == NULL)
		{
			return NULL;
		}
		if (head == INVALID_PTR)
		{
			continue;
		}
		AtomicQueueItem* expected = head;
		//<<<insert：在此插入“关闭”中断机制
		if (m_head.compare_exchange_weak(expected, INVALID_PTR))
		{
			break;
		}
		//<<<insert：在此插入“恢复”中断机制
	}
	if (!filterFunc(head))
	{
		m_head.store(head);
		return NULL;
	}
	//m_tail == head 为最后一个元素，则 set m_tail=NULL
	AtomicQueueItem* expected = head;
	if (m_tail.compare_exchange_strong(expected, NULL))
	{
		m_head.store(NULL);
		//<<<insert：在此插入“恢复”中断机制
		m_size.fetch_sub(1);
		assert(head->getNext() == NULL);
		return head;
	}
	//等待append将begin接入
	while (!head->getNext()){};
	m_head.store(head->getNext());
	//<<<insert：在此插入“恢复”中断机制
	m_size.fetch_sub(1);
	head->setNext(NULL);
	return head;
}

void AtomicQueueBase::front(std::function<void(const AtomicQueueItem*)> recver)
{
	AtomicQueueItem* head = m_head.load();
	for (; true; head = m_head.load())
	{
		if (head == NULL)
		{
			return recver(NULL);
		}
		if (head == INVALID_PTR)
		{
			continue;
		}
		AtomicQueueItem* expected = head;
		//<<<insert：在此插入“关闭”中断机制
		if (m_head.compare_exchange_weak(expected, INVALID_PTR))
		{
			break;
		}
		//<<<insert：在此插入“恢复”中断机制
	}
	recver(head);
	m_head.store(head);
	//<<<insert：在此插入“恢复”中断机制
}