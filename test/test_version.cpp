#define BOOST_TEST_MODULE RVN_METADATA_VERSION
#include <boost/test/unit_test.hpp>

#include "test_helpers.h"

bool check_comparison(const Version& a, const Version& b, bool compatible, const Version::Comparison& comparison) {
	auto cmp = a.compare(b);

	return cmp.is_compatible() == compatible && cmp.detail == comparison.detail;
}

bool check_comparison(const std::string& a, const std::string& b, bool compatible, const Version::Comparison& comparison) {
	return check_comparison(Version::from_string(a), Version::from_string(b), compatible, comparison);
}

BOOST_AUTO_TEST_CASE(from_string)
{
	BOOST_CHECK(check_version_strict_equality("1.2.3", Version(1, 2, 3)));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo",
	    Version(
	        1, 2, 3,
	        {{"foo"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo-bar",
	    Version(
	        1, 2, 3,
	        {{"foo-bar"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo.bar",
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo.bar.42",
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3+foo",
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3+foo-bar",
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo-bar"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3+foo.bar",
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo"}, {"bar"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3+foo.bar.42",
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo"}, {"bar"}, {42}}
	    )
	));


	BOOST_CHECK(check_version_strict_equality("1.2.3-foo.bar.42+foo",
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo.bar.42+foo-bar",
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo-bar"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo.bar.42+foo.bar",
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo"}, {"bar"}}
	    )
	));

	BOOST_CHECK(check_version_strict_equality("1.2.3-foo.bar.42+foo.bar.42",
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo"}, {"bar"}, {42}}
	    )
	));

	BOOST_CHECK_THROW(Version::from_string("1-extra"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2-extra"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2.3.4"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2.3-ab*d"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2.3+ab*d"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("foo"), reven::metadata::MetadataError);

	BOOST_CHECK_THROW(Version::from_string("01.2.3"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.02.3"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2.03"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2.3-042"), reven::metadata::MetadataError);
	BOOST_CHECK_THROW(Version::from_string("1.2.3+042"), reven::metadata::MetadataError);

	BOOST_CHECK_THROW(Version::from_string("100000000000000000000000000.0.0"), std::out_of_range);
	BOOST_CHECK_THROW(Version::from_string("0.100000000000000000000000000.0"), std::out_of_range);
	BOOST_CHECK_THROW(Version::from_string("0.0.100000000000000000000000000"), std::out_of_range);
	BOOST_CHECK_THROW(Version::from_string("0.0.0-100000000000000000000000000"), std::out_of_range);
	BOOST_CHECK_THROW(Version::from_string("0.0.0+100000000000000000000000000"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(to_string)
{
	BOOST_CHECK(Version(1, 2, 3).to_string() == "1.2.3");

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {{"foo"}}
	    ).to_string() == "1.2.3-foo"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}}
	    ).to_string() == "1.2.3-foo.bar"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}}
	    ).to_string() == "1.2.3-foo.bar.42"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo"}}
	    ).to_string() == "1.2.3+foo"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo"}, {"bar"}}
	    ).to_string() == "1.2.3+foo.bar"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {},
	        {{"foo"}, {"bar"}, {42}}
	    ).to_string() == "1.2.3+foo.bar.42"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo"}}
	    ).to_string() == "1.2.3-foo.bar.42+foo"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo"}, {"bar"}}
	    ).to_string() == "1.2.3-foo.bar.42+foo.bar"
	);

	BOOST_CHECK(
	    Version(
	        1, 2, 3,
	        {{"foo"}, {"bar"}, {42}},
	        {{"foo"}, {"bar"}, {42}}
	    ).to_string() == "1.2.3-foo.bar.42+foo.bar.42"
	);
}

BOOST_AUTO_TEST_CASE(compare_equal)
{
	BOOST_CHECK(Version(1, 2, 3) == Version(1, 2, 3));
	BOOST_CHECK(!(Version(1, 2, 3) == Version(1, 2, 4)));
	BOOST_CHECK(!(Version(1, 2, 3) == Version(1, 3, 3)));
	BOOST_CHECK(!(Version(1, 2, 3) == Version(2, 2, 3)));

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{},
			{{"foo"}}
		) == Version(
			1, 2, 3,
			{},
			{{"bar"}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{},
			{{"foo"}, {42}}
		) == Version(
			1, 2, 3,
			{},
			{{"bar"}, {24}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"foo"}}
		) == Version(
			1, 2, 3,
			{{"foo"}}
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"foo"}}
		) == Version(
			1, 2, 3,
			{{"bar"}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{42}}
		) == Version(
			1, 2, 3,
			{{42}}
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{42}}
		) == Version(
			1, 2, 3,
			{{24}}
		))
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"foo"}}
		) == Version(
			1, 2, 3,
			{{42}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"foo"}, {42}}
		) == Version(
			1, 2, 3,
			{{"foo"}, {42}}
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"foo"}, {42}}
		) == Version(
			1, 2, 3,
			{{"foo"}}
		))
	);
}

BOOST_AUTO_TEST_CASE(compare_not_equal)
{
	BOOST_CHECK(Version(1, 2, 3) != Version(1, 2, 4));
	BOOST_CHECK(Version(1, 2, 3) != Version(1, 3, 3));
	BOOST_CHECK(Version(1, 2, 3) != Version(2, 2, 3));
	BOOST_CHECK(!(Version(1, 2, 3) != Version(1, 2, 3)));

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{},
			{{"foo"}}
		) != Version(
			1, 2, 3,
			{},
			{{"bar"}}
		))
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{},
			{{"foo"}, {42}}
		) != Version(
			1, 2, 3,
			{},
			{{"bar"}, {24}}
		))
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"foo"}}
		) != Version(
			1, 2, 3,
			{{"foo"}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"foo"}}
		) != Version(
			1, 2, 3,
			{{"bar"}}
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{42}}
		) != Version(
			1, 2, 3,
			{{42}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{42}}
		) != Version(
			1, 2, 3,
			{{24}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"foo"}}
		) != Version(
			1, 2, 3,
			{{42}}
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"foo"}, {42}}
		) != Version(
			1, 2, 3,
			{{"foo"}, {42}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"foo"}, {42}}
		) != Version(
			1, 2, 3,
			{{"foo"}}
		)
	);
}


BOOST_AUTO_TEST_CASE(compare_less)
{
	BOOST_CHECK(!(Version(1, 2, 3) < Version(1, 2, 3)));

	BOOST_CHECK(Version(1, 0, 0) < Version(2, 0, 0));
	BOOST_CHECK(Version(2, 0, 0) < Version(2, 1, 0));
	BOOST_CHECK(Version(2, 1, 0) < Version(2, 1, 1));

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"alpha"}}
		) < Version(
			1, 2, 3
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"alpha"}}
		) < Version(
			1, 2, 3,
			{{"alpha"}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"alpha"}}
		) < Version(
			1, 2, 3,
			{{"alpha"}, {1}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"alpha"}, {1}}
		) < Version(
			1, 2, 3,
			{{"alpha"}, {"beta"}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"alpha"}, {"beta"}}
		) < Version(
			1, 2, 3,
			{{"beta"}, {2}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"beta"}, {2}}
		) < Version(
			1, 2, 3,
			{{"beta"}, {11}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"beta"}, {11}}
		) < Version(
			1, 2, 3,
			{{"rc"}, {1}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"rc"}, {1}}
		) < Version(
			1, 2, 3
		)
	);
}

BOOST_AUTO_TEST_CASE(compare_greater)
{
	BOOST_CHECK(!(Version(1, 2, 3) > Version(1, 2, 3)));

	BOOST_CHECK(Version(2, 0, 0) > Version(1, 0, 0));
	BOOST_CHECK(Version(2, 1, 0) > Version(2, 0, 0));
	BOOST_CHECK(Version(2, 1, 1) > Version(2, 1, 0));

	BOOST_CHECK(
		Version(
			1, 2, 3
		) > Version(
			1, 2, 3,
			{{"alpha"}}
		)
	);

	BOOST_CHECK(
		!(Version(
			1, 2, 3,
			{{"alpha"}}
		) > Version(
			1, 2, 3,
			{{"alpha"}}
		))
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"alpha"}, {1}}
		) > Version(
			1, 2, 3,
			{{"alpha"}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"alpha"}, {"beta"}}
		) > Version(
			1, 2, 3,
			{{"alpha"}, {1}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"beta"}, {2}}
		) > Version(
			1, 2, 3,
			{{"alpha"}, {"beta"}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"beta"}, {11}}
		) > Version(
			1, 2, 3,
			{{"beta"}, {2}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3,
			{{"rc"}, {1}}
		) > Version(
			1, 2, 3,
			{{"beta"}, {11}}
		)
	);

	BOOST_CHECK(
		Version(
			1, 2, 3
		) > Version(
			1, 2, 3,
			{{"rc"}, {1}}
		)
	);
}

BOOST_AUTO_TEST_CASE(compare_less_or_equal)
{
	BOOST_CHECK(Version(1, 2, 3) <= Version(1, 2, 3));
	BOOST_CHECK(Version(1, 0, 0) <= Version(2, 0, 0));
	BOOST_CHECK(Version(2, 0, 0) <= Version(2, 1, 0));
	BOOST_CHECK(Version(2, 1, 0) <= Version(2, 1, 1));
}

BOOST_AUTO_TEST_CASE(compare_greater_or_equal)
{
	BOOST_CHECK(Version(1, 2, 3) >= Version(1, 2, 3));

	BOOST_CHECK(Version(2, 0, 0) >= Version(1, 0, 0));
	BOOST_CHECK(Version(2, 1, 0) >= Version(2, 0, 0));
	BOOST_CHECK(Version(2, 1, 1) >= Version(2, 1, 0));
}

BOOST_AUTO_TEST_CASE(comparison)
{
	BOOST_CHECK(check_comparison("0.2.3", "1.2.3", false, {Version::Comparison::PastIncompatible}));
	BOOST_CHECK(check_comparison("2.2.3", "1.2.3", false, {Version::Comparison::FutureIncompatible}));

	BOOST_CHECK(check_comparison("1.2.3", "1.2.3", true, {Version::Comparison::Current}));

	BOOST_CHECK(check_comparison("1.1.3", "1.2.3", true, {Version::Comparison::PastInFunctionalities}));
	BOOST_CHECK(check_comparison("1.3.3", "1.2.3", true, {Version::Comparison::FutureInFunctionalities}));

	BOOST_CHECK(check_comparison("1.2.2", "1.2.3", true, {Version::Comparison::PastInFixes}));
	BOOST_CHECK(check_comparison("1.2.4", "1.2.3", true, {Version::Comparison::FutureInFixes}));

	BOOST_CHECK(Version::Comparison::PastIncompatible < Version::Comparison::PastInFunctionalities);
	BOOST_CHECK(Version::Comparison::PastInFunctionalities < Version::Comparison::PastInFixes);
	BOOST_CHECK(Version::Comparison::PastInFixes < Version::Comparison::Current);

	BOOST_CHECK(Version::Comparison::Current < Version::Comparison::FutureInFixes);
	BOOST_CHECK(Version::Comparison::FutureInFixes < Version::Comparison::FutureInFunctionalities);
	BOOST_CHECK(Version::Comparison::FutureInFunctionalities < Version::Comparison::FutureIncompatible);
}
