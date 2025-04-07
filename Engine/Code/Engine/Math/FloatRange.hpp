#pragma once
struct FloatRange {
	/*a.A new math class / struct, FloatRange, defined in Engine / Math / FloatRange.cpp, hpp
		b.Member variables : float m_min, float m_max
		c.Default construction to the range[0, 0]
		d.Explicit construction to a range[min, max]
		e.Basic methods, including operator =, == , != , IsOnRange( float ), IsOverlappingWith( FloatRange )
		f.Named static const objects : ZERO[0, 0], ONE[1, 1], ZERO_TO_ONE[0, 1]*/
public:
	FloatRange();
	explicit FloatRange( float min, float max );
	float m_min = 0.f;
	float m_max = 0.f;

	bool SetFromText( char const* text );

	void operator=( FloatRange const& copyFrom );
	bool operator==( FloatRange const& compare ) const;
	bool operator!=( FloatRange const& compare ) const;
	bool IsOnRange( float num ) const;
	bool IsOverlappingWith( FloatRange const& comparedFloatRange );

	static FloatRange const ZERO;
	static FloatRange const ONE;
	static FloatRange const ZERO_TO_ONE;
};