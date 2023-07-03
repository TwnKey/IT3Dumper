#pragma once
#include <map>
#include <string>
#include "utilities.h"
#include "IT3File.h"
#include "MTBFile.h"
#include <map>
#include <assimp/scene.h>

class node {
public:
	node() = default;
	std::string name;
	aiMatrix4x4 transform;
	aiMatrix4x4 bind_mat;
	std::vector<std::string> children;
	std::string parent = "";
	unsigned int id;
	unsigned int depth = -1;
	node(std::string name, aiMatrix4x4 mat) :name(name), transform(mat) {}
};



struct material {
	std::string name;
	std::vector<std::string> textures_paths;
};
struct bone : node { //keep in mind a bone can be shared between different meshes
	std::string name;
	aiMatrix4x4 offset_matrix;
	std::vector<float> weights;
	std::vector<unsigned int> idx_v;

	bone() = default;

	bone(std::string name, aiMatrix4x4 mat) :node(name, mat) {}
};
struct mesh : node {
	std::map<std::string, bone> bones;
	std::vector<vector3<float>> vertices;
	std::vector<vector2<float>> uv;
	std::vector<vector2<float>> uv_n;
	std::vector<vector3<unsigned int>> faces_indexes;
	unsigned int mat_id;
	unsigned int mat_variant;
	mesh() = default;
	mesh(std::string name, aiMatrix4x4 mat) :node(name, mat) {}

};




struct keys {

	std::vector<key_animation> positions;
	std::vector<key_animation> rotations;
	std::vector<key_animation> scales;

};
struct animation {
	std::string name;
	unsigned int start;
	unsigned int duration;
	unsigned int end;
	unsigned int ticks_per_second;
	std::map<std::string, keys> bones_data;
	animation() = default;
	animation(std::string name, unsigned int duration, unsigned int ticks_per_second, unsigned int end) : name(name), duration(duration), end(end), ticks_per_second(ticks_per_second){}
};


#include <tsl/ordered_map.h>

class Scene
{
public:
	Scene(std::string name, IT3File it3_p, IT3File it3_m, MTBFile mtb);
	~Scene() = default;
	std::string name;
	std::map<std::string, node> nodes; 
	std::map<std::string, std::vector<mesh>> meshes; //node name
	std::map<std::string, animation> anis; //animation name
	std::map<unsigned int, material> mats;
	std::map<std::string, std::vector<bone *>> bones;
	std::map<std::string, aiMatrix4x4> ibms;
};

