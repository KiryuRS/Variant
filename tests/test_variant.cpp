#include "simple_tester.h"
#include "../variant/variant_impl.h"

TEST_BEGIN(Constructor)
{
  ki::Variant var1;
  ki::Variant var2{'A'};
  EXPECT_TRUE(Constructor, var2.get_type_id() == 0);

  ki::Variant var3{var2};
  EXPECT_TRUE(Constructor, var2 == var3);

  ki::Variant var4{std::move(var2)};
  EXPECT_TRUE(Constructor, var4.get_value<char>() == 'A');

  ki::Variant var5{static_cast<long>(100)};
  EXPECT_TRUE(Constructor, var5.get_type_id() == 3);

  const ki::Variant var6{var4};
  EXPECT_TRUE(Constructor, var6.get_value<char>() == 'A');

  std::vector<int> container{1,2,3,4,5};
  ki::Variant var7{container};
  EXPECT_TRUE(Constructor, var7.get_value<std::vector<int>>() == container);

  ki::Variant var8{"Hello World"};
  EXPECT_TRUE(Constructor, !std::strcmp(var8.get_value<const char*>(), "Hello World"));

  var8 = std::string{"Goodbye World"};
  EXPECT_TRUE(Constructor, var8.get_value<std::string>() == std::string{"Goodbye World"});
}
TEST_END

TEST_BEGIN(Assignment)
{
  ki::Variant var1{'B'};
  ki::Variant var2{static_cast<int>(50)};
  var1 = 'C';
  var1 = static_cast<short>(81);
  EXPECT_TRUE(Assignment, var1.get_value<short>() == 81);

  var1 = var2;
  EXPECT_TRUE(Assignment, var1.get_type_id() == 2);
  EXPECT_TRUE(Assignment, var1.get_value<int>() == 50);

  ki::Variant var3{static_cast<long long>(1000)};
  var1 = std::move(var3);
  EXPECT_TRUE(Assignment, var1.get_type_id() == 4);
  EXPECT_TRUE(Assignment, var1.get_value<long long>() == 1000);
}
TEST_END

TEST_BEGIN(Operators)
{
  ki::Variant var1;
  ki::Variant var2{'D'};
  ki::Variant var3{static_cast<short>(10)};
  EXPECT_TRUE(Operators, var1 != var2);

  ki::Variant var4{var1};
  ki::Variant var5{var2};
  EXPECT_TRUE(Operators, var1 == var4);
  EXPECT_TRUE(Operators, var2 == var5);

  short s{var3};
  EXPECT_TRUE(Operators, s == 10);
  EXPECT_TRUE(Operators, var3);
  EXPECT_TRUE(Operators, !var1);

  ki::Variant var6{123};
  int i = 123;
  EXPECT_TRUE(Operators, var6 == i);
}
TEST_END

void Foo(int, int) { }
TEST_BEGIN(Containers)
{
  std::vector<int> vec{4,5,6,7};
  int i = 0;
  int* pi = &i;
  std::vector<ki::Variant> container
  {
    'A',
    static_cast<int>(20),
    static_cast<short>(1),
    vec,
    Foo,
    pi
  };

  EXPECT_TRUE(Containers, container[0].get_value<char>() == 'A');
  EXPECT_TRUE(Containers, container[3].get_value<std::vector<int>>() == vec);
  EXPECT_TRUE(Containers, container[5].get_value<int*>() == pi);

  auto functor = container[4].get_value<void(*)(int, int)>();
  functor(1,2);
}
TEST_END

int main(void)
{
  AddTestContainers();
  AddTestOperators();
  AddTestAssignment();
  AddTestConstructor();
  ki::testing::RunAllTests();
}