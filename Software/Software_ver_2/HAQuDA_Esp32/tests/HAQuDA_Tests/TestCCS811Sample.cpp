#include <gtest/gtest.h>

#include "HAQuDA_CCS811Sample.h"

TEST(CCS811SampleTest, WarmUpZeroIsNotASample) {
	EXPECT_FALSE(HAQuDA_IsCCS811Sample(0));
}

TEST(CCS811SampleTest, BelowTheFloorIsNotASample) {
	EXPECT_FALSE(HAQuDA_IsCCS811Sample(399));
}

TEST(CCS811SampleTest, TheFloorItselfIsASample) {
	EXPECT_TRUE(HAQuDA_IsCCS811Sample(400));
}

TEST(CCS811SampleTest, ARealReadingIsASample) {
	EXPECT_TRUE(HAQuDA_IsCCS811Sample(1200));
	EXPECT_TRUE(HAQuDA_IsCCS811Sample(UINT16_MAX));
}
