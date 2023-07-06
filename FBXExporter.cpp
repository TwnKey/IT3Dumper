#include "FBXExporter.h"
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\Exporter.hpp>
#include <iostream>
#include <unordered_map>
#include "Scene.h"





void assign_property(aiMaterial& mat_to_update, material mat, unsigned int index_tex, aiTextureType type) { //aiTextureType_DIFFUSE
	if (index_tex < mat.textures_paths.size()) {
		if (mat.textures_paths[index_tex].compare("") != 0) {
			aiString name;
			name.Set(mat.textures_paths[index_tex]);
			size_t nb_tex = mat_to_update.GetTextureCount(type);
			mat_to_update.AddProperty(&name, AI_MATKEY_TEXTURE(type, nb_tex));
		}
		//there was no texture with that index, so we don't do anything
	}
	//there was no texture with that index, so we don't do anything

}

void assign_property_based_on_name(aiMaterial& mat_to_update, material mat) {

	

	for (unsigned int idx_tex = 0; idx_tex < mat.textures_paths.size(); idx_tex++) {
		//we guess from the texture name...
		aiString name;
		name.Set(mat.textures_paths[idx_tex]);


		//size_t idx = mat.textures_paths[idx_tex].find_last_of("_");
		size_t idx = mat.textures_paths[idx_tex].find_last_of("0123456789");
		if (idx != std::string::npos) {
			std::string suffix = mat.textures_paths[idx_tex].substr(idx + 1, 1);

			aiTextureType type;
			int channel;
			if (suffix.compare("n") == 0) {
				channel = 1;
				type = aiTextureType_NORMALS;
			}
				
			else if (suffix.compare("s") == 0) {
				channel = 0;
				type = aiTextureType_SPECULAR;
			}
				
			else
			{
				channel = 0;
				type = aiTextureType_DIFFUSE;
			}
				

			size_t nb_tex = mat_to_update.GetTextureCount(type);
			mat_to_update.AddProperty(&name, AI_MATKEY_TEXTURE(type, nb_tex));
			aiTextureMapping mapping = aiTextureMapping_UV;
			mat_to_update.AddProperty<int>((int *)&mapping,1, AI_MATKEY_MAPPING(type, nb_tex));

			mat_to_update.AddProperty<int>((int*)&channel, 1, AI_MATKEY_UVWSRC(type, nb_tex));
			

		}


	}
}

aiMaterial* guess_material_from_mat_struct(material mat, unsigned int variant) {
	//0638870 <= I hate this

	/*we will miss a LOT of parameters, because I'm certainly not going to reverse every layout of material parameters
	(afaik their parsing is hardcoded and very specific); it will also not be perfect and more of a guess than anything*/

	aiMaterial* material = new aiMaterial();

	aiString name;
	name.Set(mat.name);
	material->AddProperty(&name, AI_MATKEY_NAME);

	switch (variant) {
	case 0:
		break;
	case 8: //lambert, osef
		break;
	/*case 15:

		break;
	case 17:
		break;
	case 32:
		break;
	case 110:
		break;
	case 115:
		break;
	case 172:
		break;*/

	/*actually you know what, fuck it*/
	default: 
		assign_property_based_on_name(*material, mat);
		break;
	}
	return material;
}


aiQuaternion interpolate_rot(std::vector<key_animation> keys, unsigned int tick) {
	aiQuaternion result = aiQuaternion(1, 0, 0, 0);
	if (keys.size() == 0)
		return result;
	else {
		unsigned int idx = 0, before, after = -1;
		while (keys[idx++].tick > tick) {
			if (idx == keys.size()) break;
		}
		if (idx == keys.size()) {
			before = keys.size() - 1;
			after = -1;
		}
		else {
			before = idx - 1;
			after = idx;

		}
		if (after == -1) {
			return aiQuaternion(keys[before].data.t, keys[before].data.x, keys[before].data.y, keys[before].data.z);

		}
		else {
			if (before == -1) {
				return aiQuaternion(keys[after].data.t, keys[after].data.x, keys[after].data.y, keys[after].data.z);

			}
			else {
				//both are present, we interpolate
				key_animation b = keys[before];
				key_animation a = keys[after];
				float t = ((float)(tick - b.tick)) / (a.tick - b.tick);

				result.x = (1 - t) * b.data.x + t * a.data.x;
				result.y = (1 - t) * b.data.y + t * a.data.y;
				result.z = (1 - t) * b.data.z + t * a.data.z;
				result.w = (1 - t) * b.data.t + t * a.data.t;
				return result;
			}

		}
	}
}

aiVector3D interpolate_vec(std::vector<key_animation> keys, unsigned int tick) {
	aiVector3D result = aiVector3D(0, 0, 0);
	if (keys.size() == 0)
		return result;
	else {
		unsigned int idx = 0, before, after = -1;
		while (keys[idx++].tick > tick) {
			if (idx == keys.size()) break;
		}
		if (idx == keys.size()) {
			before = keys.size() - 1;
			after = -1;
		}
		else {
			before = idx - 1;
			after = idx;

		}
		if (after == -1) {
			return aiVector3D(keys[before].data.x, keys[before].data.y, keys[before].data.z);

		}
		else {
			if (before == -1) {
				return aiVector3D(keys[after].data.x, keys[after].data.y, keys[after].data.z);

			}
			else {
				//both are present, we interpolate
				key_animation b = keys[before];
				key_animation a = keys[after];
			
				float t = ((float)(tick - b.tick)) / (a.tick - b.tick);

				result.x = (1 - t) * b.data.x + t * a.data.x;
				result.y = (1 - t) * b.data.y + t * a.data.y;
				result.z = (1 - t) * b.data.z + t * a.data.z;
			
				return result;
			}

		}
	}
}

void FBXExporter::ExportScene(Scene scene){
	

	size_t nb_meshes_nodes = scene.meshes.size();
	size_t nb_nodes = scene.nodes.size();
	size_t nb_materials = scene.mats.size();
	size_t nb_ani = scene.anis.size();
	//size_t nb_nodes = nb_meshes_nodes + nb_simple_nodes;
	
	size_t real_nb_of_meshes = 0;
	for (auto mesh : scene.meshes)
		real_nb_of_meshes += mesh.second.size();


	aiNode** aiNodes = new aiNode* [nb_nodes];
	aiMesh** aimeshes = new aiMesh * [real_nb_of_meshes];
	aiMaterial** aimaterials = new aiMaterial * [nb_materials];


	std::map<std::string, aiNode*> map_ptr;
	std::map<std::string, std::vector<aiBone*>> bones_ptr;
	std::vector <std::vector<std::string>> children;

	std::unordered_map<unsigned int, unsigned int> mat_id_to_idx;
	unsigned int count_total_mesh = 0, count_total_material = 0;
	/*first we create all aiNodes separately, then we will organize them.
	We start from the simple nodes*/
	unsigned int idx_node = 0;

	for (auto it_nd : scene.nodes) {

		aiNodes[idx_node] = new aiNode();
		aiNodes[idx_node]->mName = it_nd.second.name;
		map_ptr[aiNodes[idx_node]->mName.C_Str()] = aiNodes[idx_node];
		aiMatrix4x4 transform_node = it_nd.second.transform;

		aiNodes[idx_node]->mTransformation = transform_node; 
		children.push_back(it_nd.second.children);
		size_t nb_children = it_nd.second.children.size();
		aiNodes[idx_node]->mNumChildren = nb_children;
		aiNodes[idx_node]->mChildren = new aiNode * [nb_children]();

		idx_node++;

	}
	
	unsigned int count_materials = 0;
	unsigned int count_mesh = 0;
	for (auto it_msh : scene.meshes) {
		
	
		map_ptr[it_msh.first]->mMeshes = new unsigned int[it_msh.second.size()];
		map_ptr[it_msh.first]->mNumMeshes = it_msh.second.size();
		unsigned int count_mesh_within_node = 0;
		for (auto mesh_ : it_msh.second) {
			

			aimeshes[count_mesh] = new aiMesh();

			aimeshes[count_mesh]->mVertices = new aiVector3D[mesh_.vertices.size()];
			aimeshes[count_mesh]->mNumUVComponents[0] = 2;
			aimeshes[count_mesh]->mNumUVComponents[1] = 2;
			 
			aimeshes[count_mesh]->mTextureCoords[0] = new aiVector3D[mesh_.vertices.size()];
			aimeshes[count_mesh]->mTextureCoords[1] = new aiVector3D[mesh_.vertices.size()];

			for (unsigned int idx_v = 0; idx_v < mesh_.vertices.size(); idx_v++)
			{
				aimeshes[count_mesh]->mVertices[idx_v].x = mesh_.vertices[idx_v].x;
				aimeshes[count_mesh]->mVertices[idx_v].y = mesh_.vertices[idx_v].y;
				aimeshes[count_mesh]->mVertices[idx_v].z = mesh_.vertices[idx_v].z;

				aimeshes[count_mesh]->mTextureCoords[0][idx_v].x = mesh_.uv[idx_v].x;
				aimeshes[count_mesh]->mTextureCoords[0][idx_v].y = mesh_.uv[idx_v].y;
				aimeshes[count_mesh]->mTextureCoords[0][idx_v].z = 0;

				aimeshes[count_mesh]->mTextureCoords[1][idx_v].x = mesh_.uv_n[idx_v].x;
				aimeshes[count_mesh]->mTextureCoords[1][idx_v].y = mesh_.uv_n[idx_v].y;
				aimeshes[count_mesh]->mTextureCoords[1][idx_v].z = 0;
			}

			size_t nb_faces = mesh_.faces_indexes.size(); 
			aiFace* faces = new aiFace[nb_faces];

			for (unsigned int idx_face = 0; idx_face < nb_faces; idx_face++)
			{
				
				faces[idx_face].mNumIndices = 3;
				faces[idx_face].mIndices = new unsigned[3];
				faces[idx_face].mIndices[0] = mesh_.faces_indexes[idx_face].x;
				faces[idx_face].mIndices[1] = mesh_.faces_indexes[idx_face].y;
				faces[idx_face].mIndices[2] = mesh_.faces_indexes[idx_face].z;
				
			}


			aiBone** bones = new aiBone*[mesh_.bones.size()];
			size_t nb_bones = 0;

			nb_bones = mesh_.bones.size();
			unsigned int count_bones = 0;

			
			for (auto it_b : mesh_.bones) {

				bones[count_bones] = new aiBone();
				bones[count_bones]->mName = it_b.second.name;
				bones[count_bones]->mNumWeights = it_b.second.weights.size();
				bones[count_bones]->mWeights = new aiVertexWeight[bones[count_bones]->mNumWeights]();
				
				for (unsigned int idx_w = 0; idx_w < bones[count_bones]->mNumWeights; idx_w++) {


					aiVertexWeight weight;
					weight.mWeight = it_b.second.weights[idx_w];
					weight.mVertexId = it_b.second.idx_v[idx_w];

					vector3<float> pos = mesh_.vertices[weight.mVertexId];

					
					bones[count_bones]->mWeights[idx_w] = weight;
				}
				aiMatrix4x4 mat = it_b.second.offset_matrix;
				bones[count_bones]->mOffsetMatrix = mat;
				bones[count_bones]->mOffsetMatrix = bones[count_bones]->mOffsetMatrix;

				bones_ptr[bones[count_bones]->mName.C_Str()].push_back(bones[count_bones]);
				

				count_bones++;
			}
			
			if (mat_id_to_idx.count(mesh_.mat_id) == 0) {
				
				aimaterials[count_total_material] = guess_material_from_mat_struct(scene.mats[mesh_.mat_id], mesh_.mat_variant);
				mat_id_to_idx[mesh_.mat_id] = count_total_material;
				count_total_material++;
			}
			aimeshes[count_mesh]->mMaterialIndex = mat_id_to_idx[mesh_.mat_id];
		
			map_ptr[it_msh.first]->mMeshes[count_mesh_within_node] = count_mesh;


			
			count_mesh_within_node++;
			aimeshes[count_mesh]->mNumBones = mesh_.bones.size();
			aimeshes[count_mesh]->mBones = bones;
			aimeshes[count_mesh]->mNumVertices = mesh_.vertices.size();
			aimeshes[count_mesh]->mNumFaces = nb_faces;
			aimeshes[count_mesh]->mFaces = faces;
			aimeshes[count_mesh]->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
			aimeshes[count_mesh]->mName = mesh_.name;

			
			count_mesh++;
		}
	}

	aiNode* rootNode = NULL;
	idx_node = 0;
	for (idx_node = 0; idx_node < nb_nodes; idx_node++) {
		aiNode* current_node = aiNodes[idx_node];
		std::string name = current_node->mName.C_Str();
		if (name.compare("root") == 0)
			rootNode = current_node;
		std::vector<std::string> children_ = children[idx_node];
		
		for (unsigned int i = 0; i < children_.size(); i++){

			current_node->mChildren[i] = map_ptr[children_[i]];
			map_ptr[children_[i]]->mParent = current_node;
		}
	}

	aiAnimation** animations = new aiAnimation * [scene.anis.size()];
	unsigned int id_ani = 0;
	for (auto ani_ : scene.anis) {


		aiAnimation* ani = new aiAnimation;
		ani->mDuration = ani_.second.duration;
		ani->mName = ani_.second.name;
		ani->mTicksPerSecond = 24;
		
		aiNodeAnim** ani_nodes = new aiNodeAnim * [ani_.second.bones_data.size()];

		unsigned int count_bones_with_ibm = 0;
		for (auto it_nd : ani_.second.bones_data) {
			std::string current_bone = it_nd.first;
			aiNode* current_node = map_ptr[current_bone];
			
			aiNodeAnim* node_ani = new aiNodeAnim();


			keys keys = it_nd.second;

			std::vector<aiVectorKey> position_keys;
			std::vector<aiQuatKey> rotation_keys;
			std::vector<aiVectorKey> scaling_keys;
			for (unsigned int i = 0; i < keys.positions.size(); i++) {
				int tick = keys.positions[i].tick;
				if (tick < ani_.second.start)
					tick = ani_.second.start;
				if (tick > (ani_.second.end))
					tick = ani_.second.end;

				auto k_t = aiVector3D(keys.positions[i].data.x, keys.positions[i].data.y, keys.positions[i].data.z); //posi - pos_bp;
				position_keys.push_back(aiVectorKey(tick / ani->mTicksPerSecond - ani_.second.start / ani->mTicksPerSecond, k_t));

			}
			for (unsigned int i = 0; i < keys.scales.size(); i++) {
				int tick = keys.scales[i].tick;
				if (tick < ani_.second.start)
					tick = ani_.second.start;
				if (tick > (ani_.second.end))
					tick = ani_.second.end;

				auto k_s = aiVector3D(keys.scales[i].data.x, keys.scales[i].data.y, keys.scales[i].data.z); //posi - pos_bp;
				scaling_keys.push_back(aiVectorKey(tick / ani->mTicksPerSecond - ani_.second.start / ani->mTicksPerSecond, k_s));

			}
			for (unsigned int i = 0; i < keys.rotations.size(); i++) {
				int tick = keys.rotations[i].tick;
				if (tick < ani_.second.start)
					tick = ani_.second.start;
				if (tick > (ani_.second.end))
					tick = ani_.second.end;

				auto k_r = aiQuaternion(keys.rotations[i].data.t, keys.rotations[i].data.x, keys.rotations[i].data.y, keys.rotations[i].data.z); //posi - pos_bp;
				
																																				 //k_r = k_r.Conjugate();
				rotation_keys.push_back(aiQuatKey(tick / ani->mTicksPerSecond - ani_.second.start / ani->mTicksPerSecond, k_r));
				/*if (current_bone.compare("Bone_root") == 0)
					std::cout << (tick / ani->mTicksPerSecond - ani_.second.start / ani->mTicksPerSecond) << " " << k_r.x << " " << k_r.y << " " << k_r.z << " " << k_r.w << std::endl;*/
			}
				
			
			
			node_ani->mNodeName = current_bone;
			node_ani->mNumPositionKeys = position_keys.size();
			node_ani->mNumRotationKeys = rotation_keys.size();
			node_ani->mNumScalingKeys = scaling_keys.size();

			node_ani->mPositionKeys = new aiVectorKey[node_ani->mNumPositionKeys];
			node_ani->mRotationKeys = new aiQuatKey[node_ani->mNumRotationKeys];
			node_ani->mScalingKeys = new aiVectorKey[node_ani->mNumScalingKeys];

			memcpy(node_ani->mPositionKeys, position_keys.data(), node_ani->mNumPositionKeys * sizeof(aiVectorKey));
			memcpy(node_ani->mRotationKeys, rotation_keys.data(), node_ani->mNumRotationKeys * sizeof(aiQuatKey));
			memcpy(node_ani->mScalingKeys, scaling_keys.data(), node_ani->mNumScalingKeys * sizeof(aiVectorKey));


			ani_nodes[count_bones_with_ibm] = node_ani;
			count_bones_with_ibm++;
			
		}
		if (count_bones_with_ibm > 0) {
			ani->mChannels = ani_nodes;
			ani->mNumChannels = count_bones_with_ibm;
			animations[id_ani] = ani;
			id_ani++;
		}
		



	}

	aiScene *out = new aiScene();                    
	out->mNumMeshes = real_nb_of_meshes;
	out->mMeshes = aimeshes;           
	out->mNumMaterials = count_total_material;
	out->mMaterials = aimaterials;
	out->mRootNode = rootNode;
	out->mMetaData = new aiMetadata();
	out->mAnimations = animations;
	out->mNumAnimations = scene.anis.size();
	// mtb.data.size();
	
	unsigned preprocessingFlags = aiProcess_JoinIdenticalVertices;
	Assimp::Exporter exporter;
	std::cout << "Writing " << scene.name + "/" + scene.name + ".fbx" << std::endl;
	exporter.Export(out, "fbx", scene.name + "/" + scene.name + ".fbx", preprocessingFlags);

	delete out;

}



FBXExporter::FBXExporter()
{
	
}


FBXExporter::~FBXExporter()
{
}
