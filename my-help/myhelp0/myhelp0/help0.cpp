
#include<iostream>
using namespace std;
class Date;                       //��Date�����ǰ��������
class Time                        //����Time ��
{ public: 
Time(int,int,int);
friend void display(const Date &,const Time &); //����ͨ����display����Ϊ����
private:
	int hour;
	int minute;
	int sec;
};



Time::Time(int h,int m,int s)      //��Time�Ĺ��캯��
{  hour=h;
minute=m;
sec=s;
}

class Date
{public:
Date(int,int,int);
friend void display(const Date &,const Time &);      //����ͨ����display����Ϊ����
private:
	int month;
	int day;
	int year;
};


Date::Date(int m,int d,int y)                   //��Date�Ĺ��캯��
{  month=m;
day=d;
year=y;
}

void display(const Date &d,const Time &t)        //��Time��Date�����������
{
	cout<<d.month<<"/"<<d.day<<"/"<<d.year<<endl;    //����Date�����t1�е����ݳ�Ա
	cout<<t.hour<<":"<<t.minute<<":"<<t.sec<<endl;   //����Time�����t1�е����ݳ�Ա
}

int main()
{  
	Time tl(10,13,56);   //����Time�����tl
	Date dl(12,25,2004); //����Date�����dl

	//error in there
//	display(dl,t1);     // ����display�������ö�������ʵ��
	//change to
	display(dl,tl);
	return 0;
}
