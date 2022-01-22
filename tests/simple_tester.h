#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

namespace ki::testing
{
  std::string PrettyPrint(size_t index)
  {
    switch (index)
    {
    case 1:
      return "1st";
    case 2:
      return "2nd";
    case 3:
      return "3rd";
    default:
      break;
    }
    return std::to_string(index) + "th";
  }

  class Test final
  {
  private:
    using func_t = std::function<void()>;

    struct TestInfo
    {
      func_t Functor;
      std::vector<bool> Conditions;

      TestInfo() noexcept = default;

      TestInfo(func_t functor, const std::vector<bool>& conditions)
        : Functor{functor}
        , Conditions{conditions}
      {
      }
    };

    std::unordered_map<std::string, TestInfo> TestsHolder;

  public:
    template <typename FunctorT>
    bool operator()(FunctorT functor) const noexcept
    {
      try
      {
        functor();
      }
      catch(const std::exception& err)
      {
        std::cout << "Exception occured in test body (" << err.what() << ')' << std::endl;
        return false;
      }
      catch (...)
      {
        std::cout << "Unknown exception occured in test body" << std::endl;
        return false;
      }

      return true;
    }

    template <typename StringT, typename FunctorT>
    void emplace(StringT&& string, FunctorT&& functor)
    {
      TestsHolder.emplace(std::make_pair(
        std::forward<StringT>(string),
        TestInfo{std::forward<FunctorT>(functor), {}}));
    }

    template <typename StringT>
    void add_condition(const StringT& testName, bool expression)
    {
      auto& testInfo = TestsHolder[testName];
      testInfo.Conditions.emplace_back(expression);
    }

    void RunAllTests() const noexcept
    {
      for (const auto& [name, testInfo] : TestsHolder)
      {
        std::cout << '[' << name << "] ..." << std::endl;
        auto result = operator()(testInfo.Functor);

        auto begin = testInfo.Conditions.begin();
        auto end = testInfo.Conditions.end();
        for (auto iter = begin; iter != end; ++iter)
        {
          if (!*iter)
          {
            result = false;
            auto index = iter - begin + 1;
            std::cout << '[' << name << ']'
              << " failed at " << PrettyPrint(index)
              << " condition" << std::endl;
          }
        }

        std::cout << '[' << name << ']' << (result ? " PASSED" : " FAILED") << std::endl;
      }
    }
  };

  inline Test TheTest;

  void RunAllTests() noexcept
  {
    TheTest.RunAllTests();
  }
}

#define TEST_BEGIN(TestName) void AddTest##TestName() { ki::testing::TheTest.emplace(#TestName, [] () -> void

#define TEST_END ); }

#define EXPECT_TRUE(TestName, expression) ki::testing::TheTest.add_condition(#TestName, expression)