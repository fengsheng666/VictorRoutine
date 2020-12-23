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

bool Locker::tryLock() const
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