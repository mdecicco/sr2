#include <entities/terrain.h>
#include <gamedata/manager.h>
#include <gamedata/btx2_file.h>
#include <r2/engine.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

using namespace r2;
using namespace sr2;

terrain_entity::terrain_entity(const mstring& btx2, const mstring& lightMap, scene* s, gamedata_manager* gdm) : scene_entity("Terrain") {
	shader = s->load_shader("./resource/shaders/terrain.glsl", "terrain_shader");
	material_format = new uniform_format();
	material_format->add_attr("color", uat_vec3f);
	material = new node_material("u_material", material_format);
	material->set_shader(shader);
	material_instance = material->instantiate(s);

	auto tpair = gdm->get_terrain(btx2, lightMap, material_instance);
	data = tpair.first;
	node = tpair.second;

	node->set_material_instance(material_instance);
	material_instance->uniforms()->uniform_vec3f("color", vec3f(0.75f, 0.1f, 0.2f));

	mat4f t = mat4f(1.0f);
	t = glm::translate(t, vec3f(-4.75752497f, 0, -5.14905024));
	t = glm::scale(t, vec3f(5.0f, 5.0f, 5.0f));
	node->uniforms()->uniform_mat4f("transform", t);
}

terrain_entity::~terrain_entity() {
}

void terrain_entity::onInitialize() {
	transform_sys::get()->addComponentTo(this);
	f32 off_x = (f32(data->width() - 1) * 0.5f) * 5.0f;
	f32 off_y = (data->max_height() - data->min_height()) * 0.5f;
	f32 off_z = (f32(data->length() - 1) * 0.5f) * 5.0f;
	mat4f t = mat4f(1.0f);
	t = glm::translate(t, vec3f(-4.75752497f + off_x, data->min_height() + off_y, -5.14905024 + off_z));
	t = glm::scale(t, vec3f(5.0f, 1.0f, 5.0f));
	transform->transform = t;

	physics_sys::get()->addComponentTo(this);
	btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(
		data->width(),
		data->length(),
		data->height_map(),
		1.0f,
		data->min_height(),
		data->max_height(),
		1,
		PHY_FLOAT,
		false
	);
	physics->set_mass(0.0f);
	physics->set_shape(shape);
	physics->rigidBody()->setFriction(0.8f);
	physics->rigidBody()->setRollingFriction(0.3f);
	physics->rigidBody()->setSpinningFriction(0.45f);
}