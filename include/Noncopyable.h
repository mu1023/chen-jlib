#ifndef _NONCOPYABLE_H_
#define _NONCOPYABLE_H_
namespace cj
{
	class Noncopyable
	{
	protected:
		Noncopyable()
		{
		}
		~Noncopyable()
		{
		}
	public:
		Noncopyable(Noncopyable&) = delete;
		Noncopyable& operator =(Noncopyable&) = delete;
	};
}
#endif