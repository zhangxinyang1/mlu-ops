/*************************************************************************
 * Copyright (C) 2021 by Cambricon, Inc. All rights reserved.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *************************************************************************/
#ifndef TEST_MLU_OP_GTEST_PB_GTEST_INCLUDE_THREAD_POOL_H_
#define TEST_MLU_OP_GTEST_PB_GTEST_INCLUDE_THREAD_POOL_H_

#include <mutex>               // NOLINT
#include <condition_variable>  // NOLINT
#include <future>              // NOLINT
#include <thread>              // NOLINT
#include <utility>             // NOLINT
#include <functional>
#include <queue>
#include <memory>
#include <vector>
#include <iostream>
#include <atomic>
#include "pb_test_tools.h"

namespace mluoptest {

class ThreadPool {
 public:
  ThreadPool()              = default;
  ThreadPool(ThreadPool &&) = default;
  explicit ThreadPool(size_t thread_num);
  ~ThreadPool();

  template <typename F, typename... Args>
  void enqueue(F &&f, Args &&... args) {
    auto task = std::make_shared<std::packaged_task<void()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    {
      std::lock_guard<std::mutex> lk(ctx_->mtx);
      ctx_->tasks.emplace([task]() { (*task)(); });
    }
    ctx_->cond.notify_all();
  }

 private:
  struct Context {
    std::mutex mtx;
    std::condition_variable cond;
    bool is_shutdown = false;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> workers;
  };
  std::shared_ptr<Context> ctx_ = nullptr;
};

}  // namespace mluoptest

#endif  // TEST_MLU_OP_GTEST_PB_GTEST_INCLUDE_THREAD_POOL_H_
