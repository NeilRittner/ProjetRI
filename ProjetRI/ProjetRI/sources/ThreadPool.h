#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <queue>
#include <functional>

class ThreadPool
{
private:
	struct SThread
	{
	public:
		SThread(std::thread *_t=nullptr) : t(_t) {}
		SThread(SThread &&st) : t(st.t) { st.t=nullptr; }
		~SThread() { if (t) { t->join(); delete(t); } }
		inline void SetThread(std::thread *_t) { t=_t; }

	private:
		SThread(SThread &st) {}

	private:
		std::thread *t;
	};

	struct StorageFunction
	{
		virtual void Execute()=0;
	};

	struct SFunction
	{
	public:
		SFunction(StorageFunction *_f) : func(_f) {}
		SFunction(SFunction &&sf) : func(sf.func) { sf.func=nullptr; }
		~SFunction() { delete(func); }
		inline void operator()() { func->Execute(); }

	private:
		SFunction(SFunction &sf) {}

	private:
		StorageFunction *func;
	};

	struct StorageFunctionImpl : public StorageFunction
	{
		using func_type=std::function<void()>;

		template <typename TFunction, typename... TArgs>
		StorageFunctionImpl(TFunction&& a_func, TArgs&&... a_args) : func(std::forward<func_type>(std::bind(std::forward<TFunction>(a_func), std::forward<TArgs>(a_args)...))) {}
		inline void operator()() { Execute(); }
		inline void Execute() { func(); }

	private:
		func_type func;
	};

public:
	ThreadPool(unsigned int size);
	~ThreadPool();
	template <typename TFunction, typename... TArgs> void Execute(TFunction&& a_func, TArgs&&... a_args);
	void Resize(unsigned int size, bool async=false);

private:
	void ThreadFunction(std::list<SThread>::iterator it);
	void _Resize(unsigned int size);

private:
	std::list<SThread> Pool,DeadPool;
	unsigned int ThreadToRemove;
	std::queue<SFunction> Queue;
	bool Stop;
	std::mutex MutexQueue, MutexPool;
	std::condition_variable ThreadHole, DeadHole;
};

template <typename TFunction, typename... TArgs>
inline void ThreadPool::Execute(TFunction && a_func, TArgs && ...a_args)
{
	std::lock_guard<std::mutex> lock(MutexQueue);
	Queue.push(SFunction(new StorageFunctionImpl(std::forward<TFunction>(a_func), std::forward<TArgs>(a_args)...)));
	ThreadHole.notify_one();
}
