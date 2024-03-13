#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <thread>
#include <future>

const int TASK_MAX_THRESHHOLD = 2048;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME_SECONDS = 60; 


enum class PoolMode
{
	MODE_FIXED,  // 固定数量的线程
	MODE_CACHED, // 线程数量可动态
};

class Thread {
public:

	// 线程函数对象类型
	using ThreadFunc = std::function<void(int)>;

	explicit Thread(ThreadFunc func)
		: func_(std::move(func))
		, threadId_(generateId_++) { }

	~Thread() = default;

	void start() {
		std::thread{func_, threadId_}.detach();
	}

	int getId() const {
		return threadId_;
	}

private:
	ThreadFunc func_;
	static int generateId_;
	int threadId_;  // 保存线程id
};

int Thread::generateId_ = 0;

class ThreadPool {
public:

	ThreadPool()
		: initThreadSize_(0)
		, threadSizeThreshHold_(THREAD_MAX_THRESHHOLD)
		, curThreadSize_(0)
		, idleThreadSize_(0)
		, taskSize_(0)
		, taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
		, poolMode_(PoolMode::MODE_FIXED)
		, isPoolRunning_(false)
	{ }

	~ThreadPool()
	{
		isPoolRunning_ = false;

		std::unique_lock<std::mutex> lock(taskQueMtx_);
		notEmpty_.notify_all();
		exitCond_.wait(lock, [&]() { return threads_.size() == 0; });
	}

	void setMode(PoolMode mode)
	{
		if (!checkRunningState()) {
		    poolMode_ = mode;
        }
	}

	// 设置task任务队列上线阈值
	void setTaskQueMaxThreshHold(int threshhold)
	{
		if (!checkRunningState()) {
		    taskQueMaxThreshHold_ = threshhold;
        }
	}

	// 设置线程池cached模式下线程阈值
	void setThreadSizeThreshHold(int threshhold)
	{
		if (!checkRunningState()) {
            if (poolMode_ == PoolMode::MODE_CACHED)
            {
                threadSizeThreshHold_ = threshhold;
            }
        }
	}

	template<typename Func, typename... Args>
	auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
	{
		using RType = decltype(func(args...));
		auto task = std::make_shared<std::packaged_task<RType()>>(
			std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
		std::future<RType> result = task->get_future();

		std::unique_lock<std::mutex> lock(taskQueMtx_);

		if (!notFull_.wait_for(lock, std::chrono::seconds(1),
			[&](){ return taskQue_.size() < static_cast<size_t>(taskQueMaxThreshHold_); }))
		{
			std::cerr << "task queue is full, submit task fail.\n";

            return std::future<RType>();  // 这样做future get后会抛出异常(future 默认构造函数的原因)
		}

		taskQue_.emplace([task]() {(*task)();});
		taskSize_++;

		// 因为新放了任务，notEmpty_上进行通知
		notEmpty_.notify_all();

		// cached模式 任务处理比较紧急 场景：小而快的任务 需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
		if (poolMode_ == PoolMode::MODE_CACHED
			&& taskSize_ > idleThreadSize_
			&& curThreadSize_ < threadSizeThreshHold_)
		{
			std::cout << ">>> create new thread...\n";

			// 创建新的线程对象
			auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
			int threadId = ptr->getId();
			threads_.emplace(threadId, std::move(ptr));
			// 启动线程
			threads_[threadId]->start();
			// 修改线程个数相关的变量
			curThreadSize_++;
			idleThreadSize_++;
		}

		// 返回任务的Result对象
		return result;
	}

	void start(int initThreadSize = std::thread::hardware_concurrency()) // 根据核心数量来设置threadnum
	{
		isPoolRunning_ = true;
		initThreadSize_ = initThreadSize;
		curThreadSize_ = initThreadSize;

		// 创建线程对象
		for (int i = 0; i < initThreadSize_; i++)
		{
			auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
			int threadId = ptr->getId();
			threads_.emplace(threadId, std::move(ptr));
		}

		// 启动所有线程  std::vector<Thread*> threads_;
		for (int i = 0; i < initThreadSize_; i++)
		{
			threads_[i]->start(); // 需要去执行一个线程函数
			idleThreadSize_++;    // 记录初始空闲线程的数量
		}
	}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:

	// 线程函数
	void threadFunc(int threadid)
	{
		auto lastTime = std::chrono::high_resolution_clock().now();

		// 所有任务必须执行完成，线程池才可以回收所有线程资源
		for (;;)
		{
			Task task;
			{
				std::unique_lock<std::mutex> lock(taskQueMtx_);
				std::cout << "tid:" << std::this_thread::get_id() << "尝试获取任务...\n";

				// cached模式下，有可能已经创建了很多的线程，但是空闲时间超过60s，应该把多余的线程
				// 结束回收掉（超过initThreadSize_数量的线程要进行回收）
				// 当前时间 - 上一次线程执行的时间 > 60s

				while (taskQue_.size() == 0)
				{
					// 线程池要结束，回收线程资源
					if (!isPoolRunning_)
					{
						threads_.erase(threadid);
						std::cout << "threadid:" << std::this_thread::get_id() << " exit!\n";
						exitCond_.notify_all();
						return; // 线程函数结束，线程结束
					}

					if (poolMode_ == PoolMode::MODE_CACHED)
					{
						// 条件变量，超时返回了
						if (std::cv_status::timeout ==
							notEmpty_.wait_for(lock, std::chrono::seconds(1)))
						{
							auto now = std::chrono::high_resolution_clock().now();
							auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
							if (dur.count() >= THREAD_MAX_IDLE_TIME_SECONDS
								&& curThreadSize_ > initThreadSize_)
							{
								// 开始回收当前线程
								// 记录线程数量的相关变量的值修改
								// 把线程对象从线程列表容器中删除   没有办法 threadFunc《=》thread对象
								// threadid => thread对象 => 删除
								threads_.erase(threadid); // std::this_thread::getid()
								curThreadSize_--;
								idleThreadSize_--;

								std::cout << "threadid:" << std::this_thread::get_id() << " exit!\n";
								return;
							}
						}
					}
					else
					{
						// 等待notEmpty条件
						notEmpty_.wait(lock);
					}
				}


				// 从任务队列种取一个任务出来
				task = taskQue_.front();
				taskQue_.pop();
				taskSize_--;
				idleThreadSize_--;

				std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功...\n";

				// 如果依然有剩余任务，继续通知其它得线程执行任务
				if (taskQue_.size() > 0)
				{
					notEmpty_.notify_all();
				}

				// 取出一个任务，进行通知，通知可以继续提交生产任务
				notFull_.notify_all();
			} // 就应该把锁释放掉

			// 当前线程负责执行这个任务
			if (task != nullptr)
			{
				task(); // 执行function<void()> 
			}

			idleThreadSize_++;
			lastTime = std::chrono::high_resolution_clock().now(); // 更新线程执行完任务的时间
		}
	}

	bool checkRunningState() const
	{
		return isPoolRunning_;
	}

private:

	std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 线程列表
	int initThreadSize_;  // 初始的线程数量
	int threadSizeThreshHold_; // 线程数量上限阈值
	std::atomic_int curThreadSize_;	// 记录当前线程池里面线程的总数量
	std::atomic_int idleThreadSize_; // 记录空闲线程的数量

	using Task = std::function<void()>; // task 可调用对象
	std::queue<Task> taskQue_; // 任务队列
	std::atomic_int taskSize_; // 任务的数量
	int taskQueMaxThreshHold_;  // 任务队列数量上限阈值
	std::mutex taskQueMtx_; // 任务队列锁
	std::condition_variable notFull_; // 表示任务队列不满
	std::condition_variable notEmpty_; // 表示任务队列不空

	std::condition_variable exitCond_; // 表示要退出
	PoolMode poolMode_; // 当前线程池的工作模式
	std::atomic_bool isPoolRunning_; // 表示当前线程池的启动状态
};

#endif
