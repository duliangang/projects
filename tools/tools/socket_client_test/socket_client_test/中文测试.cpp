#include <iostream>

class ���Ĵ���
{
public:
	���Ĵ���(){}
	void ע��(){std::cout<<"ע��"<<std::endl;}
	void ���Ĳ��Ժ���(){std::cout<<"���Ĳ��Ժ���"<<std::endl;}
	virtual void ���Ĳ����麯��(){std::cout<<"���Ĳ����麯��"<<std::endl;}
};
class ���Դ���:public ���Ĵ���
{
public:
	���Դ���(){}
	virtual void ���Ĳ����麯��(){std::cout<<"���Դ����麯��"<<std::endl;}
};
int main()
{
	���Դ��� ����1;
	����1
}