#pragma once

#include <atomic>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <tuple> // for std::apply

/* 注：该线程池仅适用于CPU密集型任务而非IO密集型任务 */
class ThreadPool {
public:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	static ThreadPool& instance() {
		static ThreadPool ins;
		return ins;
	}

	using Task = std::packaged_task<void()>;

	~ThreadPool() {
		stop();
	}

	template <class F, class... Args>
	auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
		using RetType = decltype(f(args...));

		if (stop_.load()) {
			throw std::runtime_error("ThreadPool is stopped, cannot commit task.");
		}

		auto task = std::make_shared<std::packaged_task<RetType()>>(
			[f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
				return std::apply(f, args);
			}
		);

		std::future<RetType> ret = task->get_future();
		{
			std::lock_guard<std::mutex> lock(mtx_);
			tasks_.emplace([task]() { (*task)(); });
		}
		cond_.notify_one();
		return ret;
	}

	int idleThreadCount() {
		return idle_thread_num_.load();
	}

private:
	ThreadPool(unsigned int num = std::thread::hardware_concurrency()) : stop_(false) {
		idle_thread_num_ = (num < 1) ? 1 : num;
		total_thread_num_ = idle_thread_num_;
		start();
	}

	void start() {
		for (int i = 0; i < total_thread_num_; ++i) {
			pool_.emplace_back([this]() {
				while (!this->stop_.load()) {
					Task task;
					{
						std::unique_lock<std::mutex> lock(mtx_);
						// 等待直到有任务或者停止信号
						this->cond_.wait(lock, [this] {
							return this->stop_.load() || !this->tasks_.empty();
							});

						if (this->tasks_.empty() && this->stop_.load())
							return;

						task = std::move(this->tasks_.front());
						this->tasks_.pop();

						this->idle_thread_num_--;
					}

					task();

					this->idle_thread_num_++;
				}
				});
		}
	}

	void stop() {
		stop_.store(true);
		cond_.notify_all();
		for (auto& td : pool_) {
			if (td.joinable()) {
				std::cout << "join thread " << td.get_id() << std::endl;
				td.join();
			}
		}
	}

private:
	std::mutex               mtx_;
	std::condition_variable  cond_;
	std::atomic_bool         stop_;
	std::atomic_int          idle_thread_num_;
	int                      total_thread_num_;
	std::queue<Task>         tasks_;
	std::vector<std::thread> pool_;
};