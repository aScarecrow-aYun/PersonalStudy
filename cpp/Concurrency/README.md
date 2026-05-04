# C++并发三剑客：std::async、std::future与std::promise

在C++11中，异步编程主要依赖于“三剑客”：`std::async`（启动异步任务）、`std::future`（获取异步结果）以及`std::promise`（在线程间传递结果）。

# 一、std::async

`std::async`是一个用于异步执行函数的模板函数，它返回一个`std::future`对象，该对象用于获取函数的返回值。

```cpp
#include <iostream>
#include <future>
#include <chrono>

// 定义一个异步任务
std::string my_task(std::string query) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return "Data: " + query;
}

int main()
{
    // 使用std::async异步执行任务
    std::future<std::string> resultFromDB = std::async(std::launch::async, my_task, "");

    std::cout << "Do something else..." << std::endl;

    // 从future对象中获取数据
    std::string dbData = resultFromDB.get();
    std::cout << dbData << std::endl;

    return 0;
}
```

## std::async启动策略

`std::async`函数可以接收几个不同的启动策略，这些策略在`std::launch`枚举中定义：

1. `std::launch::async`：强制任务在新线程上异步执行（非阻塞）

2. `std::launch::deferred`：任务将被延迟执行。只有在调用`std::future::get()`或`std::future::wait()`时，任务才会在当前线程中同步执行。换句话说，任务将在需要结果时同步执行

3. `std::launch::async | std::launch::deferred`：默认策略。这意味着任务可能异步执行，也可能延迟执行，具体由操作系统根据当前CPU负载情况决定

---

# 二、std::future

`std::future`代表一个异步操作的结果，它可以用于从异步任务中获取返回值或异常。注意，`std::future`只能被移动，不能被复制，且通常只允许一个线程调用`get()`。

1. `std::future::get()`
   
   - 这是一个阻塞调用，用于获取`std::future`对象表示的值或异常。
   
   - 如果异步任务还没有完成，`get()`会阻塞当前线程，直到任务完成
   
   - 如果异步任务已经完成，`get()`会立即返回任务的结果
   
   - 注意：`get()`只能被调用一次。一旦`get()`被调用，`std::future`对象的状态就会被消耗，就不能再被用来获取结果。`std::future::wait()`

2. `std::future::wait()`
   
   - 也是一个阻塞调用，但它与`get()`的主要区别在于它不会返回任务的结果，只是等待异步任务完成
   
   - 如果任务已经完成，`wait()`会立即返回
   
   - 如果任务没有完成，`wait()`会阻塞当前线程，直到任务完成
   
   - 与`get()`的另一个区别是：`wait()`可以被调用多次，它不会消耗`std::future`对象的状态

3. `std::future::wait_for(std::chrono::duration)`
   
   - 阻塞当前线程一定的时间（等多久）：
     
     - 如果在这段时间内异步任务完成，则立即返回
     
     - 如果超时后任务仍未完成，也会返回
   
   - 返回`std::future_status`枚举：
     
     - `ready`：任务已完成
     
     - `timeout`：超时，任务未完成
     
     - `deferred`：任务被延迟执行，并且尚未开始

4. `std::future::wait_until(std::chrono::time_point)`
   
   - 阻塞当前线程直到一个绝对时间点（等到几点）
   
   - 返回值和`std::future::wait_for(std::chrono::duration)`一致
   
   - 这两个函数最大优势在于**非阻塞的轮训机制**。例如在GUI程序或游戏引擎中，我们不能让主线程被`get()`卡死，这时就可以用`wait_for(0ms)`来定期检查后台任务是否完成，如果没完成就继续处理渲染、响应用户操作等其他事务，实现了平滑的异步交互。

## 将任务和future关联：std::packaged_task

`std::packaged_task`是一个可调用目标，它包装了一个任务，该任务可以在另一个线程上运行。它可以捕获任务的返回值或异常，并将其存储在`std::future`对象中。

以下是使用`std::packaged_task`和`std::future`的基本步骤和实例代码：

1. 创建一个`std::packaged_task`对象，该对象包装了要执行的任务

2. 调用`std::packaged_task`对象的`get_future()`方法，获取与任务关联的`std::future`对象

3. 在另一个线程上调用`std::packaged_task`对象的`operator()`，以执行任务

4. 在需要任务结果的地方，调用与任务关联的`std::future`对象的`get()`方法，以获取任务的返回值或异常

```cpp
#include <iostream>
#include <future>
#include <chrono>

int my_task() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "my task run 5 s" << std::endl;
    return 42;
}

void main() {    
    std::packaged_task<int()> task(my_task);        // 创建一个包装了任务的 std::packaged_task 对象  

    std::future<int> fut = task.get_future();    // 获取与任务关联的 std::future 对象  

    std::thread t(std::move(task));                    // 在另一个线程上执行任务  
    t.detach();                                        // 将线程与主线程分离，以便主线程可以等待任务完成  

    int value = fut.get();                        // 等待任务完成并获取结果  
    std::cout << "The result is: " << value << std::endl;
}
```

---

# 三、std::promise

`std::promise`用于在某一线程中设置某个值或异常，而`std::future`用于在另一线程中获取这个值或异常。

1. `set_value()`
   
   ```cpp
   #include <iostream>
   #include <thread>
   #include <future>
   
   void my_task(std::promise<int> prom) {
      // 设置 promise 的值
      prom.set_value(10);
   }
   int main() {
      // 创建一个 promise 对象
      std::promise<int> prom;
   
      // 获取与 promise 相关联的 future 对象
      std::future<int> fut = prom.get_future();
   
      // 在新线程中设置 promise 的值
      std::thread t(my_task, std::move(prom));
   
      // 在主线程中获取 future 的值
      std::cout << "Waiting for the thread to set the value...\n";
      std::cout << "Value set by the thread: " << fut.get() << '\n';
   
      t.join();
   
      return 0;
   }
   ```

> 核心区别：前面的`std::packaged_task`包装的任务必须全部执行完后，`get()`才能拿到返回值。而`std::promise`的好处在于：只要子线程执行了`set_value()`，主线程就可以通过`get_value()`拿到值，不必等子线程全部执行完成。

2. `set_exception()`
   
   用于在子线程中捕获异常，并传递给主线程处理。
   
   ```cpp
   #include <iostream>
   #include <thread>
   #include <future>
   
   void my_task(std::promise<void> prom) {
      try {
          throw std::runtime_error("An error occurred!");
      }
      catch (...) {
          // 设置 promise 的异常
          prom.set_exception(std::current_exception());
      }
   }
   
   int main() {
      // 创建一个 promise 对象
      std::promise<void> prom;
   
      // 获取与 promise 相关联的 future 对象
      std::future<void> fut = prom.get_future();
   
      // 在新线程中设置 promise 的异常
      std::thread t(my_task, std::move(prom));
   
      try {
          std::cout << "Waiting for the thread to set the exception...\n";
   
          // 在主线程中获取 future 的异常
          // 输出：Exception set by the thread: An error occurred!
          fut.get();
      }
      catch (const std::exception& e) {
          std::cout << "Exception set by the thread: " << e.what() << '\n';
      }
   
      t.join();
      return 0;
   }
   ```

## 四、std::shared_future

由于 `std::future` 是独占的（只能调用一次 `get()`），当我们需要多个线程等待同一个执行结果时，需要使用 `std::shared_future`。`std::shared_future` 支持复制，多个线程可以安全地各自调用 `get()` 获取相同的结果。

```cpp
#include <iostream>
#include <thread>
#include <future>

void myFunction(std::promise<int>&& promise) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    promise.set_value(42);
}

void threadFunction(std::shared_future<int> future) {
    try {
        // shared_future 的 get() 可以被多个线程安全地调用多次
        int result = future.get();
        std::cout << "Result: " << result << std::endl;
    }
    catch (const std::future_error& e) {
        std::cout << "Future error: " << e.what() << std::endl;
    }
}

void main() {
    std::promise<int> promise;
    std::shared_future<int> future = promise.get_future();

    // 将 promise 移动到线程中
    std::thread myThread1(myFunction, std::move(promise));

    // shared_future 支持值传递（复制）
    std::thread myThread2(threadFunction, future);
    std::thread myThread3(threadFunction, future);

    myThread1.join();
    myThread2.join();
    myThread3.join();
}
```

---

# 对线程池代码的分析

```cpp
// 原来的代码：虽然在外面用了std::forward（意图是完美转发），但std::bind内部的机制是：默认按值存储所有传入的参数
auto task = std::make_shared<std::packaged_task<RetType()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
```

## 场景一：引用丢失（你想修改外部变量，但修改失败）

```cpp
// 假设你有一个需求：在线程池里跑一个任务，这个任务需要修改主线程中的一个变量

int count = 0;

// 一个期望通过引用修改变量的函数
void add_count(int& num) {
    num += 10;
}

// 提交任务 (使用原始的 std::bind 代码)
pool.commit(add_count, count); 

// 过程分析如下：
// count是一个左值，std::bind拿到count后，拷贝了count的副本
// 等到线程池执行这个任务时，它处理的是这个count的副本，而不是传入的count
// 结果就是线程池里把count的副本的值增加了10，而主线程的count依然是0
// 补救方法：pool.commit(add_count, std::ref(count));这么做的目的是让std::bind不拷贝
// 但一旦调用者忘了写std::ref，程序就bug了，并且遇到场景二的时候也会编译报错
```

## 场景二：移动语义失效（遇到独占资源直接编译报错）

```cpp
// 假设你的参数是一个只能移动、不能拷贝的对象，比如std::unique_ptr

void do_something(std::unique_ptr<int> ptr) {
    // 处理智能指针
}

auto p = std::make_unique<int>(42);
pool.commit(do_something, std::move(p));

// 过程分析如下：
// 明明用了std::move(p)，表示“我要把p的所有权移交出去，不要拷贝”
// std::bind内部拿到这个右值后，依然试图按值存储它
// 按值存储意味着需要拷贝构造。但是std::unique_ptr是禁止拷贝的，所以这会编译失败
```

# 改进后的代码

```cpp
#include <tuple> // C++14 for std::apply

auto task = std::make_shared<std::packaged_task<RetType()>>(
    [f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(f, args);
    }
);


// f = std::forward<F>(f)：Lambda 的变量捕获。如果外面传进来的是左值，它就在Lambda内部拷贝一份；如果外面传进来的是右值，它就在Lambda内部移动一份。绝不强制转换属性。
// args = std::make_tuple(...)：把所有参数打包进一个std::tuple。这个tuple是一个“完美的容器”，它原封不动地记录了你传进来的参数是引用还是值。
// std::apply(f, args)：当任务在线程池里真正执行时，std::apply会把tuple里的参数像剥洋葱一样，原样展开传给函数f（如果tuple里存的是引用/右值，传给f的就是引用/右值。）
```

---

学习参考链接：

> [恋恋风辰官方博客](https://llfc.club/articlepage?id=2VWIJgH3zKEww0BpLnYQX0NMpQ9)
> 
> [C++ 并发编程(7) 并发三剑客async,promise和future_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV18w411i74T/?spm_id_from=333.1007.top_right_bar_window_history.content.click&vd_source=4bdb551b219dd3278ebb7f1179036dc4)
