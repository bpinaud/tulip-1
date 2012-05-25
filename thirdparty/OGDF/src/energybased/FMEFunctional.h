#ifndef _FME_FUNCTIONAL_H_
#define _FME_FUNCTIONAL_H_

//! the useless do nothing function
struct do_nothing 
{ 
	template<typename A> inline void operator()(A a) { }; 
	template<typename A, typename B> inline void operator()(A a, B b) { }; 
};

//! condition functor for returning a constant boolean value
template<bool result>
struct const_condition
{
	template<typename A> inline bool operator()(A a) { return result; }; 
};

//! the corresponding typedefs
typedef const_condition<true> true_condition;
typedef const_condition<false> false_condition;

//! functor for negating a condition
template<typename Func>
struct not_condition_functor
{
	Func cond_func;
	
	not_condition_functor(const Func& cond) : cond_func(cond) {};

	template<typename A> inline bool operator()(A a) { return !cond_func(a); }; 
};

//! creator of the negator
template<typename Func>
static inline not_condition_functor<Func> not_condition(const Func& func)
{
	return not_condition_functor<Func>(func);
};

//! Functor for conditional usage of a functor
template<typename CondType, typename ThenType, typename ElseType = do_nothing>
struct if_then_else_functor
{
	CondType condFunc;
	ThenType thenFunc;
	ElseType elseFunc;
	
	if_then_else_functor(const CondType& c, const ThenType& f1) : condFunc(c), thenFunc(f1) {};

	if_then_else_functor(const CondType& c, const ThenType& f1, const ElseType& f2) : condFunc(c), thenFunc(f1), elseFunc(f2) {};

	template<typename A>
	inline void operator()(A a) 
	{ 
		if (condFunc(a)) 
			thenFunc(a); 
		else 
			elseFunc(a); 
	};

	template<typename A, typename B>
	inline void operator()(A a, B b)
	{
		if (condFunc(a, b)) 
			thenFunc(a, b); 
		else 
			elseFunc(a, b);
	};
};

//! creates an if then else functor with a condition and a then and an else functor
template<typename CondType, typename ThenType, typename ElseType>
static inline if_then_else_functor<CondType, ThenType, ElseType> if_then_else(const CondType& cond, const ThenType& thenFunc, const ElseType& elseFunc)
{
	return if_then_else_functor<CondType, ThenType, ElseType>(cond, thenFunc, elseFunc);
};

//! creates an if then functor with a condition and a then functor
template<typename CondType, typename ThenType>
static inline if_then_else_functor<CondType, ThenType> if_then(const CondType& cond, const ThenType& thenFunc)
{
	return if_then_else_functor<CondType, ThenType>(cond, thenFunc);
};

//! helper functor to generate a pair as parameters
template<typename F, typename A>
struct pair_call_functor
{
	F func;
	A first;
	pair_call_functor(F f, A a) : func(f), first(a) {};

	template<typename B>
	inline void operator()(B second)
	{
		func(first, second);
	};
};

//! creates a pair call resulting in a call f(a, *)
template<typename F, typename A>
static inline pair_call_functor<F, A> pair_call(F f, A a)
{
	return pair_call_functor<F, A>(f, a);
};

//! Functor for composing two other functors
template<typename FuncFirst, typename FuncSecond>
struct composition_functor
{
	FuncFirst firstFunc;
	FuncSecond secondFunc;

	composition_functor(const FuncFirst& first, const FuncSecond& second) : firstFunc(first), secondFunc(second) {}; 
	template<typename A>
	void operator()(A a)
	{
		firstFunc(a);
		secondFunc(a);
	};

	template<typename A, typename B>
	void operator()(A a, B b)
	{
		firstFunc(a, b);
		secondFunc(a, b);
	};
};

//! create a functor composition of two functors
template<typename FuncFirst, typename FuncSecond>
static inline composition_functor<FuncFirst, FuncSecond> func_comp(const FuncFirst& first, const FuncSecond& second)
{
	return composition_functor<FuncFirst, FuncSecond>(first, second);
};

//! functor for invoking a functor for a pair(u,v) and then (v,u)
template<typename Func>
struct pair_vice_versa_functor
{
	Func func;

	pair_vice_versa_functor(const Func& f) : func(f) {};

	template<typename A, typename B>
	void operator()(A a, B b)
	{
		func(a, b);
		func(b, a);
	};
};

//! creates a functor for invoking a functor for a pair(u,v) and then (v,u)
template<typename Func>
static inline pair_vice_versa_functor<Func> pair_vice_versa(const Func& f)
{
	return pair_vice_versa_functor<Func>(f);
};

//! generic min max functor for an array
template<typename T>
struct min_max_functor
{
	const T* a;
	T& min_value;
	T& max_value;

	min_max_functor(const T* ptr, T& min_var, T& max_var) : a(ptr), min_value(min_var), max_value(max_var) 
	{
		min_value = a[0];
		max_value = a[0];
	};

	inline void operator()(__uint32 i)
	{
		min_value = min<T>(min_value, a[i]);
		max_value = max<T>(max_value, a[i]);
	};
};


#endif
