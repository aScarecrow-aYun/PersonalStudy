#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "thread_pool.h"

// 定义一个全局的互斥锁，专门用于保护 std::cout
std::mutex g_cout_mutex;

// 定义一个线程安全的打印宏，确保一整行输出不会被截断
#define SAFE_LOG(msg) \
    do { \
        std::lock_guard<std::mutex> lock(g_cout_mutex); \
        std::cout << msg << std::endl; \
    } while(0)

int heavy_compute(int task_id, int sleep_ms) {
	// 打印正在执行的线程ID，证明线程是被复用的
	SAFE_LOG("[Task " << task_id << "] 开始执行，线程ID: "
		<< std::this_thread::get_id() << " (预计耗时 " << sleep_ms << "ms)");

	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));

	// 模拟异常：任务ID为5的任务故意失败，测试线程池的异常隔离能力
	if (task_id == 5) {
		throw std::runtime_error("数据库连接超时！");
	}

	return task_id * 100;
}

int main() {
	auto& pool = ThreadPool::instance();
	SAFE_LOG("线程池初始化完成，当前空闲线程数: " << pool.idleThreadCount());
	SAFE_LOG("=========================================");

	// 保存所有任务的 future，用于后续获取结果
	std::vector<std::future<int>> futures;

	// 瞬间提交 10 个任务（注意：池子只有 4 个线程）
	for (int i = 1; i <= 10; ++i) {
		// 交替制造长短任务
		int sleep_time = (i % 2 == 0) ? 500 : 1000;

		// 提交任务，并立刻将返回的 future 存入 vector
		futures.push_back(pool.commit(heavy_compute, i, sleep_time));
	}

	// 故意让主线程休眠 10 毫秒，让工作线程有时间去拿任务并修改空闲数
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	SAFE_LOG("10个任务已全部提交！主线程休眠10ms后，当前空闲线程数: " << pool.idleThreadCount());
	SAFE_LOG("=========================================");

	// 主线程按照提交的顺序（0~9）遍历 future 获取结果
	for (int i = 0; i < futures.size(); ++i) {
		try {
			// 这里会阻塞：如果任务还没做完，主线程会等；如果做完了，立刻拿结果
			int result = futures[i].get();
			SAFE_LOG(">> 主线程收到 Task " << (i + 1) << " 的结果: " << result);
		}
		catch (const std::exception& e) {
			// 捕获子任务抛出的异常，不会导致主线程或线程池崩溃
			SAFE_LOG(">> 主线程收到 Task " << (i + 1) << " 的异常: " << e.what());
		}
	}

	SAFE_LOG("=========================================");
	SAFE_LOG("所有任务处理完毕，最终空闲线程数: " << pool.idleThreadCount());

	return 0;
}
