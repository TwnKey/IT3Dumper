#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <math.h>
#include <assimp/scene.h>
template<typename T>
struct vector4 {
	T x;
	T y;
	T z; 
	T t;
};
template<typename T>
struct vector3 {
	T x;
	T y;
	T z;
};

template<typename T>
struct vector2 {
	T x;
	T y;
};
aiMatrix4x4 create_transform_from_trs(aiQuaternion R, aiVector3D T, aiVector3D S);

struct matrix4 {
	vector4<float> a, b, c, d;

	matrix4 multiply(matrix4 m) {
		float ax = this->a.x * m.a.x + this->a.y * m.b.x + this->a.z * m.c.x + this->a.t * m.d.x;
		float ay = this->a.x * m.a.y + this->a.y * m.b.y + this->a.z * m.c.y + this->a.t * m.d.y;
		float az = this->a.x * m.a.z + this->a.y * m.b.z + this->a.z * m.c.z + this->a.t * m.d.z;
		float at = this->a.x * m.a.t + this->a.y * m.b.t + this->a.z * m.c.t + this->a.t * m.d.t;

		float bx = this->b.x * m.a.x + this->b.y * m.b.x + this->b.z * m.c.x + this->b.t * m.d.x;
		float by = this->b.x * m.a.y + this->b.y * m.b.y + this->b.z * m.c.y + this->b.t * m.d.y;
		float bz = this->b.x * m.a.z + this->b.y * m.b.z + this->b.z * m.c.z + this->b.t * m.d.z;
		float bt = this->b.x * m.a.t + this->b.y * m.b.t + this->b.z * m.c.t + this->b.t * m.d.t;

		float cx = this->c.x * m.a.x + this->c.y * m.b.x + this->c.z * m.c.x + this->c.t * m.d.x;
		float cy = this->c.x * m.a.y + this->c.y * m.b.y + this->c.z * m.c.y + this->c.t * m.d.y;
		float cz = this->c.x * m.a.z + this->c.y * m.b.z + this->c.z * m.c.z + this->c.t * m.d.z;
		float ct = this->c.x * m.a.t + this->c.y * m.b.t + this->c.z * m.c.t + this->c.t * m.d.t;

		float dx = this->d.x * m.a.x + this->d.y * m.b.x + this->d.z * m.c.x + this->d.t * m.d.x;
		float dy = this->d.x * m.a.y + this->d.y * m.b.y + this->d.z * m.c.y + this->d.t * m.d.y;
		float dz = this->d.x * m.a.z + this->d.y * m.b.z + this->d.z * m.c.z + this->d.t * m.d.z;
		float dt = this->d.x * m.a.t + this->d.y * m.b.t + this->d.z * m.c.t + this->d.t * m.d.t;



			
		matrix4 result = { ax, ay, az, at,
			bx, by, bz, bt,
			cx, cy, cz, ct, 
			dx, dy, dz, dt
		};
		return result;
	}
	void transpose() {
		float ax = this->a.x;
		float ay = this->b.x;
		float az = this->c.x;
		float at = this->d.x;

		float bx = this->a.y;
		float by = this->b.y;
		float bz = this->c.y;
		float bt = this->d.y;

		float cx = this->a.z;
		float cy = this->b.z;
		float cz = this->c.z;
		float ct = this->d.z;

		float dx = this->a.t;
		float dy = this->b.t;
		float dz = this->c.t;
		float dt = this->d.t;




		this->a = { ax, ay, az, at };
		this->b = { bx, by, bz, bt };
		this->c = { cx, cy, cz, ct };
		this->d = { dx, dy, dz, dt };
	
	
	}
	void decompose(vector3<float>& T, vector4<float>& R, vector3<float>& S);
	std::string to_string() {
		std::stringstream ss;
		ss << std::setw(5) << this->a.x << " " << this->a.y << " " << this->a.z << " " << this->a.t << std::endl;
		ss << std::setw(5) << this->b.x << " " << this->b.y << " " << this->b.z << " " << this->b.t << std::endl;
		ss << std::setw(5) << this->c.x << " " << this->c.y << " " << this->c.z << " " << this->c.t << std::endl;
		ss << std::setw(5) << this->d.x << " " << this->d.y << " " << this->d.z << " " << this->d.t << std::endl;
		
		return ss.str();
	}
};
template<typename T>
T read_data(const std::vector<uint8_t> &file_content, unsigned int &addr) {
	T result;
	std::memcpy(&result, file_content.data() + addr, sizeof(T));
	addr += sizeof(T);
	return result;
}
std::string read_string(const std::vector<uint8_t> &file_content, unsigned int &addr);
std::string id_to_ascii(unsigned int identifier_uint);
std::vector<uint8_t> int_to_bytes(int x);