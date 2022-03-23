#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include<CommonDefine.h>
#include<Noncopyable.h>
template<class T>
class Singleton {
public:
	

	static T* Instance() {
		if (m_Val == NULL) {
			m_Val = new T();
		}
		return m_Val;
	}
protected:
	Singleton() {};
	~Singleton() {};
private:
	
	static T* Destory() {
		if (m_Val != NULL) {
			delete m_Val;
		}
		m_Val = NULL;
	};
	template<class T>
	class DestoryClass {
		~DestoryClass() {
			Singleton<T>::Destory();
		}
	};
	static T* m_Val;
	static DestoryClass<T> m_Del;
};
template < class T>
T*  Singleton<T>::m_Val = NULL;

template < class T>
Singleton<T>::DestoryClass<T> Singleton<T>::m_Del ;
#endif