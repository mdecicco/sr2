#include <entities/vehicle.h>
#include <entities/pkg.h>

#include <gamedata/bbnd_file.h>

#include <r2/engine.h>
#include <r2/systems/cascade_functions.h>
using namespace r2;

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <glm/gtc/type_ptr.hpp>

/* Notes:
	Breakable parts pivot around the X axis
	Wheels spin around the X axis and pivot around the Y axis
	Seems like everything that pivots does so around the X axis
	Shocks probably compress in the Z axis
 */

namespace sr2 {
	vehicle_camera::vehicle_camera(const mstring& name) : scene_entity(name) {
	}

	vehicle_camera::~vehicle_camera() {
	}

	void vehicle_camera::onInitialize() {
		setUpdateFrequency(60.0f);
		transform_sys::get()->addComponentTo(this);
		camera_sys::get()->addComponentTo(this);
		camera->activate();
		vec2f screen = r2engine::get()->window()->get_size();
		camera->projection = glm::perspective(glm::radians(90.0f), screen.x / screen.y, 0.1f, 200.0f);
	}

	void vehicle_camera::onUpdate(f32 frameDt, f32 updateDt) {
		vec2f screen = r2engine::get()->window()->get_size();
		camera->projection = glm::perspective(glm::radians(90.0f), screen.x / screen.y, 0.1f, 1000.0f);

		transform->transform = glm::inverse(tracking->transform->transform * glm::translate(glm::mat4(1.0f), vec3f(0.0f, 5.0f, 10.0f)));
	}

	// IMPORTANT NOTE: This function assumes that all meshes associated
	// with the entity (and its children) use vertex formats that specify
	// the position components first, and in x-y-z order
	btConvexHullShape* build_hull(scene_entity* entity) {
		btConvexHullShape initialHull;

		if (entity->mesh) {
			render_node* node = entity->mesh->get_node();
			u8* vertices = (u8*)node->vertex_data();
			size_t stride = node->vertices().buffer->format()->size();
			size_t vcount = node->vertex_count();
			for (size_t i = 0;i < vcount;i++) {
				size_t off = i * stride;
				vec3f* vert = (vec3f*)(vertices + off);
				initialHull.addPoint(btVector3(vert->x, vert->y, vert->z));
			}
		}

		entity->for_each_child([&initialHull](scene_entity* e) {
			if (e->mesh) {
				render_node* node = e->mesh->get_node();
				u8* vertices = (u8*)node->vertex_data();
				size_t stride = node->vertices().buffer->format()->size();
				size_t vcount = node->vertex_count();
				for (size_t i = 0;i < vcount;i++) {
					size_t off = i * stride;
					vec3f* vert = (vec3f*)(vertices + off);
					initialHull.addPoint(btVector3(vert->x, vert->y, vert->z));
				}
			}
		});

		initialHull.setMargin(0.0f);
		btShapeHull* hull = new btShapeHull(&initialHull);
		hull->buildHull(0.0f);
		btConvexHullShape* shape = new btConvexHullShape(
			(f32*)hull->getVertexPointer(),
			hull->numVertices(),
			sizeof(btVector3)
		);
		delete hull;
		return shape;
	}

	// IMPORTANT NOTE: This function assumes that all meshes associated
	// with the entity (and its children) use vertex formats that specify
	// the position components first, and in x-y-z order
	// also, returns x = radius, y = thickness
	vec2f calculateWheelDimensions(scene_entity* entity) {
		f32 max_y = 0.0f;
		f32 min_x = FLT_MAX;
		f32 max_x = FLT_MIN;
		if (entity->mesh) {
			render_node* node = entity->mesh->get_node();
			u8* vertices = (u8*)node->vertex_data();
			size_t stride = node->vertices().buffer->format()->size();
			size_t vcount = node->vertex_count();
			for (size_t i = 0;i < vcount;i++) {
				size_t off = i * stride;
				vec3f* vert = (vec3f*)(vertices + off);
				if (vert->y > max_y) max_y = vert->y;
				if (vert->x < min_x) min_x = vert->x;
				if (vert->x > max_x) max_x = vert->x;
			}
		}

		return vec2f(max_y, max_x - min_x);
	}

	btTransform glmToBt(const mat4f& transform) {
		btTransform out;
		mat4f trns = transform;
		trns[0] = glm::normalize(trns[0]);
		trns[1] = glm::normalize(trns[1]);
		trns[2] = glm::normalize(trns[2]);
		out.setFromOpenGLMatrix(glm::value_ptr(trns));
		return out;
	}

	btVector3 glmToBt(const vec3f& v) {
		return btVector3(v.x, v.y, v.z);
	}

	vehicle_entity::vehicle_entity(const mstring& name) : scene_entity(name) {
		memset(wheels, 0, sizeof(wheels));
		memset(arms, 0, sizeof(arms));
		memset(shocks, 0, sizeof(shocks));
		memset(shafts, 0, sizeof(shafts));
		memset(sirens, 0, sizeof(sirens));
		memset(exhausts, 0, sizeof(exhausts));
		memset(axles, 0, sizeof(axles));
		memset(breakables, 0, sizeof(breakables));
		memset(variants, 0, sizeof(variants));
		body = nullptr;
		engine = nullptr;
		oil = nullptr;
		shadow = nullptr;
		driver = nullptr;

		steerAngle = 0.0f;
		maxTorque = 500.0f;
		time = 0.0f;
	}

	vehicle_entity::~vehicle_entity() { }

	void vehicle_entity::onInitialize() {
		transform_sys* tsys = transform_sys::get();
		physics_sys* psys = physics_sys::get();
		tsys->addComponentTo(this);

		for_each_child([tsys](scene_entity* e) {
			tsys->addComponentTo(e);
			if (e->has_children()) {
				e->for_each_child([tsys, e](scene_entity* gc) {
					e->transform->transform = ((pkg_entity*)gc)->trns;
					tsys->addComponentTo(gc);
					gc->transform->transform = mat4f(1.0f);
				});
			} else e->transform->transform = ((pkg_entity*)e)->trns;
		});

		if (body) {
			auto& pstate = psys->physState();
			pstate.enable();

			psys->addComponentTo(this);
			physics->set_mass(65.0f);
			physics->set_shape(build_hull(body));

			for (u8 i = 0;i < 4;i++) {
				if (!arms[i]) continue;
				psys->addComponentTo(arms[i]);
				arms[i]->physics->set_mass(1.0f);
				arms[i]->physics->set_shape(build_hull(arms[i]));

				btTransform frameA, frameB;
				frameA = glmToBt(arms[i]->transform->transform);
				frameB.setIdentity();
				armConstraints[i] = new btGeneric6DofSpring2Constraint(
					*physics->rigidBody(),
					*arms[i]->physics->rigidBody(),
					frameA,
					frameB
				);
				armConstraints[i]->setAngularLowerLimit(btVector3(glm::radians(-5.0f), 0.0f, 0.0f));
				armConstraints[i]->setAngularUpperLimit(btVector3(glm::radians( 10.0f), 0.0f, 0.0f));
				armConstraints[i]->setOverrideNumSolverIterations(20);
				armConstraints[i]->enableSpring(3, true);
				armConstraints[i]->setEquilibriumPoint(3, glm::radians(6.0f));
				armConstraints[i]->setStiffness(3, 5465.0f);
				armConstraints[i]->setDamping(3, 0.35f);

				pstate->world->addConstraint(armConstraints[i], true);
			}

			for (u8 i = 0;i < 4;i++) {
				if (!wheels[i]) continue;

				psys->addComponentTo(wheels[i]);
				wheels[i]->physics->set_mass(40.0f);
				vec2f wheelDims = calculateWheelDimensions(wheels[i]);
				btCylinderShape* shape = new btCylinderShapeX(btVector3(wheelDims.y * 0.5f, wheelDims.x, wheelDims.x));
				wheels[i]->physics->set_shape(shape);
					
				btTransform frameA, frameB;
				frameA = glmToBt(glm::inverse(arms[i]->transform->transform) * wheels[i]->transform->transform);
				frameB.setIdentity();
				tireConstraints[i] = new btGeneric6DofConstraint(
					*arms[i]->physics->rigidBody(),
					*wheels[i]->physics->rigidBody(),
					frameA,
					frameB,
					false
				);
				bool rearWheel = i == 2 || i == 3;
				tireConstraints[i]->setAngularLowerLimit(btVector3( 1.0f, rearWheel ? 0.0f : glm::radians(steerAngle), 0.0f));
				tireConstraints[i]->setAngularUpperLimit(btVector3(-1.0f, rearWheel ? 0.0f : glm::radians(steerAngle), 0.0f));
				tireConstraints[i]->setOverrideNumSolverIterations(40);

				pstate->world->addConstraint(tireConstraints[i], true);

				wheels[i]->physics->rigidBody()->setActivationState(DISABLE_DEACTIVATION);
				wheels[i]->physics->rigidBody()->setFriction(0.45f);
				wheels[i]->physics->rigidBody()->setRollingFriction(0.45f);
			}
			pstate.disable();
		}

		start_periodic_updates();
		setUpdateFrequency(60.0f);
		r2engine::get()->remove_child(this);
	}

	void vehicle_entity::onUpdate(f32 frameDt, f32 updateDt) {
		time += updateDt;
		steerAngle = sin(time) * 45.0f;

		if (r2engine::input()->joystick_count() > 0) {
			auto js = r2engine::input()->joystick(0);
			auto jState = js->getJoyStickState();
			vec2f lstick = vec2f(f32(jState.mAxes[1].abs) / 32767.0f, f32(jState.mAxes[0].abs) / 32767.0f);
			steerAngle = lstick.x * 45.0f;
			if (steerAngle < -45.0f) steerAngle = -45.0f;
			else if (steerAngle > 45.0f) steerAngle = 45.0f;

			if (jState.mButtons[0]) {
				physics->rigidBody()->applyCentralImpulse(btVector3(0.0f, 100.0f, 0.0f));
				/*
				btTransform t;
				t.setIdentity();
				t.setOrigin(btVector3(0.0f, 10.0f, 0.0f));
				physics->rigidBody()->setWorldTransform(t);
				physics->rigidBody()->setLinearVelocity(btVector3(0, 0, 0));
				physics->rigidBody()->setAngularVelocity(btVector3(0, 0, 0));
				*/
			}
			if (jState.mButtons[8]) {
				btRigidBody* bodies[2] = {
					wheels[2]->physics->rigidBody(),
					wheels[3]->physics->rigidBody()
				};
				btTransform t;
				t.setIdentity();
				//t.setRotation(bodies[0]->getWorldTransform().getRotation());
				//bodies[0]->applyTorque(t * btVector3(-maxTorque, 0, 0));
				//t.setRotation(bodies[1]->getWorldTransform().getRotation());
				//bodies[1]->applyTorque(t * btVector3(-maxTorque, 0, 0));
				t.setRotation(physics->rigidBody()->getWorldTransform().getRotation());
				physics->rigidBody()->applyForce(t * btVector3(0, 0, -maxTorque), btVector3(0, 0, 0));
			} else if (jState.mButtons[10]) {
				btRigidBody* bodies[2] = {
					wheels[2]->physics->rigidBody(),
					wheels[3]->physics->rigidBody()
				};
				btTransform t;
				t.setIdentity();
				//t.setRotation(bodies[0]->getWorldTransform().getRotation());
				//bodies[0]->applyTorque(t * btVector3(maxTorque, 0, 0));
				//t.setRotation(bodies[1]->getWorldTransform().getRotation());
				//bodies[1]->applyTorque(t * btVector3(maxTorque, 0, 0));
				t.setRotation(physics->rigidBody()->getWorldTransform().getRotation());
				physics->rigidBody()->applyForce(t * btVector3(0, 0, maxTorque), btVector3(0, 0, 0));
			}
		}

		// keep front tires aligned with steering angle
		tireConstraints[0]->setAngularLowerLimit(btVector3( 1.0f, glm::radians(steerAngle), 0.0f));
		tireConstraints[0]->setAngularUpperLimit(btVector3(-1.0f, glm::radians(steerAngle), 0.0f));
		tireConstraints[1]->setAngularLowerLimit(btVector3( 1.0f, glm::radians(steerAngle), 0.0f));
		tireConstraints[1]->setAngularUpperLimit(btVector3(-1.0f, glm::radians(steerAngle), 0.0f));

		// scale the shocks and shafts
		for (u8 i = 0;i < 4;i++) {
			if (!wheels[i] || !shocks[i]) continue;

			mat4f wheelTrans = wheels[i]->transform->transform;

			vec3f wheelNow = wheelTrans * vec4f(0, 0, 0, 1);
			vec3f wheelInitial = ((pkg_entity*)wheels[i])->trns * vec4f(0, 0, 0, 1);
			vec3f shockTop = shocks[i]->transform->transform * vec4f(0, 0, 0, 1);
			f32 scale = (wheelNow.y - shockTop.y) / (wheelInitial.y - shockTop.y);
			
			// normalize shock scale
			mat4f shockTransform = shocks[i]->transform->transform;
			shockTransform[0] = glm::normalize(shockTransform[0]);
			shockTransform[1] = glm::normalize(shockTransform[1]);
			shockTransform[2] = glm::normalize(shockTransform[2]);
			shocks[i]->transform->transform = glm::scale(shockTransform, vec3f(1.0f, 1.0f, scale));

			if (!shafts[i]) continue;

			auto shaftParts = shafts[i]->children();
			mat4f shaftPartTransforms[2] = {
				shaftParts[0]->transform->transform,
				shaftParts[1]->transform->transform
			};

			vec3f shaftTop = shaftPartTransforms[1] * vec4f(0, 0, 0, 1);
			scale = (wheelNow.y - shaftTop.y) / (wheelInitial.y - shaftTop.y);
			scale = 1.0f + (1.0f - scale);

			// normalize shaft scale
			mat4f shaftTransform = shaftPartTransforms[1];
			shaftTransform[0] = glm::normalize(shaftTransform[0]);
			shaftTransform[1] = glm::normalize(shaftTransform[1]);
			shaftTransform[2] = glm::normalize(shaftTransform[2]);
			shaftParts[1]->transform->transform = glm::scale(shaftTransform, vec3f(1.0f, scale, 1.0f));

			//todo: automatic bone weights for stupid brake disk assembly
		}

		for_each_child([](scene_entity* e) {
			if (e->physics) return;

			if (e->mesh) {
				e->mesh->set_instance_transform(e->transform->cascaded_property(&transform_component::transform, &cascade_mat4f));
			}

			if (e->has_children()) {
				e->for_each_child([e](scene_entity* gc) {
					gc->mesh->set_instance_transform(gc->transform->cascaded_property(&transform_component::transform, &cascade_mat4f));
				});
			}
		});
	}

	void vehicle_entity::move_to(const mat4f& transform) {
		btTransform bodyTrans = glmToBt(transform);
		physics->set_transform(transform);
		physics->rigidBody()->setLinearVelocity(btVector3(0, 0, 0));
		physics->rigidBody()->setAngularVelocity(btVector3(0, 0, 0));

		for_each_child([transform](scene_entity* e) {
			if (e->physics) {
				mat4f initial = e->has_children() ? ((pkg_entity*)e->children()[0])->trns : ((pkg_entity*)e)->trns;
				mat4f now = transform * initial;
				e->physics->set_transform(now);
				e->physics->rigidBody()->setLinearVelocity(btVector3(0, 0, 0));
				e->physics->rigidBody()->setAngularVelocity(btVector3(0, 0, 0));
			}
		});
	}
};