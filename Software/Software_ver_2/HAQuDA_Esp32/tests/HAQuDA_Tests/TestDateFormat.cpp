#include <gtest/gtest.h>

#include <cstring>

#include "HAQuDA_DateFormat.h"

// ---------------------------------------------------------------------------------------------
// HAQuDA_FormatDateTime(): "DD.MM.YYYY HH:MM:SS"
// ---------------------------------------------------------------------------------------------

TEST(DateFormatTest, FormatNormalDateReturnsTrue) {
	const size_t outSize = 20;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = true;

	char out[20];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatNormalDateWritesExactString) {
	const size_t outSize = 20;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const char *exp_text = "15.03.2025 14:25:36";

	char out[20];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatSingleDigitFieldsAreZeroPadded) {
	const size_t outSize = 20;
	const int date = 1, month = 2, year = 2025, hour = 3, min = 4, sec = 5;

	const char *exp_text = "01.02.2025 03:04:05";

	char out[20];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatShortYearIsZeroPaddedToFourDigits) {
	const size_t outSize = 20;
	const int date = 10, month = 11, year = 25, hour = 12, min = 13, sec = 14;

	const char *exp_text = "10.11.0025 12:13:14";

	char out[20];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatAllZeroFieldsAreZeroPadded) {
	const size_t outSize = 20;
	const int date = 0, month = 0, year = 0, hour = 0, min = 0, sec = 0;

	const char *exp_text = "00.00.0000 00:00:00";

	char out[20];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatLargestFieldsFitFixedLength) {
	const size_t outSize = 20;
	const int date = 99, month = 99, year = 9999, hour = 99, min = 99, sec = 99;

	const char *exp_text = "99.99.9999 99:99:99";

	char out[20];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatResultLengthIsDateTimeTextLen) {
	const size_t outSize = 20;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const size_t exp_len = DATE_TIME_TEXT_LEN;

	char out[20];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_len, strlen(out));
}

// ---------------------------------------------------------------------------------------------
// Buffer size
// ---------------------------------------------------------------------------------------------

TEST(DateFormatTest, FormatBufferOfTextLenPlusTerminatorFits) {
	const size_t outSize = DATE_TIME_TEXT_LEN + 1;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = true;

	char out[DATE_TIME_TEXT_LEN + 1];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatBufferOfTextLenIsTruncationAndFails) {
	const size_t outSize = DATE_TIME_TEXT_LEN;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char out[DATE_TIME_TEXT_LEN];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatTruncationLeavesEmptyOutput) {
	const size_t outSize = DATE_TIME_TEXT_LEN;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const char *exp_text = "";

	char out[DATE_TIME_TEXT_LEN];
	memset(out, 'X', sizeof(out));

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatOneByteBufferFailsWithEmptyOutput) {
	const size_t outSize = 1;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;
	const char *exp_text = "";

	char out[1] = {'X'};

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatLargerBufferStillWritesExactString) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const char *exp_text = "15.03.2025 14:25:36";

	char out[64];

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatZeroSizeFails) {
	const size_t outSize = 0;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char out[20];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatZeroSizeDoesNotTouchBuffer) {
	const size_t outSize = 0;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const char exp_first = 'X';

	char out[20];
	memset(out, 'X', sizeof(out));

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_first, out[0]);
}

TEST(DateFormatTest, FormatNullBufferFails) {
	const size_t outSize = 20;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char *out = nullptr;

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

// ---------------------------------------------------------------------------------------------
// Fields wider than their digits
// ---------------------------------------------------------------------------------------------

TEST(DateFormatTest, FormatFiveDigitYearFails) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 10000, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatFiveDigitYearLeavesEmptyOutput) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 10000, hour = 14, min = 25, sec = 36;

	const char *exp_text = "";

	char out[64];
	memset(out, 'X', sizeof(out));

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatThreeDigitHourFails) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 2025, hour = 100, min = 25, sec = 36;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatThreeDigitHourLeavesEmptyOutput) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 2025, hour = 100, min = 25, sec = 36;

	const char *exp_text = "";

	char out[64];
	memset(out, 'X', sizeof(out));

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatThreeDigitDateFails) {
	const size_t outSize = 64;
	const int date = 100, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatThreeDigitSecondFails) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = 100;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

// ---------------------------------------------------------------------------------------------
// Negative fields: "-1" is two characters, so only an explicit check rejects them
// ---------------------------------------------------------------------------------------------

TEST(DateFormatTest, FormatNegativeDateFails) {
	const size_t outSize = 64;
	const int date = -1, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatNegativeDateLeavesEmptyOutput) {
	const size_t outSize = 64;
	const int date = -1, month = 3, year = 2025, hour = 14, min = 25, sec = 36;

	const char *exp_text = "";

	char out[64];
	memset(out, 'X', sizeof(out));

	HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_STREQ(exp_text, out);
}

TEST(DateFormatTest, FormatNegativeSecondFails) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = 2025, hour = 14, min = 25, sec = -5;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}

TEST(DateFormatTest, FormatNegativeYearFails) {
	const size_t outSize = 64;
	const int date = 15, month = 3, year = -2025, hour = 14, min = 25, sec = 36;

	const bool exp_res = false;

	char out[64];

	bool res;

	res = HAQuDA_FormatDateTime(out, outSize, date, month, year, hour, min, sec);

	EXPECT_EQ(exp_res, res);
}
