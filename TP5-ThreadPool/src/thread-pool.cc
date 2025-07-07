#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false), numThreads(numThreads) {
    dt = thread([this] { dispatcher(); });
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].id = i;
        wts[i].available = true;
        wts[i].ts = thread([this, i] { worker(i); });
    }
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    {
        lock_guard<mutex> lock(queueLock);
        tasksQueue.push(thunk);
        ++tasksPending;
    }
    taskAvailable.signal();
}

void ThreadPool::dispatcher() {
    while (true) {
        taskAvailable.wait();

        if (done) break;

        function<void(void)> task;
        {
            lock_guard<mutex> lock(queueLock);
            if (!tasksQueue.empty()) {
                task = tasksQueue.front();
                tasksQueue.pop();
            } else {
                continue;
            }
        }

        // Esperar a que haya un worker disponible
        while (true) {
            for (auto& wt : wts) {
                lock_guard<mutex> lock(wt.mtx);
                if (wt.available) {
                    wt.thunk = task;
                    wt.available = false;
                    wt.ready.signal();
                    goto tarea_asignada;
                }
            }
            // Si ningún worker está libre, dormir un poco y volver a intentar
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    tarea_asignada:
        continue;
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].ready.wait();

        if (done) break;

        function<void(void)> task;
        {
            lock_guard<mutex> lock(wts[id].mtx);
            task = wts[id].thunk;
        }
        if (task) task();

        {
            lock_guard<mutex> lock(wts[id].mtx);
            wts[id].available = true;
        }

        {
            lock_guard<mutex> lock(waitMtx);
            --tasksPending;
            if (tasksPending == 0) waitCv.notify_all();
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(waitMtx);
    waitCv.wait(lock, [this]() { return tasksPending == 0; });
}

ThreadPool::~ThreadPool() {
    wait();
    done = true;
    taskAvailable.signal(); // Despierta al dispatcher para salir
    if (dt.joinable()) dt.join();

    for (auto& wt : wts) {
        wt.ready.signal(); // Despierta a los workers para que terminen
        if (wt.ts.joinable()) wt.ts.join();
    }
}
