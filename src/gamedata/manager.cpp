#include <gamedata/manager.h>
#include <gamedata/gamedata.h>
#include <gamedata/btx2_file.h>
#include <gamedata/pkg_file.h>
#include <gamedata/tex_file.h>
#include <gamedata/inst_file.h>
#include <gamedata/bbnd_file.h>
#include <gamedata/vag_file.h>
#include <gamedata/mission.h>
#include <gamedata/vehicle_props.h>

#include <entities/pkg.h>
#include <entities/vehicle.h>

#include <r2/engine.h>
#include <r2/utilities/utils.h>
using namespace r2;

namespace sr2 {
	gamedata_manager::gamedata_manager(scene* s) :
		m_scene(s), m_archive(nullptr), m_terrainVertexFormat(nullptr),
		m_pkgVertexFormat(nullptr), m_pkgInstanceFormat(nullptr),
		m_pkgMtrlFormat(nullptr), m_pkgMtrl(nullptr), m_ramshop(nullptr)
	{
		m_archive = new sr2::dave_zip();
		if (!m_archive->load("./resource/data/sr.zip")) {
			exit(-1);
			return;
		}

		m_ramshop = new sr2::ramshop_zip();
		if (!m_ramshop->load("./resource/data/ramshop.zip")) {
			exit(-1);
			return;
		}

		m_terrainVertexFormat = new vertex_format();
		m_terrainVertexFormat->add_attr(vat_vec3f); // positions
		m_terrainVertexFormat->add_attr(vat_vec2f); // texcoords
		m_terrainVertexFormat->add_attr(vat_vec2f); // tex*map coords

		m_pkgVertexFormat = new vertex_format();
		m_pkgVertexFormat->add_attr(vat_vec3f); // positions
		m_pkgVertexFormat->add_attr(vat_vec3f); // normal
		m_pkgVertexFormat->add_attr(vat_vec2f); // texcoords

		m_pkgInstanceFormat = new instance_format();
		m_pkgInstanceFormat->add_attr(iat_mat4f, true); // transform

		m_pkgMtrlFormat = new uniform_format();
		m_pkgMtrlFormat->add_attr("ambient", uat_vec4f);
		m_pkgMtrlFormat->add_attr("diffuse", uat_vec4f);
		m_pkgMtrlFormat->add_attr("specular", uat_vec4f);
		m_pkgMtrlFormat->add_attr("emissive", uat_vec4f);
		m_pkgMtrlFormat->add_attr("tex_factor", uat_float);
		m_pkgMtrlFormat->add_attr("highlight", uat_float);

		m_pkgShdr = m_scene->load_shader("./resource/shaders/pkg.glsl", "pkg_shader");
		m_pkgMtrl = new node_material("u_material", m_pkgMtrlFormat);
		m_pkgMtrl->set_shader(m_pkgShdr);

		m_skyMtrlFormat = new uniform_format();
		m_skyMtrlFormat->add_attr("ambient", uat_vec4f);
		m_skyMtrlFormat->add_attr("diffuse", uat_vec4f);
		m_skyMtrlFormat->add_attr("specular", uat_vec4f);
		m_skyMtrlFormat->add_attr("emissive", uat_vec4f);
		m_skyMtrlFormat->add_attr("tex_factor", uat_float);

		m_skyShdr = m_scene->load_shader("./resource/shaders/sky.glsl", "sky_shader");
		m_skyMtrl = new node_material("u_material", m_skyMtrlFormat);
		m_skyMtrl->set_shader(m_skyShdr);

		m_fogFormat = new uniform_format();
		m_fogFormat->add_attr("start", uat_float);
		m_fogFormat->add_attr("end", uat_float);
		m_fogFormat->add_attr("clamp", uat_float);
		m_fogFormat->add_attr("color", uat_vec3f);
		m_fogUniforms = m_scene->allocate_uniform_block("u_fog", m_fogFormat);

		m_terrainLightMap = nullptr;
	}

	gamedata_manager::~gamedata_manager() {
		delete m_terrainVertexFormat;
		delete m_pkgVertexFormat;
		delete m_pkgInstanceFormat;
		delete m_pkgMtrlFormat;
		delete m_skyMtrlFormat;
		delete m_fogFormat;
		delete m_archive;
		delete m_ramshop;
	}

	std::pair<btx2_file*, r2::render_node*> gamedata_manager::get_terrain(const mstring& file, const mstring& lightMapFile, node_material_instance* material) {
		using result = std::pair<btx2_file*, r2::render_node*>;
		btx2_file* terrain = m_archive->open_terrain(file);
		if (!terrain) return result(nullptr, nullptr);

		f32 min = 100000.0f;
		f32 max = 0.0f;
		f32* heights = terrain->height_map();
		u32 width = terrain->width();
		u32 length = terrain->length();
		for (u32 i = 0; i < width * length; i++) {
			if (heights[i] < min) min = heights[i];
			if (heights[i] > max) max = heights[i];
		}

		struct tvert {
			vec3f position;
			vec2f tex0;
			vec2f tex1;
		};
		dynamic_pod_array<tvert> vertices(size_t(width) * size_t(length));
		dynamic_pod_array<u32> indices((size_t(width) - 1) * (size_t(length) - 1) * 6);

		u32 idxOffset = 0;
		for (u32 x = 0; x < width; x++) {
			for (u32 z = 0; z < length; z++) {
				u32 hidx = x + (z * width);
				u32 vidx = x + (z * width);
				f32 raw_height = heights[hidx];
				f32 height_val = raw_height * 0.2f;

				*vertices[vidx] = {
					vec3f(f32(x), height_val, f32(z)),
					vec2f(f32(x) / f32(width), f32(z) / f32(length)),
					vec2f(f32(x), f32(z))
				};

				if (x < width - 1 && z < length - 1) {
					u32 tl = vidx;
					u32 tr = vidx + 1;
					u32 bl = vidx + width;
					u32 br = bl + 1;
					*indices[idxOffset++] = bl;
					*indices[idxOffset++] = tl;
					*indices[idxOffset++] = tr;
					*indices[idxOffset++] = bl;
					*indices[idxOffset++] = tr;
					*indices[idxOffset++] = br;
				}
			}
		}

		mesh_construction_data* mesh = new mesh_construction_data(m_terrainVertexFormat, it_unsigned_int);
		mesh->set_max_vertex_count(vertices.size());
		mesh->set_max_index_count(indices.size());
		size_t vc = vertices.for_each([mesh](tvert* vertex) { return mesh->append_vertex(*vertex); });
		if (vc != vertices.size()) {
			delete mesh;
			delete terrain;
			return result(nullptr, nullptr);
		}

		size_t ic = indices.for_each([mesh](u32* index) { return mesh->append_index(*index); });
		if (ic != indices.size()) {
			delete mesh;
			delete terrain;
			return result(nullptr, nullptr);
		}

		render_node* node = m_scene->add_mesh(mesh);
		if (!node) {
			delete mesh;
			delete terrain;
			return result(nullptr, nullptr);
		}

		for (size_t i = 0;i < terrain->texture_count();i++) {
			auto tex = get_texture(terrain->texture(i));
			if (tex) material->set_texture(format_string("textures[%llu]", i), tex);
			else {
				r2Warn("Terrain texture '%s' failed to load. Too late to delete the node, it'll just be missing this texture", terrain->texture(i).c_str());
			}
		}

		texture_buffer* texIds = m_scene->create_texture();
		texIds->create(terrain->tex_id_map(), width, length, 1, tt_unsigned_byte);
		material->set_texture("textureIds", texIds);

		texture_buffer* texOrientations = m_scene->create_texture();
		texOrientations->create(terrain->tex_orientation_map(), width, length, 1, tt_unsigned_byte);
		material->set_texture("textureOrientations", texOrientations);

		texture_buffer* pathTexIds = m_scene->create_texture();
		pathTexIds->create(terrain->path_tex_id_map(), width, length, 1, tt_unsigned_byte);
		material->set_texture("pathTextureIds", pathTexIds);

		texture_buffer* pathInfo = m_scene->create_texture();
		pathInfo->create((u8*)terrain->path_info_map(), width, length, 4, tt_unsigned_int);
		material->set_texture("pathInfo", pathInfo);

		data_container* lm = m_archive->open(lightMapFile, DM_BINARY);
		if (lm) {
			assert(lm->size() == width * length);
			m_terrainLightMap = m_scene->create_texture();
			m_terrainLightMap->set_mag_filter(tmgf_linear);
			m_terrainLightMap->set_min_filter(tmnf_linear);
			m_terrainLightMap->create(width, length, 1, tt_unsigned_byte);
			lm->read_data(m_terrainLightMap->data(), width * length);
			material->set_texture("lightMap", m_terrainLightMap);
			r2engine::files()->destroy(lm);
		}

		node->add_uniform_block(m_fogUniforms);

		m_pkgShdr->activate();
		m_pkgShdr->uniform2f(m_pkgShdr->get_uniform_location("terrainSize"), width, length);
		m_pkgShdr->deactivate();

		shader_program* tshdr = material->material()->shader();
		tshdr->activate();
		tshdr->uniform2f(tshdr->get_uniform_location("terrainSize"), width, length);
		tshdr->deactivate();

		return result(terrain, node);
	}

	mvector<std::pair<render_node*, bbnd_file*>> gamedata_manager::get_mesh(const mstring& file, u32 max_instances) {
		if (m_meshes.count(file) != 0) return m_meshes[file];
		pkg_file* pkg = m_archive->open_mesh(file);
		if (!pkg) {
			pkg = m_ramshop->open_mesh(file);
			if (!pkg) return mvector<std::pair<render_node*, bbnd_file*>>();
		}

		u32 i0 = file.find_last_of("/");
		if (i0 == string::npos) i0 = file.find_last_of("\\");
		i0++;
		mstring meshName = file.substr(i0, file.find_last_of(".") - i0);
		bbnd_file* bounds = nullptr;
		if (m_archive->exists("bound/" + meshName + "_BOUND.bbnd")) {
			bounds = m_archive->open_mesh_bounds("bound/" + meshName + "_BOUND.bbnd");
		}

		mvector<std::pair<render_node*, bbnd_file*>> nodes;

		for (auto& mesh : pkg->meshes) {
			mesh_construction_data* mc = new mesh_construction_data(m_pkgVertexFormat, it_unsigned_int, m_pkgInstanceFormat);
			mc->set_max_vertex_count(mesh.vertices.size());
			mc->set_max_index_count(mesh.indices.size());
			mc->set_max_instance_count(max_instances);

			bool okay = true;
			for (u32 v = 0;v < mesh.vertices.size();v++) {
				if (!mc->append_vertex(mesh.vertices[v])) {
					delete mc;
					okay = false;
					break;
				}
			}

			for (u32 idx : mesh.indices) {
				if (!mc->append_index(idx)) {
					delete mc;
					okay = false;
					break;
				}
			}

			for (u32 i = 0;i < max_instances;i++) {
				if (!mc->append_instance(mat4f(1.0f))) {
					delete mc;
					okay = false;
					break;
				}
			}

			if (okay) {
				render_node* node = m_scene->add_mesh(mc);
				if (node) {
					node->add_uniform_block(m_fogUniforms);
					node_material_instance* instance = m_pkgMtrl->instantiate(m_scene);
					if (m_terrainLightMap) instance->set_texture("lightMap", m_terrainLightMap);
					node->set_material_instance(instance);
					if (mesh.material.texture.length() > 0) {
						texture_buffer* tex = get_texture(mesh.material.texture);
						if (tex) {
							instance->set_texture("tex", tex);
							struct color4b { u8 r, g, b, a; };
							color4b* pixels = (color4b*)tex->data();
							for (size_t i = 0;i < tex->width() * tex->height();i++) {
								if (pixels[i].a != 255) {
									node->has_transparency = true;
									break;
								}
							}
							instance->uniforms()->uniform_float("tex_factor", 1.0f);
						} else {
							mvector<texture_buffer*> sequence = get_texture_sequence(mesh.material.texture);
							if (sequence.size() > 0) {
								instance->set_texture("tex", sequence, 1.0f, true);
								instance->uniforms()->uniform_float("tex_factor", 1.0f);
							} else instance->uniforms()->uniform_float("tex_factor", 0.0f);
						}
					} else instance->uniforms()->uniform_float("tex_factor", 0.0f);

					instance->uniforms()->uniform_vec4f("ambient", mesh.material.unk[0]);
					instance->uniforms()->uniform_vec4f("diffuse", mesh.material.unk[1]);
					instance->uniforms()->uniform_vec4f("specular", mesh.material.unk[2]);
					instance->uniforms()->uniform_vec4f("emissive", mesh.material.unk[3]);
					instance->uniforms()->uniform_float("highlight", 0.0f);
					nodes.push_back(std::pair<render_node*, bbnd_file*>(node, bounds));
				}
			}
		}

		return nodes;
	}

	render_node* gamedata_manager::get_sky(const mstring& file) {
		data_container* sf = open(file, DM_TEXT);
		if (!sf) return nullptr;
		mstring line;
		sf->read_line(line);
		r2engine::files()->destroy(sf);

		mvector<mstring> comps = split(line, " \t\n\r");
		if (comps.size() != 6) {
			r2Error("Invalid sky file '%s'", file.c_str());
			return nullptr;
		}

		pkg_file* pkg = m_archive->open_mesh("geometry/" + comps[0] + ".pkg");
		if (!pkg) {
			pkg = m_ramshop->open_mesh(file);
			if (!pkg) return nullptr;
		}

		render_node* node = nullptr;
		if (pkg->meshes.size() > 1) {
			r2Error("Sky has multiple meshes 'geometry/%s.pkg'", comps[0].c_str());
			delete pkg;
			return nullptr;
		}

		mstring texture = "s_sky_gray";
		if (comps[0] == "a_morning01") texture = "a_sky_mor";
		else if (comps[0] == "a_sunset01") texture = "a_sky_set";
		else if (comps[0] == "a_rain01") texture = "a_sky_rain03";
		else if (comps[0] == "v_sky_eve03") texture = "v_sky_eve03";
		else if (comps[0] == "v_sky_eve02") texture = "v_sky_eve";
		else if (comps[0] == "v_sky_eve01") texture = "v_sky_eve";
		else if (comps[0] == "v_sky_day01") texture = "v_sky_day";
		else if (comps[0] == "s_sky_morn01") texture = "s_sky_mor";
		else if (comps[0] == "s_sky_eve01") texture = "s_sky_set";
		texture = "texture/" + texture + ".tex";

		for (auto& mesh : pkg->meshes) {
			mesh_construction_data* mc = new mesh_construction_data(m_pkgVertexFormat, it_unsigned_int);
			mc->set_max_vertex_count(mesh.vertices.size());
			mc->set_max_index_count(mesh.indices.size());

			bool okay = true;
			f32 radius = 0.0f;
			for (u32 v = 0;v < mesh.vertices.size();v++) {
				f32 l = glm::length(mesh.vertices[v].position);
				if (l > radius) radius = l;
			}
			radius = 1.0f / radius;
			for (u32 v = 0;v < mesh.vertices.size();v++) {
				mesh.vertices[v].position *= radius;
				if (!mc->append_vertex(mesh.vertices[v])) {
					delete mc;
					okay = false;
					break;
				}
			}
			for (u32 v = 0;v < mesh.vertices.size();v++) {
				f32 l = glm::length(mesh.vertices[v].position);
				if (l > radius) radius = l;
			}

			for (u32 idx : mesh.indices) {
				if (!mc->append_index(idx)) {
					delete mc;
					okay = false;
					break;
				}
			}

			if (okay) {
				node = m_scene->add_mesh(mc);
				delete mc;
				if (node) {
					node->add_uniform_block(m_fogUniforms);
					node_material_instance* instance = m_skyMtrl->instantiate(m_scene);
					node->set_material_instance(instance);
					if (mesh.material.texture.length() > 0) texture = mesh.material.texture;

					texture_buffer* tex = get_texture(texture);
					if (tex) {
						instance->set_texture("tex", tex);
						struct color4b { u8 r, g, b, a; };
						color4b* pixels = (color4b*)tex->data();
						for (size_t i = 0;i < tex->width() * tex->height();i++) {
							if (pixels[i].a != 255) {
								node->has_transparency = true;
								break;
							}
						}
						instance->uniforms()->uniform_float("tex_factor", 1.0f);
					} else {
						mvector<texture_buffer*> sequence = get_texture_sequence(mesh.material.texture);
						if (sequence.size() > 0) {
							instance->set_texture("tex", sequence, 1.0f, true);
							instance->uniforms()->uniform_float("tex_factor", 1.0f);
						} else instance->uniforms()->uniform_float("tex_factor", 0.0f);
					}

					instance->uniforms()->uniform_vec4f("ambient", mesh.material.unk[0]);
					instance->uniforms()->uniform_vec4f("diffuse", mesh.material.unk[1]);
					instance->uniforms()->uniform_vec4f("specular", mesh.material.unk[2]);
					instance->uniforms()->uniform_vec4f("emissive", mesh.material.unk[3]);
				} else {
					delete pkg;
					return nullptr;
				}
			} else {
				delete mc;
				delete pkg;
				return nullptr;
			}
		}

		delete pkg;
		return node;
	}

	uniform_block* gamedata_manager::get_fog(const mstring& file) {
		data_container* ff = open(file, DM_TEXT);
		if (!ff) return nullptr;
		while (!ff->at_end(1)) {
			mstring line;
			if (!ff->read_line(line)) break;

			mvector<mstring> comps = split(line, " \t\n\r");
			if (comps.size() == 0) continue;

			if (comps[0] == "fogstart") {
				if (comps.size() != 2) {
					r2Error("Invalid 'fogstart' line in fog file '%s'", file.c_str());
					continue;
				}

				m_fogUniforms->uniform_float("start", atof(comps[1].c_str()));
			}
			else if (comps[0] == "fogend") {
				if (comps.size() != 2) {
					r2Error("Invalid 'fogend' line in fog file '%s'", file.c_str());
					continue;
				}

				m_fogUniforms->uniform_float("end", atof(comps[1].c_str()));
			}
			else if (comps[0] == "fogclamp") {
				if (comps.size() != 2) {
					r2Error("Invalid 'fogclamp' line in fog file '%s'", file.c_str());
					continue;
				}

				m_fogUniforms->uniform_float("clamp", atof(comps[1].c_str()));
			}
			else if (comps[0] == "fogcolor") {
				if (comps.size() != 4) {
					r2Error("Invalid 'fogcolor' line in fog file '%s'", file.c_str());
					continue;
				}
				m_fogUniforms->uniform_vec3f("color", vec3f(
					atof(comps[1].c_str()) / 255.0f,
					atof(comps[2].c_str()) / 255.0f,
					atof(comps[3].c_str()) / 255.0f
				));
			}
		}
		r2engine::files()->destroy(ff);

		return m_fogUniforms;
	}

	vehicle_entity* gamedata_manager::get_vehicle(const mstring& file, u32 max_instances) {
		pkg_file* pkg = m_ramshop->open_mesh(file);
		if (!pkg) return nullptr;

		u32 i0 = file.find_last_of("/");
		if (i0 == string::npos) i0 = file.find_last_of("\\");
		i0++;
		mstring vehicleName = file.substr(i0, file.find_last_of(".") - i0);


		munordered_map<mstring, mvector<scene_entity*>> entityMap;
		for (auto& mesh : pkg->meshes) {
			mat4f transform = mat4f(1.0f);

			mstring meshName = mesh.name.substr(0, mesh.name.find_first_of("_"));
			if (meshName == "SHADOW") continue;
			if (meshName != "BODY" && meshName != "SHADOW") {
				mstring mtxPath = "geometry/" + vehicleName + "_" + meshName + ".mtx";
				if (!m_ramshop->exists(mtxPath) && !m_archive->exists(mtxPath)) {
					r2Log("No .mtx file exists for %s's '%s' mesh", vehicleName.c_str(), meshName.c_str());
				} else {
					data_container* mtx = open(mtxPath, DM_BINARY);
					if (mtx) {
						glm::mat4x3 mat;
						if (mtx->read_data(&mat, 48)) {
							transform = glm::mat4(mat);
						}
					}
					r2engine::files()->destroy(mtx);
				}
			}

			// create entity
			mesh_construction_data* mc = new mesh_construction_data(m_pkgVertexFormat, it_unsigned_int, m_pkgInstanceFormat);
			mc->set_max_vertex_count(mesh.vertices.size());
			mc->set_max_index_count(mesh.indices.size());
			mc->set_max_instance_count(max_instances);

			bool okay = true;
			for (u32 v = 0;v < mesh.vertices.size();v++) {
				if (!mc->append_vertex(mesh.vertices[v])) {
					delete mc;
					okay = false;
					break;
				}
			}

			for (u32 idx : mesh.indices) {
				if (!mc->append_index(idx)) {
					delete mc;
					okay = false;
					break;
				}
			}

			for (u32 i = 0;i < max_instances;i++) {
				if (!mc->append_instance(mat4f(1.0f))) {
					delete mc;
					okay = false;
					break;
				}
			}

			if (okay) {
				render_node* node = m_scene->add_mesh(mc);
				if (node) {
					node->add_uniform_block(m_fogUniforms);
					node_material_instance* instance = m_pkgMtrl->instantiate(m_scene);
					if (m_terrainLightMap) instance->set_texture("lightMap", m_terrainLightMap);
					node->set_material_instance(instance);
					if (mesh.material.texture.length() > 0) {
						texture_buffer* tex = get_texture(mesh.material.texture);
						if (tex) {
							instance->set_texture("tex", tex);
							struct color4b { u8 r, g, b, a; };
							color4b* pixels = (color4b*)tex->data();
							for (size_t i = 0;i < tex->width() * tex->height();i++) {
								if (pixels[i].a != 255) {
									node->has_transparency = true;
									break;
								}
							}
							instance->uniforms()->uniform_float("tex_factor", 1.0f);
						}
						else {
							mvector<texture_buffer*> sequence = get_texture_sequence(mesh.material.texture);
							if (sequence.size() > 0) {
								instance->set_texture("tex", sequence, 1.0f, true);
								instance->uniforms()->uniform_float("tex_factor", 1.0f);
							} else instance->uniforms()->uniform_float("tex_factor", 0.0f);
						}
					}
					else instance->uniforms()->uniform_float("tex_factor", 0.0f);

					instance->uniforms()->uniform_vec4f("ambient", mesh.material.unk[0]);
					instance->uniforms()->uniform_vec4f("diffuse", mesh.material.unk[1]);
					instance->uniforms()->uniform_vec4f("specular", mesh.material.unk[2]);
					instance->uniforms()->uniform_vec4f("emissive", mesh.material.unk[3]);
					instance->uniforms()->uniform_float("highlight", 0.0f);

					entityMap[meshName].push_back(
						new pkg_entity(meshName, node, nullptr, transform)
					);
				}
			}
		}

		vehicle_entity* root = new vehicle_entity(vehicleName);
		for (auto& info : entityMap) {
			mstring name = info.first;
			auto& entities = info.second;

			scene_entity* entity = entities[0];
			if (entities.size() > 1) {
				entity = new scene_entity(name);
				u32 idx = 0;
				for (scene_entity* e : entities) {
					e->set_name(format_string("%s_%d", name.c_str(), idx++));
					entity->add_child_entity(e);
				}
			}

			if (name.find("WHL") != mstring::npos) {
				u32 idx = atoi(name.substr(3).c_str());
				root->wheels[idx] = entity;
			} else if (name.find("AXLE") != mstring::npos) {
				u32 idx = atoi(name.substr(4).c_str());
				root->axles[idx] = entity;
			} else if (name.find("EXHAUST") != mstring::npos) {
				u32 idx = atoi(name.substr(7).c_str());
				root->exhausts[idx] = entity;
			} else if (name.find("BREAK") != mstring::npos) {
				u32 idx = atoi(name.substr(5).c_str());
				root->breakables[idx] = entity;
			} else if (name.find("SHOCK") != mstring::npos) {
				u32 idx = atoi(name.substr(5).c_str());
				root->shocks[idx] = entity;
			} else if (name.find("SRN") != mstring::npos) {
				u32 idx = atoi(name.substr(3).c_str());
				root->sirens[idx] = entity;
			} else if (name.find("SIREN") != mstring::npos) {
				u32 idx = atoi(name.substr(5).c_str());
				root->sirens[idx] = entity;
			} else if (name.find("ARM") != mstring::npos) {
				u32 idx = atoi(name.substr(3).c_str());
				root->arms[idx] = entity;
			} else if (name.find("VARIANT") != mstring::npos) {
				u32 idx = atoi(name.substr(7).c_str());
				root->variants[idx] = entity;
			} else if (name.find("SHAFT") != mstring::npos) {
				u32 idx = atoi(name.substr(5).c_str());
				root->shafts[idx] = entity;
			} else if (name == "DRIVER") root->driver = entity;
			else if (name == "SHADOW") root->shadow = entity;
			else if (name == "OIL") root->oil = entity;
			else if (name == "ENGINE") root->engine = entity;
			else if (name == "BODY") root->body = entity;
			
			root->add_child_entity(entity);
		}

		vehicle_camera* cam = new vehicle_camera(vehicleName + "_camera");
		root->camera = cam;
		cam->tracking = root;

		if (m_archive->exists("bound/" + vehicleName + "_BOUND.bnd")) {
			root->bounds = m_archive->open_mesh_bounds_text("bound/" + vehicleName + "_BOUND.bnd");
		}

		vehicle_props_loader props_loader;
		if (!props_loader.load(this, root, vehicleName)) {
			r2Error("Failed to load vehicle props");
		}


		if (root->props.sound.engine_sample_count > 0) {
			root->engine_sounds = new audio_source*[root->props.sound.engine_sample_count];
			for (u8 i = 0;i < root->props.sound.engine_sample_count;i++) {
				mstring audioName = root->props.sound.engine_samples[i].file_name;
				root->engine_sounds[i] = get_audio(format_string("aud/audvag/eng/%c/%s.vag", audioName[0], audioName.c_str()));
				root->engine_sounds[i]->setDoesLoop(true);
				root->engine_sounds[i]->setGain(0.0f);
			}
		}

		return root;
	}

	audio_source* gamedata_manager::get_audio(const mstring& file) {
		vag_file* audio_data = m_archive->open_audio(file);
		if (!audio_data) {
			audio_data = m_ramshop->open_audio(file);
			if (!audio_data) return nullptr;
		}

		audio_source* src = new audio_source();
		if (!src->buffer(asf_mono_16bit, audio_data->samples(), audio_data->sampleCount() * sizeof(u16), audio_data->sampleRate())) {
			delete src;
			delete audio_data;
			return nullptr;
		}

		delete audio_data;
		return src;
	}

	texture_buffer* gamedata_manager::get_texture(const mstring& file) {
		if (m_textures.has(file)) {
			r2Log("Using cached '%s'", file.c_str());
			return *m_textures.get(file);
		}
		tex_file* tex_data = m_archive->open_texture(file);
		if (!tex_data) {
			tex_data = m_ramshop->open_texture(file);
			if (!tex_data) return nullptr;
		}

		texture_buffer* tex = m_scene->create_texture();
		tex->create(tex_data->data(), tex_data->width(), tex_data->height(), 4, tt_unsigned_byte);
		tex->set_min_filter(tmnf_linear);
		tex->set_mag_filter(tmgf_linear);
		m_textures.set(file, tex);
		return tex;
	}

	mvector<texture_buffer*> gamedata_manager::get_texture_sequence(const mstring& file) {
		mstring basefn = file.substr(0, file.find_last_of('.'));

		mvector<texture_buffer*> sequence;
		if (basefn == "texture/") return sequence;
		mvector<mstring> all = m_archive->file_list();
		mvector<mstring> relevant;
		for (mstring& file : all) {
			if (file.find(basefn) != mstring::npos) {
				relevant.push_back(file);
			}
		}
		sort(relevant.begin(), relevant.end());
		for (mstring& file : relevant) {
			texture_buffer* tex = get_texture(file);
			if (tex) sequence.push_back(tex);
		}

		return sequence;
	}

	mvector<pkg_entity*> gamedata_manager::get_instances(const mstring& file) {
		inst_file* instance_data = m_archive->open_instances(file);
		if (!instance_data) return mvector<pkg_entity*>();

		r2Log("Creating entities for each instance");
		mvector<pkg_entity*> entities;
		u32 limit = 212000;
		u32 cur = 0;
		u32 num = instance_data->instances.size();
		for (auto& it : instance_data->instances) {
			mstring file = it.first;
			mvector<inst_file::instance>& instances = it.second;

			mvector<std::pair<render_node*, bbnd_file*>> nodes = get_mesh(file, instances.size());
			r2Log("Processing '%s' (%d of %d) (%llu instances, %llu nodes)", file.c_str(), cur, num, instances.size(), nodes.size());
			u32 idx = 0;
			for (auto& instance : instances) {
				vec2f rotAngles = instance.unk4;
				f32 rot = atan2(-rotAngles.y, rotAngles.x);
				f32 scale = glm::length(rotAngles);

				mat4f transform = mat4f(1.0f);
				transform = glm::translate(transform, instance.position);
				transform = glm::rotate(transform, rot, vec3f(0, 1, 0));
				//transform = glm::scale(transform, vec3f(scale, scale, scale));

				if (nodes.size() == 1) {
					pkg_entity* entity = new pkg_entity(format_string("%s[%d]", file.c_str(), idx), nodes[0].first, nodes[0].second, transform);
					entities.push_back(entity);
				} else {
					pkg_entity* root = new pkg_entity(format_string("%s[%d] (root)", file.c_str(), idx), nullptr, nullptr, mat4f(1.0f));

					u32 nidx = 0;
					for (auto& info : nodes) {
						render_node* node = info.first;
						bbnd_file* bounds = info.second;

						pkg_entity* entity = new pkg_entity(format_string("%s[%d] (child: %d)", file.c_str(), idx, nidx), node, bounds, transform);
						root->add_child_entity(entity);
						nidx++;
						if (entities.size() > limit) break;
					}

					entities.push_back(root);
				}
				idx++;
				if (entities.size() > limit) break;
			}

			if (entities.size() > limit) break;
			cur++;
		}
		delete instance_data;

		r2Log("Finished creating entities");
		return entities;
	}

	mission* gamedata_manager::get_mission(const mstring& name) {
		mission* miss = new mission();
		if (!miss->load(this, name)) {
			delete miss;
			miss = nullptr;
		}
		return miss;
	}

	data_container* gamedata_manager::open(const mstring& file, r2::DATA_MODE mode) {
		data_container* data = m_archive->open(file, mode);

		if (!data) {
			data = m_ramshop->open(file, mode);
			if (!data) return nullptr;
		}

		return data;
	}

	void gamedata_manager::render_debug_ui() {
		static float ambientMult = 0.48f;
		ImGui::Text("Ambient Multiplier");
		ImGui::InputFloat("##ambmult", &ambientMult, 0.01f, 0.01f, 3);
		m_pkgShdr->activate();
		m_pkgShdr->uniform1f(m_pkgShdr->get_uniform_location("ambientMult"), ambientMult);
		m_pkgShdr->deactivate();
	}
};