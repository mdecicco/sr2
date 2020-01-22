#include <entities/pkg.h>
#include <gamedata/bbnd_file.h>

#include <r2/engine.h>
using namespace r2;

namespace sr2 {
	pkg_entity::pkg_entity(const mstring& name, render_node* rnode, bbnd_file* _bounds, const mat4f& transform)
		: scene_entity(name), node(rnode), bounds(_bounds), trns(transform)
	{ }

	pkg_entity::~pkg_entity() { }

	void pkg_entity::onInitialize() {
		stop_periodic_updates();
		if (node) {
			mesh_sys::get()->addComponentTo(this);
			mesh->set_node(node);
			mesh->set_instance_data(trns);
		}
		if (bounds) {
			transform_sys::get()->addComponentTo(this);
			transform->transform = trns;

			physics_sys::get()->addComponentTo(this);
			physics->set_mass(0.0f);
			btTriangleMesh* msh = new btTriangleMesh(false, false);
			for (u32 i = 0;i < bounds->vertices.size();i += 3) {
				bbnd_file::vertex v0 = bounds->vertices[i + 0];
				bbnd_file::vertex v1 = bounds->vertices[i + 1];
				bbnd_file::vertex v2 = bounds->vertices[i + 2];
				msh->addTriangle(
					btVector3(v0.position.x, v0.position.y, v0.position.z),
					btVector3(v1.position.x, v1.position.y, v1.position.z),
					btVector3(v2.position.x, v2.position.y, v2.position.z),
					true
				);
			}

			btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(msh, true);
			physics->set_shape(shape);
		}
		r2engine::get()->remove_child(this);
	}
};