#include "utilities.h"
#include <assimp/scene.h>

std::string read_string(const std::vector<uint8_t> &file_content, unsigned int &addr) {
	std::string result = std::string((char *)(file_content.data() + addr));
	addr += result.length()+1;
	return result;
}



aiMatrix4x4 create_transform_from_trs(aiQuaternion R, aiVector3D T, aiVector3D S) {

	aiMatrix4x4 transfo;
	//first is rotation matrix:
	float fVar7 = 0.0;
	float fVar1 = R.y;
	float fVar2 = R.z;
	float fVar3 = R.x;
	float fVar4 = R.w;
	float fVar5 = fVar4 * fVar4 + fVar2 * fVar2 + fVar3 * fVar3 + fVar1 * fVar1;
	if (0.0 < fVar5) {
		fVar7 = 2.0 / fVar5;
	}
	transfo.d4 = 1.0;
	float fVar6 = fVar2 * fVar7;
	fVar5 = fVar1 * fVar7;
	transfo.c4 = 0;
	transfo.b4 = 0.0;
	transfo.a4 = 0.0;
	transfo.d2 = 0;
	float fVar8 = fVar3 * fVar3 * fVar7;
	fVar7 = fVar4 * fVar3 * fVar7;
	transfo.a1 = (1.0 - (fVar2 * fVar6 + fVar1 * fVar5)) * S.x;
	transfo.b1 = (fVar4 * fVar6 + fVar3 * fVar5) * S.y;
	transfo.a2 = (fVar3 * fVar5 - fVar4 * fVar6) * S.x;
	transfo.c1 = (fVar3 * fVar6 - fVar4 * fVar5) * S.z;
	transfo.a3 = (fVar4 * fVar5 + fVar3 * fVar6) * S.x;
	transfo.b3 = (fVar1 * fVar6 - fVar7) * S.y;
	transfo.b2 = (1.0 - (fVar2 * fVar6 + fVar8)) * S.y;
	transfo.c2 = (fVar7 + fVar1 * fVar6) * S.z;
	transfo.c3 = (1.0 - (fVar1 * fVar5 + fVar8)) * S.z;
	transfo.d1 = T.x;
	transfo.d2 = T.y;
	transfo.d3 = T.z;

	transfo.Transpose();
	return transfo;

}



void matrix4::decompose(vector3<float>& T, vector4<float>& R, vector3<float>& S) {
	T.x = a.t;
	T.y = b.t;
	T.z = c.t;
	S.x = sqrt(a.x * a.x + b.x * b.x + c.x * c.x);
	S.y = sqrt(a.y * a.y + b.y * b.y + c.y * c.y);
	S.z = sqrt(a.z * a.z + b.z * b.z + c.z * c.z);

	float m00 = a.x / S.x;
	float m01 = a.y / S.x;
	float m02 = a.z / S.x;
	float m03 = 0;
	float m10 = b.x / S.y;
	float m11 = b.y / S.y;
	float m12 = b.z / S.y;
	float m13 = 0;
	float m20 = c.x / S.z;
	float m21 = c.y / S.z;
	float m22 = c.z / S.z;
	float m23 = 0;
	float m30 = 0;
	float m31 = 0;
	float m32 = 0;
	float m33 = 1;

	float tr = m00 + m11 + m22;
	float qw, qx, qy, qz;
	if (tr > 0) {
		float S = sqrt(tr + 1.0) * 2; // S=4*qw 
		qw = 0.25 * S;
		qx = (m21 - m12) / S;
		qy = (m02 - m20) / S;
		qz = (m10 - m01) / S;
	}
	else if ((m00 > m11)& (m00 > m22)) {
		float S = sqrt(1.0 + m00 - m11 - m22) * 2; // S=4*qx 
		qw = (m21 - m12) / S;
		qx = 0.25 * S;
		qy = (m01 + m10) / S;
		qz = (m02 + m20) / S;
	}
	else if (m11 > m22) {
		float S = sqrt(1.0 + m11 - m00 - m22) * 2; // S=4*qy
		qw = (m02 - m20) / S;
		qx = (m01 + m10) / S;
		qy = 0.25 * S;
		qz = (m12 + m21) / S;
	}
	else {
		float S = sqrt(1.0 + m22 - m00 - m11) * 2; // S=4*qz
		qw = (m10 - m01) / S;
		qx = (m02 + m20) / S;
		qy = (m12 + m21) / S;
		qz = 0.25 * S;
	}
}

std::string id_to_ascii(unsigned int identifier_uint) {
	char identifier[5] = { 0 };
	memcpy(identifier, &identifier_uint, 4);

	return std::string(identifier);
}
std::vector<uint8_t> int_to_bytes(int x) {
	std::vector<uint8_t> result = std::vector<uint8_t>(4);
	result[0] = (x & 0x000000FF);
	result[1] = (x & 0x0000FF00) >> 8;
	result[2] = (x & 0x00FF0000) >> 16;
	result[3] = (x & 0xFF000000) >> 24;
	return result;
}