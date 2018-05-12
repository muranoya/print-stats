#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

static std::map<time_t, unsigned long long int> hist;
static std::atomic<unsigned long long int> count;

static double
agg(time_t now, int loop_num)
{
    unsigned long long int sum = 0ULL;
    bool inc_nan = false;
    for (int i = 0; i < loop_num; ++i)
    {
        const auto iter = hist.find(now - i * 10);
        if (iter == hist.cend())
        {
            inc_nan = true;
            break;
        }

        sum += iter->second;
    }
    return (inc_nan
            ? std::numeric_limits<double>::quiet_NaN()
            : static_cast<double>(sum) / loop_num);
}

static std::tuple<double, double, double>
agg_stat(time_t now)
{
    const auto dat = std::make_tuple(agg(now, 1), agg(now, 6), agg(now, 60));

    for (auto iter = hist.cbegin(); iter != hist.cend(); ++iter)
    {
        if (!(now - 10 * 60 <= iter->first && iter->first <= now))
        {
            hist.erase(iter);
        }
    }

    return dat;
}

static void
print_loop()
{
    for (;;)
    {
        std::mutex mtx;
        std::condition_variable cond;
        std::unique_lock<std::mutex> unique_mtx(mtx);
        cond.wait_until(unique_mtx,
                std::chrono::steady_clock::now() + std::chrono::seconds(10));

        const time_t now = time(nullptr);
        hist.insert(std::make_pair(now, count.exchange(0ULL)));

        const auto tpl = agg_stat(now);
        tm pnow;
        localtime_r(&now, &pnow);
        std::printf("%04d-%02d-%02d %02d:%02d:%02d\t"
                "%012.3f\t%012.3f\t%012.3f\n",
                pnow.tm_year + 1900, pnow.tm_mon + 1, pnow.tm_mday,
                pnow.tm_hour,        pnow.tm_min,     pnow.tm_sec,
                std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
}

int
main(int argc, char *argv[])
{
    std::thread thr(print_loop);

    count = 0ULL;
    for (;;)
    {
        std::string str;
        if (std::getline(std::cin, str)) count++;
    }

    thr.join();

    return EXIT_SUCCESS;
}
