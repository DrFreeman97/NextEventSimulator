#include "EventHandler.hpp"
#include "Measure.hpp"
#include "rngs.hpp"
#include <future>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(TestEventHandler, test_handling)
{
    std::mutex mutex;
    EventHandler handler;
    handler += new FunctionHandler<>([&mutex]() { mutex.unlock(); });
    auto ret = std::async([&]() {
        sleep(1);
        handler.Invoke();
    });
    mutex.lock();
}

void fnc(bool *a)
{
    *a = true;
}

TEST(TestEventHandler, test_handler_deletion)
{
    bool a = false;
    EventHandler<bool *> handler;
    FunctionHandler<bool *> fnc([](bool *a) { *a = false; });
    handler += &fnc;
    handler -= &fnc;
    handler.Invoke(&a);
    ASSERT_EQ(a, false);
}

TEST(TestMeasure, test_measure_add)
{
    Measure<> measure{"dummy", ""};

    for (int i = 0; i <= 100; i++)
    {
        measure(i);
    }
    ASSERT_EQ(50, measure.mean());
    ASSERT_EQ(850, measure.variance());
    ASSERT_EQ(101, measure.count());
}

TEST(TestRandom, test_random_generator)
{
    RandomStream stream{};
    long i;
    long x;
    double u;
    char ok = 0;
    stream.SelectStream(0); /* select the default stream */
    stream.PutSeed(1);      /* and set the state to 1    */
    for (i = 0; i < 10000; i++)
        u = stream.Random();
    stream.GetSeed(&x); /* get the new state value   */
    ok = (x == CHECK);  /* and check for correctness */

    stream.SelectStream(1);              /* select stream 1                 */
    stream.PlantSeeds(1);                /* set the state of all streams    */
    stream.GetSeed(&x);                  /* get the state of stream 1       */
    ASSERT_TRUE(ok = ok && (x == A256)); /* x should be the jump multiplier */
}