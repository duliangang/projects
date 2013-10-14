
#include<iostream>
using namespace std;
class Date;                       //对Date类的提前引用声明
class Time                        //定义Time 类
{ public: 
Time(int,int,int);
friend void display(const Date &,const Time &); //将普通函数display声明为朋友
private:
	int hour;
	int minute;
	int sec;
};



Time::Time(int h,int m,int s)      //类Time的构造函数
{  hour=h;
minute=m;
sec=s;
}

class Date
{public:
Date(int,int,int);
friend void display(const Date &,const Time &);      //将普通函数display声明为朋友
private:
	int month;
	int day;
	int year;
};


Date::Date(int m,int d,int y)                   //类Date的构造函数
{  month=m;
day=d;
year=y;
}

void display(const Date &d,const Time &t)        //是Time和Date两个类的朋友
{
	cout<<d.month<<"/"<<d.day<<"/"<<d.year<<endl;    //引用Date类对象t1中的数据成员
	cout<<t.hour<<":"<<t.minute<<":"<<t.sec<<endl;   //引用Time类对象t1中的数据成员
}

int main()
{  
	Time tl(10,13,56);   //定义Time类对象tl
	Date dl(12,25,2004); //定义Date类对象dl

	//error in there
//	display(dl,t1);     // 调用display函数，用对象名做实参
	//change to
	display(dl,tl);
	return 0;
}
