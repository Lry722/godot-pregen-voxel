#include <iostream>

#define TEST(x)                                                  \
	std::cout << "Running " #x " test...";                       \
	try {                                                        \
		if (test_##x())                                          \
			std::cout << "Pass " #x " test.";                    \
		else                                                     \
			std::cout << "Failed " #x " test!";                       \
	} catch (const std::exception &e) {                          \
		std::cout << "Error occured in " #x " test: ", e.what(); \
	}

int main() {

}
