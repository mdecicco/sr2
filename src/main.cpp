#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>
#include <r2/utilities/utils.h>
#include <r2/utilities/debug_drawer.h>
#include <r2/utilities/fly_camera.h>
#include <r2/systems/cascade_functions.h>

#include <gamedata/gamedata.h>
#include <gamedata/tex_file.h>
#include <gamedata/vag_file.h>
#include <gamedata/btx2_file.h>
#include <gamedata/mission.h>
#include <gamedata/manager.h>

#include <entities/terrain.h>
#include <entities/pkg.h>
#include <entities/vehicle.h>

#include <glm/gtx/matrix_decompose.hpp>

#define INSTANCE_COUNT 128
#define INSTANCE_RADIUS 128.0f

using namespace r2;
mvector<mstring> split(const mstring& str, const mstring& sep) {
	char* cstr = const_cast<char*>(str.c_str());
	char* current;
	mvector<mstring> arr;
	current = strtok(cstr, sep.c_str());
	while (current!=NULL) {
		arr.push_back(current);
		current = strtok(NULL, sep.c_str());
	}
	return arr;
}

render_node* load_obj(const mstring& obj, scene* destScene) {
	auto file = r2engine::get()->files()->load(obj, DM_TEXT, "obj");
	if (file) {
		mvector<mstring> lines;
		while(!file->at_end(1)) {
			mstring line;
			if(!file->read_line(line)) {
				r2Error("Failed to read %s\n", obj.c_str());
				r2engine::get()->files()->destroy(file);
				return nullptr;
			}
			lines.push_back(line);
		}
		r2engine::get()->files()->destroy(file);

		shader_program* shader = destScene->load_shader("./resource/physics_test/shader.glsl", "shader");
		if (!shader) {
			r2Error("Failed to load shader\n");
			return nullptr;
		}

		struct vertex {
			vec3f position;
			vec3f normal;
		};
		vector<vertex> vertices;
		vector<u32> indices;
		vector<vec3f> positions;
		vector<vec3f> normals;
		vector<vec2f> texcoords;
		unordered_map<mstring, u32> fvIndices;
		for(auto line : lines) {
			mvector<mstring> comps = split(line, " ");
			if (comps.size() > 0) {
				if (comps[0] == "v") {
					positions.push_back(vec3f(
						atof(comps[1].c_str()),
						atof(comps[2].c_str()),
						atof(comps[3].c_str())
					));
				} else if (comps[0] == "vn") {
					normals.push_back(vec3f(
						atof(comps[1].c_str()),
						atof(comps[2].c_str()),
						atof(comps[3].c_str())
					));
				} else if (comps[0] == "vt") {
					texcoords.push_back(vec2f(
						atof(comps[1].c_str()),
						atof(comps[2].c_str())
					));
				} else if (comps[0] == "f") {
					if (comps.size() == 4) {
						for(u8 i = 1;i < comps.size();i++) {
							if (fvIndices.find(comps[i]) != fvIndices.end()) indices.push_back(fvIndices[comps[i]]);
							else {
								vector<i32> idxGroup;
								mstring idx = "";
								for(char ch : comps[i]) {
									if (ch == '/') {
										if (idx.length() > 0) idxGroup.push_back(atoi(idx.c_str()));
										else idxGroup.push_back(-1);
										idx = "";
									} else idx += ch;
								}

								if (idx.length() > 0) idxGroup.push_back(atoi(idx.c_str()));
								else idxGroup.push_back(-1);

								vertex v;
								for(i32 j = 0;j < idxGroup.size();j++) {
									i32 gidx = idxGroup[j];
									if (j == 0 && gidx != -1) v.position = positions[gidx - 1];
									//if (j == 1 && gidx != -1) v.texcoord = texcoords[gidx - 1];
									if (j == 2 && gidx != -1) v.normal = normals[gidx - 1];
								}

								vertices.push_back(v);
								indices.push_back(vertices.size() - 1);
								fvIndices[comps[i]] = vertices.size() - 1;
							}
						}
					}
				}
			}
		}

		vertex_format* vfmt = new vertex_format();
		vfmt->add_attr(vat_vec3f);
		vfmt->add_attr(vat_vec3f);

		instance_format* ifmt = new instance_format();
		ifmt->add_attr(iat_mat4f, true);

		uniform_format* mfmt = new uniform_format();
		mfmt->add_attr("color", uat_vec3f);

		mesh_construction_data* mesh = new mesh_construction_data(vfmt, it_unsigned_short, ifmt);
		mesh->set_max_index_count(indices.size());
		mesh->set_max_vertex_count(vertices.size());
		mesh->set_max_instance_count(INSTANCE_COUNT + 1);
		for(vertex v : vertices) mesh->append_vertex(v);
		for(i32 idx : indices) mesh->append_index<u16>(idx);
		for(u32 i = 0;i < INSTANCE_COUNT + 1;i++) mesh->append_instance(mat4f(1));

		render_node* node = destScene->add_mesh(mesh);
		node_material* mtrl = new node_material("u_material", mfmt);
		mtrl->set_shader(shader);
		node->set_material_instance(mtrl->instantiate(destScene));
		node->material_instance()->uniforms()->uniform_vec3f("color", vec3f(0.75f, 0.1f, 0.2f));

		return node;
	}
}

static bool mesh_debug = false;
static bool is_vehicle = true;
#define debug_file "geometry/vpsupbug.pkg"

f32 random(f32 min = -1.0f, f32 max = 1.0f) {
	return min + f32(rand()) / (f32(RAND_MAX) / (max - min));
}

class test_entity : public scene_entity {
	public:
		test_entity(render_node* _mesh, bool _is_floor = false) : scene_entity("TestEntity"), node(_mesh), is_floor(_is_floor) {
		}

		~test_entity() {
		}

		virtual void onInitialize() {
			setUpdateFrequency(60.0f);
			stop_periodic_updates();
			transform_sys::get()->addComponentTo(this);
			mesh_sys::get()->addComponentTo(this);
			mesh->set_node(node);
			physics_sys::get()->addComponentTo(this);
		
			mat4f t(1.0f);
			f32 mass = 10.0f;
			if (is_floor) {
				mass = 0.0f;
				t = glm::translate(t, vec3f(0.0f, -2.0f, 0.0f));
				t = glm::scale(t, vec3f(100.0f, 1.0f, 100.0f));
			} else {
				vec3f r(random() * INSTANCE_RADIUS, random() * 5.0f, random() * INSTANCE_RADIUS);
				t = glm::translate(t, vec3f(896.0f * 2.5f, 50.0f, 576.0f * 2.5f) + r);
			}
			transform->transform = t;
			mesh->set_instance_transform(t);

			physics->set_mass(mass);
			physics->set_shape(new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)));
			if (is_floor) physics->rigidBody()->setFriction(0.5f);
			r2engine::get()->remove_child(this);
		}

		virtual void onUpdate(f32 frameDt, f32 updateDt) {
		}

		virtual void onEvent(event* evt) {
		}

		virtual void willBeDestroyed() {
		}

		virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) {
		}
		
		bool is_floor;
		render_node* node;
};

class physics_drawer : public btIDebugDraw {
	public:
		physics_drawer(debug_drawer* _drawer) : drawer(_drawer) {
			if (mesh_debug) debugMode = btIDebugDraw::DBG_DrawWireframe;
			else debugMode = btIDebugDraw::DBG_DrawAabb;

			if (mesh_debug && is_vehicle) {
				debugMode |= btIDebugDraw::DBG_DrawConstraints;
				debugMode |= btIDebugDraw::DBG_DrawConstraintLimits;
			}
		}

		~physics_drawer() { }

		virtual void reportErrorWarning(const char* warningString) {
			r2Warn(warningString);
		}

		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
			drawer->line(vec3f(from.x(), from.y(), from.z()), vec3f(to.x(), to.y(), to.z()), vec4f(color.x(), color.y(), color.z(), 1.0f));
		}

		virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
		}

		virtual void draw3dText(const btVector3& location, const char* textString) {
		}

		virtual void setDebugMode(int mode) {
			debugMode = mode;
		}

		virtual int getDebugMode() const {
			return debugMode;
		}

		i32 debugMode;
		debug_drawer* drawer;
};

class test_state : public state {
	public:
		test_state() : state("TestState", MBtoB(1)) {
			camera = nullptr;
			gdm = nullptr;
			terrain = nullptr;
			fullscreen_tex = nullptr;
			render_target = nullptr;
			mission = nullptr;
			draw = nullptr;
			draw_shader = nullptr;
			listItem = 0;
		}

		~test_state() {
		}



		virtual void becameActive() {
			setUpdateFrequency(5.0f);
			glEnable(GL_MULTISAMPLE);

			gdm = new sr2::gamedata_manager(getScene());

			init_scene();
			init_render_target();
			init_debug_draw();
		}

		virtual void willBecomeInactive() {
			// let the state take care of deallocations
		}

		virtual void onRender() {
			draw->begin();

			if (mesh_debug) {
				auto& ps = physics_sys::get()->physState();
				ps.enable();
				ps->world->debugDrawWorld();
				ps.disable();
			}

			bool open = true;
			ImGui::Begin("Info", &open, ImVec2(300, 100), 0.5f);
				ImGui::Text("FPS: %.2f", r2engine::get()->fps());

				render_terrain_ui();

				gdm->render_debug_ui();
				
				if (mesh_debug) {
					mvector<const char*> files;
					for (mstring& file : allFiles) {
						if (file.find(".pkg") != mstring::npos) {
							files.push_back(file.c_str());
						}
					}
					if (ImGui::ListBox("##files", &listItem, &files[0], files.size(), 30)) {
						load_mesh(files[listItem]);
					}
				}

				if (!mesh_debug || !is_vehicle) ImGui::InputFloat3("Position", &camera->j_pos.x, 2);
			ImGui::End();

			if (mesh_debug) {
				ImGui::Begin("Entities", &open);
					for (scene_entity* e : meshEntities) {
						render_entity_ui(e);
					}
				ImGui::End();
			} else {
				//render_entity_ui(vehicle);
			}

			draw->end();
			r2engine::renderer()->driver()->present_texture(render_target->attachment(0), fullscreen_tex);
		}

		virtual void doUpdate(f32 frameDt, f32 updateDt) {
			if (mesh_debug) return;
			if (r2engine::input()->joystick_count() > 0) {
				auto js = r2engine::input()->joystick(0);
				if (js->getJoyStickState().mButtons[0]) {
					vehicle->physics->rigidBody()->applyCentralImpulse(btVector3(0.0f, 100.0f, 0.0f));
					vehicle->move_to(glm::translate(mat4f(1.0f), vec3f(666.0f * 2.5f, 40.0f, 576.0f * 2.5f)));
				}
				if (js->getJoyStickState().mButtons[8]) {
					test_entity* e = *cubes->at(rand() % INSTANCE_COUNT);
					
					mat4f cam = glm::inverse(camera->transform->transform);
					vec3f spawn_at = cam * vec4f(0.0f, 0.0f, -10.0f, 1.0f);
					vec3f shoot_at = cam * vec4f(0.0f, 0.0f, -11.0f, 1.0f);
					vec3f shoot_dir = glm::normalize(shoot_at - spawn_at);

					mat4f t(1.0f);
					t = glm::translate(t, spawn_at);
					e->physics->set_transform(t);
					e->physics->rigidBody()->activate(true);
					e->physics->rigidBody()->setLinearVelocity(btVector3(shoot_dir.x, shoot_dir.y, shoot_dir.z) * 25.0f);
					e->physics->rigidBody()->applyTorqueImpulse(btVector3(random() * 15.0f, random() * 15.0f, random() * 15.0f));
				}
				if (js->getJoyStickState().mButtons[10]) {
					test_entity* e = *cubes->at(rand() % INSTANCE_COUNT);
					mat4f t(1.0f);
					vec3f r(random() * INSTANCE_RADIUS, random() * 5.0f, random() * INSTANCE_RADIUS);
					t = glm::translate(t, vec3f(896.0f * 2.5f, 50.0f, 576.0f * 2.5f) + r);
					e->physics->set_transform(t);
					e->physics->rigidBody()->activate(true);
					e->physics->rigidBody()->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
					e->physics->rigidBody()->applyTorqueImpulse(btVector3(random() * 15.0f, random() * 15.0f, random() * 15.0f));
				}
			}
		}



		void init_scene() {
			if (!mesh_debug || !is_vehicle) {
				camera = new fly_camera_entity();
				camera->j_pos = vec3f(0.09f, -0.11f, -3.7f);
			}

			if (!mesh_debug) {
				cubes = new dynamic_pod_array<test_entity*>();

				mesh = load_obj("./resource/physics_test/cube.obj", getScene());

				for(u32 i = 0;i < INSTANCE_COUNT;i++) {
					cubes->push(new test_entity(mesh));
				}

				terrain = new terrain_entity("offroad/Afghan.btx2", "offroad/afghan.imap", getScene(), gdm);
				mvector<sr2::pkg_entity*> entities = gdm->get_instances("offroad/Afghan.inst");

				//load_vehicle("geometry/vpsupbug.pkg");
				camera = new fly_camera_entity();
				camera->j_pos = vec3f(0.09f, -0.11f, -3.7f);

				mission = gdm->get_mission("Afghan_AAAcruise.sp");
				camera->j_pos = vec3f(-896.0f * 2.5f, -40.0f, -576.0f * 2.5f);//-mission->player_start_pos;
				r2Warn("%f, %f, %f", mission->player_start_pos.x, mission->player_start_pos.y, mission->player_start_pos.z);
			} else {
				cubes = new dynamic_pod_array<test_entity*>();

				mesh = load_obj("./resource/physics_test/cube.obj", getScene());

				cubes->push(new test_entity(mesh, true));
				
				allFiles = gdm->archive()->file_list();
				i32 i = 0;
				for (mstring& file : allFiles) {
					if (file.find(".pkg") != mstring::npos) {
						if (file == debug_file) {
							listItem = i;
							break;
						}
						i++;
					}
				}

				if (!is_vehicle) load_mesh(debug_file);
				else load_vehicle(debug_file);
			}
		}

		void init_render_target() {
			vec2i ws = r2engine::window()->get_size();
			render_target = getScene()->create_render_target();
			render_target->set_depth_mode(rbdm_24_bit);

			texture_buffer* color_texture = getScene()->create_texture();
			color_texture->create(ws.x, ws.y, 4, tt_unsigned_byte);
			render_target->attach(color_texture);

			texture_buffer* surfaceInfo_texture = getScene()->create_texture();
			surfaceInfo_texture->create(ws.x, ws.y, 4, tt_int);
			render_target->attach(surfaceInfo_texture);

			getScene()->set_render_target(render_target);

			fullscreen_tex = getScene()->load_shader("./resource/shaders/fullscreen_tex.glsl", "fullscreen_shader");
		}

		void init_debug_draw() {
			draw_shader = getScene()->load_shader("./resource/shaders/debug.glsl", "debug_shader");
			draw = new debug_drawer(getScene(), draw_shader, 131072 * 2, 8192 * 3);
			physDraw = new physics_drawer(draw);

			auto& ps = physics_sys::get()->physState();
			ps.enable();
			ps->world->setDebugDrawer(physDraw);
			ps.disable();
		}



		void load_mesh(const char* file) {
			for (auto entity : meshEntities) entity->destroy();
			meshEntities.clear();

			auto nodes = gdm->get_mesh(file, 1);
			for (auto info : nodes) {
				render_node* node = info.first;
				sr2::bbnd_file* bounds = info.second;
				node->destroy_when_unused = true;
				sr2::pkg_entity* e = new sr2::pkg_entity(format_string("%s[%d]", file, 0), node, bounds, mat4f(1.0f));
				meshEntities.push_back(e);
				meshNodes.push_back(node);
			}
		}

		void load_vehicle(const char* file) {
			for (auto entity : meshEntities) entity->destroy();
			meshEntities.clear();

			vehicle = gdm->get_vehicle(file, 1);
			meshEntities.push_back(vehicle);
		}



		void render_terrain_ui() {
			if (mesh_debug) return;

			vec2i cp = r2engine::window()->get_cursor();
			vec2i wd = r2engine::window()->get_size();
			vec4i px = vec4i(-1, -1, -1, -1);
			if (cp.x >= 0 && cp.y >= 0 && cp.x < wd.x && cp.y < wd.y) {
				px = render_target->pixel<vec4i>(cp.x, cp.y, 1);
			}
			vec2i terrainCoord(px.y, px.z);
			if (terrainCoord.x >= 0 && terrainCoord.x < terrain->data->width() && terrainCoord.y >= 0 && terrainCoord.y < terrain->data->length()) {
				vec4ui pathInfo = *(vec4ui*)&terrain->data->path_info_map()[(terrainCoord.x * 4) + (terrainCoord.y * terrain->data->width() * 4)];
				u8 pathTexId = terrain->data->path_tex_id_map()[terrainCoord.x + (terrainCoord.y * terrain->data->width())];
				if (pathTexId != 0xFF) {
					texture_buffer* tex = terrain->material_instance->texture(pathTexId)->textures[0];
					GLuint texId = ((gl_render_driver*)r2engine::renderer()->driver())->get_texture_id(tex);
					ImGui::Image((void*)texId, ImVec2(128, 128), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 1));

					ImGui::Columns(4);
					ImGui::Text("unk0");
					ImGui::NextColumn();
					ImGui::Text("unk1");
					ImGui::NextColumn();
					ImGui::Text("unk2");
					ImGui::NextColumn();
					ImGui::Text("tidx?");
					ImGui::NextColumn();

					u8* pathInfoVals = (u8*)&pathInfo.x;
					for (u32 i = 0;i < 4;i++) {
						u8 xyzw[4] = {
							pathInfoVals[0 + (i * 4)],
							pathInfoVals[1 + (i * 4)],
							pathInfoVals[2 + (i * 4)],
							pathInfoVals[3 + (i * 4)]
						};
						/*
						ImGui::Text("%.2f", ((f32(xyzw[0]) / 255.0f) * 2.0f) - 1.0f);
						ImGui::NextColumn();
						ImGui::Text("%.2f", ((f32(xyzw[1]) / 255.0f) * 2.0f) - 1.0f);
						ImGui::NextColumn();
						ImGui::Text("%.2f", ((f32(xyzw[2]) / 255.0f) * 2.0f) - 1.0f);
						ImGui::NextColumn();
						*/
						ImGui::Text("0x%.2x", xyzw[0]);
						ImGui::NextColumn();
						ImGui::Text("0x%.2x", xyzw[1]);
						ImGui::NextColumn();
						ImGui::Text("0x%.2x", xyzw[2]);
						ImGui::NextColumn();
						ImGui::Text("0x%.2x", xyzw[3]);
						ImGui::NextColumn();
					}
					ImGui::Columns(1);
				}
			}

			static vec2i infoTexCoordIndices = vec2i(0, 1);
			ImGui::InputInt("infoTexCoordIndices.x", &infoTexCoordIndices.x, 1, 1);
			ImGui::InputInt("infoTexCoordIndices.y", &infoTexCoordIndices.y, 1, 1);
			static vec4i cornerIndices = vec4i(0, 1, 2, 3);
			ImGui::InputInt("top left", &cornerIndices.x, 1, 1);
			ImGui::InputInt("top right", &cornerIndices.y, 1, 1);
			ImGui::InputInt("bottom left", &cornerIndices.z, 1, 1);
			ImGui::InputInt("bottom right", &cornerIndices.w, 1, 1);

			f32 rf = terrain->physics->rigidBody()->getRollingFriction();
			f32 sf = terrain->physics->rigidBody()->getFriction();
			if (ImGui::InputFloat("Rolling friction", &rf, 0.01f, 0.01f, 2)) {
				terrain->physics->rigidBody()->setRollingFriction(rf);
			}
			if (ImGui::InputFloat("Static friction", &sf, 0.01f, 0.01f, 2)) {
				terrain->physics->rigidBody()->setFriction(sf);
			}
			//ImGui::InputFloat("Max vehicle torque", &vehicle->maxTorque, 10.0f, 10.0f, 2);

			shader_program* tshdr = terrain->shader;
			tshdr->activate();
			tshdr->uniform2i(tshdr->get_uniform_location("hoverCoords"), terrainCoord.x, terrainCoord.y);
			tshdr->uniform2i(tshdr->get_uniform_location("infoTexCoordIndices"), infoTexCoordIndices.x, infoTexCoordIndices.y);
			tshdr->uniform1i(tshdr->get_uniform_location("tl"), cornerIndices.x);
			tshdr->uniform1i(tshdr->get_uniform_location("tr"), cornerIndices.y);
			tshdr->uniform1i(tshdr->get_uniform_location("bl"), cornerIndices.z);
			tshdr->uniform1i(tshdr->get_uniform_location("br"), cornerIndices.w);
			tshdr->deactivate();
		}

		void render_entity_ui(scene_entity* e, bool hovered = false) {
			ImGui::Text("%s", e->name().c_str());
			bool thisHovered = ImGui::IsItemHovered();

			if (e->physics) {
				f32 rf = e->physics->rigidBody()->getRollingFriction();
				f32 sf = e->physics->rigidBody()->getFriction();
				f32 mass = e->physics->rigidBody()->getMass();
				if (ImGui::InputFloat(format_string("Rolling friction##%d", e->id()).c_str(), &rf, 0.01f, 0.01f, 2)) {
					e->physics->rigidBody()->setRollingFriction(rf);
				}
				if (ImGui::InputFloat(format_string("Static friction##%d", e->id()).c_str(), &sf, 0.01f, 0.01f, 2)) {
					e->physics->rigidBody()->setFriction(sf);
				}
				if (ImGui::InputFloat(format_string("Mass##%d", e->id()).c_str(), &mass, 1.0f, 1.0f, 2)) {
					e->physics->set_mass(mass);
				}
			}

			if (e->mesh) {
				render_node* node = e->mesh->get_node();
				if (hovered || thisHovered) {
					node->material_instance()->uniforms()->uniform_float("highlight", 1.5f);
					
					if (e->transform) {
						mat4f transform = e->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
						vec3f origin = transform * vec4f(0, 0, 0, 1);
						vec3f right = transform * vec4f(1, 0, 0, 1);
						vec3f up = transform * vec4f(0, 1, 0, 1);
						vec3f forward = transform * vec4f(0, 0, 1, 1);

						draw->line(origin, right, vec4f(1, 0, 0, 1));
						draw->line(origin, up, vec4f(0, 1, 0, 1));
						draw->line(origin, forward, vec4f(0, 0, 1, 1));
					}
				}
				else node->material_instance()->uniforms()->uniform_float("highlight", 0.0f);
			}

			auto children = e->children();
			if (children.size() > 0) {
				ImGui::Indent(10.0f);
				for (scene_entity* child : children) render_entity_ui(child, hovered || thisHovered);
				ImGui::Unindent(10.0f);
			}
		}

		dynamic_pod_array<test_entity*>* cubes;
		render_node* mesh;

		fly_camera_entity* camera;
		terrain_entity* terrain;
		sr2::vehicle_entity* vehicle;

		render_buffer* render_target;
		shader_program* fullscreen_tex;

		shader_program* draw_shader;
		debug_drawer* draw;
		physics_drawer* physDraw;

		i32 listItem;
		mvector<mstring> allFiles;
		mvector<render_node*> meshNodes;
		mvector<scene_entity*> meshEntities;

		sr2::mission* mission;
		sr2::gamedata_manager* gdm;
};

int main(int argc, char** argv) {
	r2engine::create(argc, argv);
	auto eng = r2engine::get();

	eng->open_window(1800, 800, "sr2", true);
	eng->renderer()->set_driver(new gl_render_driver(eng->renderer()));

	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	test_state* state = new test_state();
	eng->states()->register_state(state);
	eng->states()->activate("TestState");

	int ret = eng->run();
	eng->shutdown();
	return ret;
}
