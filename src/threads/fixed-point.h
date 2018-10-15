#include <stdint.h>

#define f (2 << 14)

int convert_to_fixed_point(int x);
int convert_to_int_floor(int x);
int convert_to_int_round(int x);
int add_ints(int x, int y);
int add_fixed_points(int x, int y);
int subtract_ints(int x, int y);
int subtract_fixed_points(int x, int y);
int multiply_ints(int x, int y);
int multiply_fixed_points(int x, int y);
int divide_ints(int x, int y);
int divide_fixed_points(int x, int y);

int convert_to_fixed_point(int x){
	return x * f;
}

int convert_to_int_floor(int x){
	return x / f;
}

int convert_to_int_round(int x){
	if (x >= 0)
		return (x + f / 2) / f;
	
	return (x - f / 2) / f;
}

int add_ints(int x, int y){
	return x + (y * f);
}

int add_fixed_points(int x, int y){
	return x + y;
}

int subtract_ints(int x, int y){
	return x - (y * f);
}

int subtract_fixed_points(int x, int y){
	return x - y;
}

int multiply_ints(int x, int y){
	return x * y;
}

int multiply_fixed_points(int x, int y){
	return ((int64_t)x) * y / f;
}

int divide_ints(int x, int y){
	return x / y;
}

int divide_fixed_points(int x, int y){
	return ((int64_t)x) * f / y;
}