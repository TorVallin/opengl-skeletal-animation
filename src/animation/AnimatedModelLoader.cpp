//
// Created by tor on 3/23/23.
//

#include "AnimatedModelLoader.h"

static const aiScene *setup_scene(const std::string &path) {
  Assimp::Importer importer;
  aiPropertyStore *props = aiCreatePropertyStore();
  aiSetImportPropertyInteger(props, "PP_PTV_NORMALIZE", 1);
  const aiScene *scene = (aiScene *)aiImportFileExWithProperties(path.c_str(),
																 aiProcess_Triangulate
																	 | aiProcess_GenSmoothNormals
																	 | aiProcess_ForceGenNormals,
																 nullptr, props);

  if (!scene || !scene->mRootNode) {
	std::cerr << "AnimatedModel::Error - Failed to load model " <<
			  importer.GetErrorString() << std::endl;
	return nullptr;
  }
  return scene;
}

std::optional<Model> AnimatedModelLoader::load_model(const std::string &model_path, const std::string &animation_path) {
  auto model_scene = setup_scene(model_path);
  auto animation_scene = setup_scene(animation_path);
  if (model_scene == nullptr || animation_scene == nullptr) {
	return std::nullopt;
  }

  Model model{};
  load_node(model, model_scene, model_scene->mRootNode);

  auto animation = animation_scene->mAnimations[1];
  model.ticks_per_second = animation->mTicksPerSecond;
  model.animation_duration = animation->mDuration;
  load_node_animations(animation_scene, animation_scene->mRootNode, model, -1);
  load_bones(model, animation);

  model.precompute_node_bone_indices();

  return model;
}

void AnimatedModelLoader::load_node(Model &model, const aiScene *scene, const aiNode *node) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
	// The node object only contains indices to index the actual objects in the scene.
	// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
	aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
	model.mesh_list.push_back(load_mesh(scene, mesh, model));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
	load_node(model, scene, node->mChildren[i]);
  }
}

void AnimatedModelLoader::load_node_animations(const aiScene *scene,
											   const aiNode *node,
											   Model &model,
											   int parent_index) {
  int this_node_index = (int)(model.node_list.size());
  model.node_list.emplace_back(node->mName.data,
							   Conversions::convertAssimpMat4ToGLM(node->mTransformation),
							   parent_index);

  // The current node is of course a parent to its children, meaning its ID will be the childrens' parent ID
  // in the next recursive call.
  // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
	load_node_animations(scene, node->mChildren[i], model, this_node_index);
  }
}

Mesh AnimatedModelLoader::load_mesh(const aiScene *, const aiMesh *mesh, Model &model) {
  std::vector<AnimatedVertex> all_vertices;
  std::vector<unsigned int> all_indices;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
	AnimatedVertex vert{};
	vert.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

	if (mesh->mTextureCoords[0]) {
	  vert.tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
	}

	all_vertices.push_back(vert);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
	for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
	  all_indices.push_back(mesh->mFaces[i].mIndices[j]);
	}
  }

  Mesh result;
  result.vertices = all_vertices;
  result.indices = all_indices;

  load_vertex_bone_weights(mesh, result.vertices, model);
  create_mesh(result);

  return result;
}

void AnimatedModelLoader::load_vertex_bone_weights(const aiMesh *mesh,
												   std::vector<AnimatedVertex> &vertices,
												   Model &model) {
  // NOTE: bone_index refers to a "relative index", that is, it is able to index into mesh->mBones.
  // Which is NOT the same as bone_id, which is a globally unique id given to all bones.
  for (int bone_index = 0; bone_index < (int)mesh->mNumBones; bone_index++) {
	int bone_id = -1;

	const auto bone_name = mesh->mBones[bone_index]->mName.C_Str();
	auto absolute_index = model.bone_name_to_index.find(bone_name);
	if (absolute_index == model.bone_name_to_index.end()) {
	  bone_id = model.next_bone_id;
	  // bone_index does not already exist in offset_matrix, so we create it & insert it
	  model.bone_offset_matrix.emplace_back(
		  Conversions::convertAssimpMat4ToGLM(mesh->mBones[bone_index]->mOffsetMatrix)
	  );
	  model.bone_name_to_index[bone_name] = bone_id;
	  model.next_bone_id += 1;
	} else {
	  // Otherwise, the bone already exists in the map so its ID is retrieved
	  bone_id = absolute_index->second;
	}

	// Configure the vertex data (weights and which bones impact this vertex)
	auto weights = mesh->mBones[bone_index]->mWeights;
	for (unsigned int weight_index = 0; weight_index < mesh->mBones[bone_index]->mNumWeights; weight_index++) {
	  // vertex_id is the index of the vertex that is influenced by the bone with bone_id
	  auto vertex_id = weights[weight_index].mVertexId;
	  vertices[vertex_id].set_weight_to_first_unset(bone_id, weights[weight_index].mWeight);
	}
  }
}

// The animation consists of an array of channels.
// Each channel consists of keyframes, the channels also have a name that corresponds to the name of a node
// in the scene.
void AnimatedModelLoader::load_bones(Model &model, const aiAnimation *animation) {
  for (unsigned int channel_index = 0; channel_index < animation->mNumChannels; channel_index++) {
	auto channel = animation->mChannels[channel_index];

	auto bone = model.bone_name_to_index.find(channel->mNodeName.data);
	if (bone == model.bone_name_to_index.end()) {
	  assert("This should not happen, something is wrong with the bone parsing");
	}

	model.bone_list.emplace_back(model.bone_name_to_index[channel->mNodeName.data], channel);
  }
}

void AnimatedModelLoader::create_mesh(Mesh &mesh) {
  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER,
			   (long)(mesh.vertices.size() * sizeof(AnimatedVertex)),
			   &mesh.vertices[0],
			   GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			   (long)(mesh.indices.size() * sizeof(unsigned int)),
			   &mesh.indices[0],
			   GL_STATIC_DRAW);

  // Vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), nullptr);

  // Texture coordinates
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex),
						(void *)offsetof(AnimatedVertex, tex_coords));

  // Bone IDs that affect this mesh's vertices
  glEnableVertexAttribArray(2);
  glVertexAttribIPointer(2, 4, GL_INT, sizeof(AnimatedVertex),
						 (void *)offsetof(AnimatedVertex, bone_ids));

  // Bone weights that affect this mesh's vertices
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex),
						(void *)offsetof(AnimatedVertex, bone_weights));

  glBindVertexArray(0);
}
