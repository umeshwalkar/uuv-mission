#ifndef DATANODE_HPP
#define DATANODE_HPP

#include <mutex>
#include <chrono>
#include <cmath>

template<typename T>
class DataNode
{
public:

    DataNode()
    {
        startTime =
            std::chrono::steady_clock::now();

        lastUpdate = startTime;
    }

    bool set(
        const T& newValue,
        double threshold = 0.0)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (hasValue)
        {
            if (!hasChanged(
                    value,
                    newValue,
                    threshold))
            {
                return false;
            }
        }

        value = newValue;

        hasValue = true;

        updateCount++;

        lastUpdate =
            std::chrono::steady_clock::now();

        return true;
    }

    T get()
    {
        std::lock_guard<std::mutex> lock(mutex);

        return value;
    }

    bool stale(uint64_t timeoutMs)
    {
        auto now =
            std::chrono::steady_clock::now();

        auto elapsed =
            std::chrono::duration_cast<
                std::chrono::milliseconds>(
                    now - lastUpdate)
                .count();

        return elapsed > timeoutMs;
    }

    uint64_t getUpdateCount()
    {
        return updateCount;
    }

    double getFrequencyHz()
    {
        auto now =
            std::chrono::steady_clock::now();

        auto elapsed =
            std::chrono::duration_cast<
                std::chrono::milliseconds>(
                    now - startTime)
                .count();

        if (elapsed == 0)
            return 0;

        return
            (1000.0 * updateCount)
            / elapsed;
    }

private:

    bool hasChanged(
        const T& oldVal,
        const T& newVal,
        double threshold)
    {
        return oldVal != newVal;
    }

private:

    T value{};

    bool hasValue = false;

    uint64_t updateCount = 0;

    std::mutex mutex;

    std::chrono::steady_clock::time_point
        startTime;

    std::chrono::steady_clock::time_point
        lastUpdate;
};

//
// FLOAT SPECIALIZATION
//

template<>
inline bool DataNode<float>::hasChanged(
    const float& oldVal,
    const float& newVal,
    double threshold)
{
    return std::fabs(
               newVal - oldVal)
           > threshold;
}

//
// DOUBLE SPECIALIZATION
//

template<>
inline bool DataNode<double>::hasChanged(
    const double& oldVal,
    const double& newVal,
    double threshold)
{
    return std::fabs(
               newVal - oldVal)
           > threshold;
}

#endif // DATANODE_HPP
