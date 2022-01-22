#include "simple_test.h"
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

int main(void)
{
  AddTestOperators();
  AddTestAssignment();
  AddTestConstructor();
  ki::testing::RunAllTests();
}