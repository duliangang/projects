#include <boost/shared_ptr.hpp>
#include "auto_ptr.h"
#include <iostream>
#include <vector>

//循环引用问题
class B; 
class A 
{ 
public: 
	A(){}
	~A(){std::cout<<"xigou a"<<std::endl;}
	auto_ptr<B> m_b; 
}; 

class B 
{ 
public: 
	B(){}
	~B(){std::cout<<"xigou b"<<std::endl;}
	auto_ptr<A> m_a; 
}; 


void  test_copy(auto_ptr<A>  test_copy)
{
	auto_ptr<A> test_1(test_copy);
}
void test_toarray(std::vector< auto_ptr<A> >& test_array)
{
	auto_ptr<A> test1(new A);
	test_array.push_back(test1);
}

int main()
{
	auto_ptr<A> a(new A());
	auto_ptr<B> b(new B());
	test_copy(a);
	std::vector<auto_ptr<A> > ary_test;
	test_toarray(ary_test);
	//循环引用问题
	a->m_b=b;
	b->m_a=a;
	return 0;
}