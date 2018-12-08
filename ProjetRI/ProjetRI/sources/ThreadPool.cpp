#include "ThreadPool.h"
#include <iostream>
#include <iterator>
#include <utility>

ThreadPool::ThreadPool(unsigned int size) :
	Stop(false)
{
	Resize(size);
}

ThreadPool::~ThreadPool()
{
	std::unique_lock<std::mutex> lockPool(MutexPool);
	std::unique_lock<std::mutex> lockQueue(MutexQueue);
	Stop=true;
	ThreadHole.notify_all();
	lockQueue.unlock();

	Pool.clear();
	lockPool.unlock();
}

void ThreadPool::Resize(unsigned int size, bool async)
{
	if (async) std::thread{&ThreadPool::_Resize,this,size}.detach();
	else _Resize(size);
}

void ThreadPool::ThreadFunction(std::list<SThread>::iterator it)
{
	std::unique_lock<std::mutex> lockQueue(MutexQueue);
	while (!Stop)
	{
		while (!Stop && Queue.empty() && (ThreadToRemove==0))
			ThreadHole.wait(lockQueue);

		if (Stop) break;
		if (ThreadToRemove>0)
		{
			ThreadToRemove--;
			DeadPool.splice(std::begin(DeadPool), Pool, it);
			break;
		}

		SFunction f=std::move(Queue.front());
		Queue.pop();
		lockQueue.unlock();

		f();

		lockQueue.lock();
	}
	lockQueue.unlock();
}

void ThreadPool::_Resize(unsigned int size)
{
	if (size==0) size++;

	std::unique_lock<std::mutex> lockPool(MutexPool);
	if (Pool.size()>size)
	{
		std::unique_lock<std::mutex> lockQueue(MutexQueue);
		ThreadToRemove=Pool.size()-size;

		ThreadHole.notify_all();

		while (ThreadToRemove>0)
		{
			DeadHole.wait(lockQueue);
			DeadPool.clear();
		}
		lockQueue.unlock();
	}
	else
	{
		while (Pool.size()<size)
		{
			Pool.push_back(SThread());
			auto it=std::end(Pool);
			it--;
			it->SetThread(new std::thread(&ThreadPool::ThreadFunction, this, it));
		}
	}
	lockPool.unlock();
}
