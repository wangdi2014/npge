/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "thread_group.hpp"

namespace bloomrepeats {

static void process_tasks(TaskGenerator& task_generator,
                          boost::mutex* mutex) {
    while (true) {
        Task task;
        if (mutex) {
            mutex->lock();
        }
        task = task_generator();
        if (mutex) {
            mutex->unlock();
        }
        if (!task.empty()) {
            task();
        } else {
            break;
        }
    }
}

static void process_tasks_one_thread(TaskGenerator& task_generator) {
    while (true) {
        Task task;
        task = task_generator();
        if (!task.empty()) {
            task();
        } else {
            break;
        }
    }
}

void do_tasks(TaskGenerator& task_generator, int workers) {
    if (workers == 1) {
        process_tasks_one_thread(task_generator);
    } else {
        boost::mutex mutex;
        boost::thread_group threads;
        for (int i = 1; i < workers; i++) {
            threads.create_thread(boost::bind(process_tasks,
                                              boost::ref(task_generator),
                                              &mutex));
        }
        process_tasks(task_generator, &mutex);
        threads.join_all();
    }
}

class VectorTaskGenerator {
public:
    VectorTaskGenerator(Tasks& tasks):
        tasks_(tasks)
    { }

    Task operator()() {
        Task task;
        if (!tasks_.empty()) {
            task = tasks_.back();
            tasks_.pop_back();
        }
        return task;
    }

private:
    Tasks& tasks_;
};

void do_tasks(Tasks& tasks, int workers) {
    VectorTaskGenerator task_generator(tasks);
    TaskGenerator task_generator_2(task_generator);
    do_tasks(task_generator_2, workers);
}

}
