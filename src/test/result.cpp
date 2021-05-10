#include <stdio.h>

#include <utility>

#include <data/result.h>
using namespace daytrender;

class Type
{
	static int count;
	int id = 0;
	char *_buffer = nullptr;
public:
	Type()
	{
		id = ++count;
		printf("\tConstructing %d\n", id);
		_buffer = new char[32];
	}

	// Copy generates new id
	Type(const Type& other)
	{
		*this = other;
		printf("\tCopying from %d to %d\n", other.id, id);
	}

	// Move does not generate new id
	Type(Type&& other)
	{
		printf("\tMoving from %d\n", other.id);
		id = other.id;
		_buffer = other._buffer;

		other.id = 0;
		other._buffer = nullptr;
	}

	~Type()
	{
		printf("\tDestructing %d\n", id);
		delete[] _buffer;
	}

	Type& operator=(const Type& other)
	{
		id = ++count;
		_buffer = new char[32];
		// copy over new data
		for (int i = 0; i < 32; ++i) _buffer[i] = other.buffer()[i];
		return *this;
	}

	const char *buffer() const { return _buffer; }
};

int Type::count = 0;

Result<Type> get_type()
{
	return Type();
}

int main(void)
{
	puts("Creating t1");
	Type t1;
	puts("Copying to t2");
	Type t2 = t1;
	puts("Creating result");
	Result<Type> res = get_type();
	puts("Moving result to t3");
	Type t3 = res.get();

	return 0;
}