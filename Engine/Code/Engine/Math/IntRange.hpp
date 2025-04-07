#pragma once
struct IntRange {
public:
	IntRange();
	explicit IntRange( int min, int max );
	int m_min = 0;
	int m_max = 0;

	void operator=( IntRange const& copyFrom );
	bool operator==( IntRange const& compare ) const;
	bool operator!=( IntRange const& compare ) const;
	bool IsOnRange( float num ) const;
	bool IsOverlappingWith( IntRange const& comparedIntRange );

};