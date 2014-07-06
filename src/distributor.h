/*
 * distributor.h
 *
 *  Created on: Jul 5, 2014
 *      Author: zks
 */

#ifndef ZKS_DISTRIBUTOR_H_
#define ZKS_DISTRIBUTOR_H_

#include <vector>
#include <thread>
#include <future>
#include <type_traits>

namespace zks {

template<typename Worker_, typename TaskIter_,
        typename ResType_ = std::result_of_t< Worker_(typename TaskIter_::value_type)>,
        class = std::enable_if_t< !std::is_void<ResType_>::value > >
inline
std::vector<ResType_> for_each(TaskIter_ beg, TaskIter_ end, Worker_ worker, int threads = 2,
        std::chrono::milliseconds wait_time = std::chrono::milliseconds{500} )
{
    std::vector<ResType_> ret_vec;
    std::vector<std::future<ResType_>> futures;
    TaskIter_ task_iter = beg;
    for (int i=0; i < threads && task_iter != end; ++i, ++task_iter) {
        futures.push_back(std::async(std::launch::async, worker, *task_iter));
    }

    for (;;) {
        bool finish_all{ true };
        for (auto& future : futures) {
            if (!future.valid()) {
                continue;
            }
            finish_all = false;
            auto status = future.wait_for(wait_time);
            if (status == std::future_status::ready) {
                ret_vec.push_back(future.get());
                if (task_iter != end) {
                    future = std::async(std::launch::async, worker, *task_iter);
                    ++task_iter;
                }
            }
        }
        if (finish_all) {
            break;
        }
    }

    return ret_vec;
}

template<typename Worker_, typename TaskIter_,
        typename ResType_ = std::result_of_t< Worker_(typename TaskIter_::value_type)>,
        class = std::enable_if_t< std::is_void<ResType_>::value > >
inline
void for_each(TaskIter_ beg, TaskIter_ end, Worker_ worker, int threads = 2,
        std::chrono::milliseconds wait_time = std::chrono::milliseconds{500} )
{
    std::vector<std::future<ResType_>> futures;
    TaskIter_ task_iter = beg;
    for (int i=0; i < threads && task_iter != end; ++i, ++task_iter) {
        futures.push_back(std::async(std::launch::async, worker, *task_iter));
    }

    for (;;) {
        bool finish_all{ true };
        for (auto& future : futures) {
            if (!future.valid()) {
                continue;
            }
            finish_all = false;
            auto status = future.wait_for(wait_time);
            if (status == std::future_status::ready) {
                future.get();
                if (task_iter != end) {
                    future = std::async(std::launch::async, worker, *task_iter);
                    ++task_iter;
                }
            }
        }
        if (finish_all) {
            break;
        }
    }

    return;
}

template<typename Task_>
inline
void distribute(void (worker)(const Task_&),
        const std::vector<Task_>& tasks,
        int threads = 2,
        std::chrono::milliseconds wait_time = std::chrono::milliseconds{500} )
{
    return for_each(tasks.cbegin(), tasks.cend(), worker, threads, wait_time);
}

template<typename ResType_, typename Task_>
inline
void distribute(ResType_ (worker)(const Task_&),
        const std::vector<Task_>& tasks,
        std::vector<ResType_>* ret_vec = 0,
        int threads = 2,
        std::chrono::milliseconds wait_time = std::chrono::milliseconds{500} )
{
    std::vector<ResType_> ret = for_each(tasks.cbegin(), tasks.cend(), worker, threads, wait_time);
    if(ret_vec) {
        (*ret_vec) = std::move(ret);
    }
    return;
}

} /* namespace zks */

#endif /* ZKS_DISTRIBUTOR_H_ */
