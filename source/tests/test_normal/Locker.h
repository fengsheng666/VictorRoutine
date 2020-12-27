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

	bool tryLock() const;
	bool unLock() const;

private:
#if defined(_WIN32)
	HANDLE m_hMutex;
#endif
};

#endif