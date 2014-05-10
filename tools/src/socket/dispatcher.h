//#include "file.pb.h"
//
//#include <boost/function.hpp>
//#include <boost/noncopyable.hpp>
//#include <boost/shared_ptr.hpp>
//
//#include <iostream>
//
//using namespace std;
//
//class Callback : boost::noncopyable
//{
//public:
//	virtual ~Callback() {};
//	virtual void onMessage(boost::shared_ptr<google::protobuf::Message> message,void*) const = 0;
//};
//
//
//template <typename T>
//class CallbackT : public Callback
//{
//public:
//	typedef boost::function<void (boost::shared_ptr<T> message,void*)> ProtobufMessageCallback;
//
//	CallbackT(const ProtobufMessageCallback& callback)
//		: callback_(callback)
//	{
//	}
//
//	virtual void onMessage(boost::shared_ptr<google::protobuf::Message> message,void*m_data) const
//	{
//		boost::shared_ptr<T> t = boost::dynamic_pointer_cast<boost::shared_ptr<T> >(message);
//		assert(t != NULL);
//		callback_(t,m_data);
//	}
//
//private:
//	ProtobufMessageCallback callback_;
//};
//
//void discardProtobufMessage(boost::shared_ptr<google::protobuf::Message> message,void *)
//{
//	cout << "Discarding " << message->GetTypeName() << endl;
//}
//
//class ProtobufDispatcher
//{
//public:
//
//	ProtobufDispatcher()
//		: defaultCallback_(discardProtobufMessage)
//	{
//	}
//
//	void onMessage(boost::shared_ptr<google::protobuf::Message> message) const
//	{
//		CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
//		if (it != callbacks_.end())
//		{
//			it->second->onMessage(message,m_data);
//		}
//		else
//		{
//			defaultCallback_(message,NULL);
//		}
//	}
//
//	template<typename T>
//	void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageCallback& callback,void* data)
//	{
//		boost::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
//		callbacks_[T::descriptor()] = pd;
//		m_data=data;
//	}
//	
//	typedef std::map<const google::protobuf::Descriptor*, boost::shared_ptr<Callback> > CallbackMap;
//	CallbackMap callbacks_;
//	void* m_data;
//	boost::function<void (boost::shared_ptr<google::protobuf::Message> message ,void* data)> defaultCallback_;
//};
