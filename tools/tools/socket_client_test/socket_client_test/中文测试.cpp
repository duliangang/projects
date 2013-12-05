#include <iostream>

class 中文代码
{
public:
	中文代码(){}
	void 注释(){std::cout<<"注释"<<std::endl;}
	void 中文测试函数(){std::cout<<"中文测试函数"<<std::endl;}
	virtual void 中文测试虚函数(){std::cout<<"中文测试虚函数"<<std::endl;}
};
class 测试代码:public 中文代码
{
public:
	测试代码(){}
	virtual void 中文测试虚函数(){std::cout<<"测试代码虚函数"<<std::endl;}
};
int main()
{
	测试代码 代码1;
	代码1
}