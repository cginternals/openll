
#include <gmock/gmock.h>


#include <openll/layout/LabelArea.h>

class LabelArea_test: public testing::Test
{
public:
};

TEST_F(LabelArea_test, Overlap)
{
    gloperate_text::LabelArea a {{0.f, 0.f}, {2.f, 2.f}, gloperate_text::RelativeLabelPosition::UpperLeft};
    gloperate_text::LabelArea b {{1.f, 1.f}, {2.f, 2.f}, gloperate_text::RelativeLabelPosition::UpperLeft};
    EXPECT_FLOAT_EQ(1.f, a.overlapArea(b));
    EXPECT_FLOAT_EQ(true, a.overlaps(b));
    EXPECT_FLOAT_EQ(9.f, a.paddedOverlapArea(b, {0.5f, 0.5f}));
    EXPECT_FLOAT_EQ(true, a.paddedOverlaps(b, {0.5f, 0.5f}));

    gloperate_text::LabelArea c {{0.f, 1.f}, {3.f, 2.f}, gloperate_text::RelativeLabelPosition::UpperLeft};
    gloperate_text::LabelArea d {{1.f, 0.f}, {1.f, 4.f}, gloperate_text::RelativeLabelPosition::UpperLeft};
    EXPECT_FLOAT_EQ(2.f, c.overlapArea(d));
    EXPECT_FLOAT_EQ(true, c.overlaps(d));
    EXPECT_FLOAT_EQ(4.f, c.paddedOverlapArea(d, {0.0f, 0.5f}));
    EXPECT_FLOAT_EQ(true, c.paddedOverlaps(d, {0.0f, 0.5f}));

    gloperate_text::LabelArea e {{0.f, 0.f}, {4.f, 2.f}, gloperate_text::RelativeLabelPosition::UpperLeft};
    gloperate_text::LabelArea f {{0.f, 3.f}, {2.f, 2.f}, gloperate_text::RelativeLabelPosition::UpperLeft};
    EXPECT_FLOAT_EQ(0.f, e.overlapArea(f));
    EXPECT_FLOAT_EQ(false, e.overlaps(f));
    EXPECT_FLOAT_EQ(0.f, e.paddedOverlapArea(f, {0.f, 0.1f}));
    EXPECT_FLOAT_EQ(false, e.paddedOverlaps(f, {0.f, 0.1f}));
    EXPECT_FLOAT_EQ(2.f, e.paddedOverlapArea(f, {0.0f, 0.5f}));
    EXPECT_FLOAT_EQ(true, e.paddedOverlaps(f, {0.0f, 0.5f}));
}
